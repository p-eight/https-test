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

class UserHandler : public IRequestHandler {
public:
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

		return response;
	}
private:
	HttpResponse handlePOSTUsers(const IRequest& request) {
		return HttpResponse();
	}

	HttpResponse handleGETSuperusers()
	{
		HttpResponse res;
		res.setStatusCode(200);
		res.sethttpVersion("HTTP/1.1");
		res.setHeader("Content-Type", "application/json");
		res.setHeader("Connection", "Keep-Alive");
		json responseJson;
		json users;
		for (const auto& user : file) {
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

	json file;
    std::chrono::steady_clock::time_point start;
};


int main()
{
	std::shared_ptr<ILogger> logger = std::make_shared<ConsoleLogger>();
	std::shared_ptr<IDatabase> db = std::make_shared<SqliteDatabase>(logger);
	std::shared_ptr<SqliteDatabase> sqliteDb = std::dynamic_pointer_cast<SqliteDatabase>(db);
	std::shared_ptr<IRequestHandler> helloHandler = std::make_shared<HelloHandler>();
	std::shared_ptr<IRequestHandler> userHandler = std::make_shared<UserHandler>();
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