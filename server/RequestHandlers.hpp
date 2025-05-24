#pragma once

#include "IRequest.hpp"
#include "UserInfo.hpp"
#include "simdjson.h"
#include "nlohmann/json.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include "ILogger.hpp"
#include <future> // for std::async

using json = nlohmann::json;

std::string get_current_utc_timestamp() {
	// Get current time point
	auto now = std::chrono::system_clock::now();

	// Convert to time_t
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);

	// Convert to UTC time
	std::tm utc_tm{};
#if defined(_WIN32) || defined(_WIN64)
	gmtime_s(&utc_tm, &now_c); // Windows
#else
	gmtime_r(&now_c, &utc_tm); // POSIX
#endif

	// Format the time to ISO 8601
	std::ostringstream oss;
	oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");

	return oss.str();
}

class HelloHandler : public IRequestHandler {
public:
	std::unique_ptr<IResponse> handleRequest(const IRequest& request) override {
		auto res = std::make_unique<HttpResponse>();
		res->setStatusCode(200);
		res->sethttpVersion(request.httpVersion());
		res->setHeader("Connection", "Keep-Alive");
		return res;
	}
};

class DefaultHandler : public IRequestHandler
{
public:
	std::unique_ptr<IResponse> handleRequest(const IRequest& request) override
	{
        auto res = std::make_unique<HttpResponse>();
        res->setStatusCode(404); // Not Found
        res->sethttpVersion(request.httpVersion());
        res->setHeader("Connection", "Keep-Alive");
        json responseJson;
        responseJson["message"] = "Resource not found";
        responseJson["timestamp"] = get_current_utc_timestamp();
        res->setBody(responseJson.dump());
        return res;
	}
};

class UserHandler : public IRequestHandler
{
public:
	UserHandler(std::shared_ptr<IBodyParser> _bodyParser, std::shared_ptr<ILogger> _logger) : m_bodyParser(_bodyParser), m_logger(_logger) {};
	std::unique_ptr<IResponse> handleRequest(const IRequest& request) override {
		std::unique_ptr<HttpResponse> response;
		auto start = std::chrono::high_resolution_clock::now();
		if (request.method() == "POST" && request.uri() == "/users") {
			response= std::make_unique < HttpResponse>(handlePOSTUsers(request));
		}
		else if (request.method() == "GET" && request.uri() == "/superusers")
		{
			response = std::make_unique < HttpResponse>(handleGETSuperusers());
        }
        else if (request.method() == "GET" && request.uri() == "/top-countries")
        {
            response = std::make_unique < HttpResponse>(handleGetTopCountries());
        }
        else if (request.method() == "GET" && request.uri() == "/team-insights")
        {
            response = std::make_unique < HttpResponse>(handleGetTeamInsights());
        }
		else if (request.method() == "GET" && request.uri() == "/active-users-per-day")
		{
			response = std::make_unique < HttpResponse>(handleGetActiveUsers());
        }
        else if (request.method() == "GET" && request.uri() == "/evaluation")
        {
            response = std::make_unique < HttpResponse>(handleGetEvaluation());
        }
		else {
			response = std::make_unique<HttpResponse>();
			response->setStatusCode(405); // Method Not Allowed			 
		}
        response->sethttpVersion(request.httpVersion());
		auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        m_logger->info("[" __FUNCTION__ "] Handled \'{}\' \'{}\' in {} ms", request.method(), request.uri(), duration.count());
		return response;
	}
private:
    std::shared_ptr<IBodyParser> m_bodyParser;
	std::shared_ptr<ILogger> m_logger;
    std::vector<UserInfo::User> users;
	
	HttpResponse handlePOSTUsers(const IRequest& request) 
	{
		auto start = std::chrono::high_resolution_clock::now();
		HttpResponse res;
		res.setStatusCode(400); // default to bad request
		res.sethttpVersion(request.httpVersion());
		json responseJson;

		if (m_bodyParser->parse(request.getHeader("Content-Type"), request.body()))
		{
			auto user_json_str = m_bodyParser->getValue("users");
			users.clear();
			if (parseUsers(user_json_str))
			{
				responseJson["message"] = "Arquivo recebido com sucesso";
				responseJson["user_count"] = static_cast<int>(users.size());
				res.setStatusCode(200);
			}
			else
			{
                responseJson["message"] = "Erro ao processar o arquivo";
                responseJson["user_count"] = 0;
			}
        }
		auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_logger->info("[" __FUNCTION__ "] Parsed users in {} ms", duration.count());
        setDefaultBody(responseJson, duration);		
		res.setBody(responseJson.dump());
		return res;
	}

	HttpResponse handleGETSuperusers()
	{
		auto start = std::chrono::high_resolution_clock::now();
		HttpResponse res;
		res.setStatusCode(200);
		res.sethttpVersion("HTTP/1.1");
		res.setHeader("Content-Type", "application/json");

		json responseJson;
		json users_list;
		for (auto user : users) {
			if (user.active == true && user.score >= 900)
			{
				users_list.push_back(std::string(user.name));
			}
        }
		responseJson["data"] = users_list;

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		setDefaultBody(responseJson, duration);
        res.setBody(responseJson.dump());
        return res;
	}
	
