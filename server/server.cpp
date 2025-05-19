// server.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include <iostream>
#include <fstream>
#include <chrono>
#include "EventLogger.hpp"
#include "SyncHTTPServer.hpp"
#include "ConsoleLogger.hpp"
#include "SqliteDatabase.hpp"
#include "HttpResponse.hpp"
#include "RequestRouter.hpp"
#include "Core/IBodyParser.hpp"
#include "MultipartBodyParser.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

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

class UserHandler : public IRequestHandler
{
public:
	UserHandler(std::shared_ptr<IBodyParser> _bodyParser) : m_bodyParser(_bodyParser) {};
	std::unique_ptr<IResponse> handleRequest(const IRequest& request) override {
		std::unique_ptr<IResponse> response;
		auto start = std::chrono::high_resolution_clock::now();
		if (request.method() == "POST" && request.uri() == "/users") {
			return std::make_unique < HttpResponse>(handlePOSTUsers(request));
		}
		else if (request.method() == "GET" && request.uri() == "/superusers")
		{
			response = std::make_unique < HttpResponse>(handleGETSuperusers());
		}
		else {
			response = std::make_unique<HttpResponse>();
			response->setStatusCode(405); // Method Not Allowed
			
		}
		auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		return response;
	}
private:

	std::string user_json_str;
	json user_list;
    std::shared_ptr<IBodyParser> m_bodyParser; \
	struct Project {
		std::string_view name;
		bool completed;
	};

	struct Team {
		std::string_view name;
		bool leader;
		std::vector<Project> projects;
	};

	struct Log {
		std::string_view date;
		std::string_view action;
	};

	struct User {
		std::string_view id;
		std::string_view name;
		int age;
		int score;
		bool active;
		std::string_view country;
		Team team;
		std::vector<Log> logs;
	};
    std::vector<User> users;
	
	HttpResponse handlePOSTUsers(const IRequest& request) 
	{
		auto start = std::chrono::high_resolution_clock::now();
		HttpResponse res;
		res.setStatusCode(400); // default to bad request
		res.sethttpVersion(request.httpVersion());

		if (m_bodyParser->parse(request.getHeader("Content-Type"), request.body()))
		{
			user_json_str = m_bodyParser->getValue("users");
            // Replace the problematic line with the following code:
            std::thread(&UserHandler::parseUsers, this, user_json_str).detach();
        }
		auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::cout << "Parsed users in "
			<< duration.count()
			<< "ms\n";
		json responseJson;

		responseJson["execution_time_ms"] = duration.count();
		res.setBody(responseJson.dump());
		res.setStatusCode(200);
		res.sethttpVersion(request.httpVersion());
		return res;
	}

	HttpResponse handleGETSuperusers()
	{
		auto start = std::chrono::high_resolution_clock::now();
		HttpResponse res;
		res.setStatusCode(200);
		res.sethttpVersion("HTTP/1.1");
		res.setHeader("Content-Type", "application/json");
		res.setHeader("Connection", "Keep-Alive");
		json responseJson;
		json users;
		for (const auto& user : user_list) {
			if (user["active"] == true && user["score"] >= 900)
			{
				users.push_back(user["name"]);
			}
        }
		responseJson["data"] = users;

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        auto dur_cnt = duration.count();
		responseJson["execution_time_ms"] = duration.count();
        res.setBody(responseJson.dump());
        return res;
	}
	
    void parseUsers(std::string_view json_str)
    {
        auto start = std::chrono::high_resolution_clock::now();
        user_list = json::parse(json_str);
		users.clear();
		for (auto user : user_list)
		{
            User u;
            u.id = user["id"];
            u.name = user["name"];
            u.active = user["active"];
            u.age = user["age"];
            u.score = user["score"];
            u.country = user["country"];
            u.team.name = user["team"]["name"];
            u.team.leader = user["team"]["leader"];
            for (auto project : user["team"]["projects"])
            {
                Project p;
                p.name = project["name"];
                p.completed = project["completed"];
                u.team.projects.push_back(p);
            }
            for (auto log : user["logs"])
            {
                Log l;
                l.date = log["date"];
                l.action = log["action"];
                u.logs.push_back(l);
            }
            users.push_back(u);
		}
		auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Parsed " << users.size() << " users in "
            << duration.count()
            << "ms\n";
    }

};


int main()
{
	std::shared_ptr<ILogger> logger = std::make_shared<ConsoleLogger>();
	std::shared_ptr<IDatabase> db = std::make_shared<SqliteDatabase>(logger);
	std::shared_ptr<SqliteDatabase> sqliteDb = std::dynamic_pointer_cast<SqliteDatabase>(db);
	std::shared_ptr<IRequestHandler> helloHandler = std::make_shared<HelloHandler>();
	std::shared_ptr<IRequestHandler> userHandler = std::make_shared<UserHandler>(std::make_shared<MultipartBodyParser>());
    std::shared_ptr<RequestRouter> router = std::make_shared<RequestRouter>();

	router->registerHandler("GET", "/hello", helloHandler);
	router->registerHandler("POST", "/users", userHandler);
	router->registerHandler("GET", "/superusers", userHandler);

	db->connect("server.db");

	logger->critical("HTTPS Server starting...");
	std::shared_ptr<IServer> pServer = std::make_shared<SyncHTTPServer>(logger, std::dynamic_pointer_cast<SqliteDatabase>(db), std::dynamic_pointer_cast<IRequestHandler>(router));

	pServer->start();

	std::this_thread::sleep_for(std::chrono::seconds(1000));

	logger->critical("HTTPS Server stopped.");

	return 0;
}