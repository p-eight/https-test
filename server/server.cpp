// server.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include <iostream>
#include "EventLogger.hpp"
#include "SyncHTTPServer.hpp"
#include "ConsoleLogger.hpp"
#include "SqliteDatabase.hpp"

int main()
{
	std::shared_ptr<ILogger> logger = std::make_shared<ConsoleLogger>();
	std::shared_ptr<IDatabase> db = std::make_shared<SqliteDatabase>(logger);
	std::shared_ptr<SqliteDatabase> sqliteDb = std::dynamic_pointer_cast<SqliteDatabase>(db);
	db->connect("server.db");

	logger->critical("HTTPS Server starting...");
	std::shared_ptr<IServer> pServer = std::make_shared<SyncHTTPServer>(logger, std::dynamic_pointer_cast<SqliteDatabase>(db));

	pServer->start();

	std::this_thread::sleep_for(std::chrono::seconds(200));

	logger->critical("HTTPS Server stopped.");

	return 0;
}