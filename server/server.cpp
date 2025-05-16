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
	sqliteDb->add_client(123456789);
	sqliteDb->add_client(1);
	sqliteDb->add_client(2);
	sqliteDb->add_client(2);

	logger->critical("HTTPS Server starting...");
	std::shared_ptr<IServer> pServer = std::make_shared<SyncHTTPServer>(logger, std::dynamic_pointer_cast<SqliteDatabase>(db));

	pServer->start();

	std::this_thread::sleep_for(std::chrono::seconds(200));

	logger->critical("HTTPS Server stopped.");

	return 0;
}

// Executar programa: Ctrl + F5 ou Menu Depurar > Iniciar Sem Depuração
// Depurar programa: F5 ou menu Depurar > Iniciar Depuração

// Dicas para Começar: 
//   1. Use a janela do Gerenciador de Soluções para adicionar/gerenciar arquivos
//   2. Use a janela do Team Explorer para conectar-se ao controle do código-fonte
//   3. Use a janela de Saída para ver mensagens de saída do build e outras mensagens
//   4. Use a janela Lista de Erros para exibir erros
//   5. Ir Para o Projeto > Adicionar Novo Item para criar novos arquivos de código, ou Projeto > Adicionar Item Existente para adicionar arquivos de código existentes ao projeto
//   6. No futuro, para abrir este projeto novamente, vá para Arquivo > Abrir > Projeto e selecione o arquivo. sln
