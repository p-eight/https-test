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
#include "RequestHandlers.hpp"

int main()
{
	std::shared_ptr<ILogger> logger = std::make_shared<ConsoleLogger>();
	std::shared_ptr<IDatabase> db = std::make_shared<SqliteDatabase>(logger);
	std::shared_ptr<SqliteDatabase> sqliteDb = std::dynamic_pointer_cast<SqliteDatabase>(db);
	std::shared_ptr<IRequestHandler> helloHandler = std::make_shared<HelloHandler>();
	std::shared_ptr<IRequestHandler> userHandler = std::make_shared<UserHandler>(std::make_shared<MultipartBodyParser>(), logger);
    std::shared_ptr<RequestRouter> router = std::make_shared<RequestRouter>(logger);

	router->registerHandler("GET", "/hello", helloHandler);
	router->registerHandler("POST", "/users", userHandler);
	router->registerHandler("GET", "/superusers", userHandler);
	router->registerHandler("GET", "/top-countries", userHandler);
	router->registerHandler("GET", "/team-insights", userHandler);
    router->registerHandler("GET", "/active-users-per-day", userHandler);

	db->connect("server.db");

	logger->critical("HTTPS Server starting...");
	std::shared_ptr<IServer> pServer = std::make_shared<SyncHTTPServer>(logger, std::dynamic_pointer_cast<SqliteDatabase>(db), std::dynamic_pointer_cast<IRequestHandler>(router));

	pServer->start();

	std::this_thread::sleep_for(std::chrono::seconds(1000));

	logger->critical("HTTPS Server stopped.");

	return 0;
}