	HttpResponse handleGetTopCountries()
	{
		auto start = std::chrono::high_resolution_clock::now();
		HttpResponse res;
		res.setStatusCode(200); // default to bad request
		json responseJson;

        std::unordered_map<std::string, int> country_count_map;
		for (auto user : users)
		{
            country_count_map[user.country]++;
		}
        std::vector<std::pair<std::string, int>> top_countries_vector(country_count_map.begin(), country_count_map.end());

        std::sort(top_countries_vector.begin(), top_countries_vector.end(),
            [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                return a.second > b.second;
            });

        for (int i = 0; i < 5 && i < top_countries_vector.size(); ++i)
        {
            responseJson["countries"][i]["country"] = top_countries_vector[i].first;
            responseJson["countries"][i]["total"] = top_countries_vector[i].second;
        }

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		setDefaultBody(responseJson, duration);
		res.setBody(responseJson.dump());
		return res;
	}

	HttpResponse handleGetTeamInsights()
	{
		auto start = std::chrono::high_resolution_clock::now();
		HttpResponse res;
		res.setStatusCode(200);
		res.setHeader("Content-Type", "application/json"); 
		json responseJson;

		struct team
		{
			int total_members;
            int active_members;
			int leaders;
			int projects;
            int completed_projects;
		};

		std::unordered_map<std::string, team> teams_map;
        for (auto user : users)
        {
            teams_map[user.team.name].total_members++;
            if (user.active == true)
            {
                teams_map[user.team.name].active_members++;
            }
            if (user.team.leader == true)
            {
                teams_map[user.team.name].leaders++;
            }
			for (auto project : user.team.projects)
			{
                teams_map[user.team.name].projects++;
                if (project.completed == true)
                {
                    teams_map[user.team.name].completed_projects++;
                }
            }
        }
        for (auto& team : teams_map)
        {
			json js;
			js["team"] = team.first;
            js["total_members"] = team.second.total_members;
            js["active_percentage"] = static_cast<float>(static_cast<float>(team.second.active_members) * 100 / team.second.total_members);
            js["leaders"] = team.second.leaders;
            js["completed_projects"] = team.second.completed_projects;
            responseJson["teams"].push_back(js);
        }

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		setDefaultBody(responseJson, duration);
		res.setBody(responseJson.dump());
		return res;
	}

	HttpResponse handleGetActiveUsers()
	{
		auto start = std::chrono::high_resolution_clock::now();
		HttpResponse res;
		res.setStatusCode(200);
		res.setHeader("Content-Type", "application/json");
		json responseJson;

		std::unordered_map<std::string, int> logins_map;
		for (auto user : users)
		{
			for (auto login : user.logs)
			{
                if (login.action == "login")
                {
                    logins_map[login.date]++;
                }
			}
		}
		
		for (auto login : logins_map)
		{
            json js;
            js["date"] = login.first;
            js["total"] = login.second;
            responseJson["logins"].push_back(js);
		}
		
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		setDefaultBody(responseJson, duration);
		res.setBody(responseJson.dump());
		return res;
	}
    
	HttpResponse handleGetEvaluation()
	{
		auto start = std::chrono::high_resolution_clock::now();
		HttpResponse res;
		res.setStatusCode(200);
		res.setHeader("Content-Type", "application/json");
		json responseJson;
		
		auto test_endpoint = [&](std::string_view endpoint, HttpResponse response) -> json
			{
				json item;
				item[endpoint]["status"] = HttpStatus::Unauthorized;
				try
				{
					item[endpoint]["status"] = response.getCode();
					json body = json::parse(response.getBody());
					item[endpoint]["time_ms"] = body["execution_time_ms"];
					item[endpoint]["valid_response"] = true;
				}
                catch (const std::exception& e)
                {
                    item[endpoint]["valid_response"] = false;
                    item[endpoint]["error"] = e.what();
                }

				return item;
			};

		std::vector<std::future<json>> futures;
		futures.push_back(std::async(std::launch::async, [&]() { return test_endpoint("/team-insights", handleGetTeamInsights()); }));
		futures.push_back(std::async(std::launch::async, [&]() { return test_endpoint("/superusers", handleGETSuperusers()); }));
		futures.push_back(std::async(std::launch::async, [&]() { return test_endpoint("/top-countries", handleGetTopCountries()); }));
		futures.push_back(std::async(std::launch::async, [&]() { return test_endpoint("/active-users-per-day", handleGetActiveUsers()); }));

		for (auto& fut : futures)
		{
			responseJson["tested_endpoints"].push_back(fut.get());
		}		

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		setDefaultBody(responseJson, duration);
		res.setBody(responseJson.dump());
		return res;
	}

	bool parseUsers(std::string_view json_str)
    {
		bool result = false;
        auto start = std::chrono::high_resolution_clock::now();
		simdjson::padded_string json_str_padded = simdjson::padded_string(json_str.data(), json_str.size());
		simdjson::ondemand::parser parser;
        simdjson::ondemand::document doc = parser.iterate(json_str_padded);
		auto error = doc.get<std::vector<UserInfo::User>>(users);
		if (error)
		{
            m_logger->error("[" __FUNCTION__ "] Error parsing JSON: {}", simdjson::error_message(error));
		}
		else
		{
			result = true;
		}
		auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        m_logger->info("[" __FUNCTION__ "] parsed {} in {} ms", users.size(), duration.count());
		return result;
    }

	void setDefaultBody(json& js, std::chrono::milliseconds duration)
	{
        js["execution_time_ms"] = duration.count();
        js["timestamp"] = get_current_utc_timestamp();
	}

};
