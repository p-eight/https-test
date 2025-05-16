#include "SqliteDatabase.hpp"
#include "sql_queries.hpp"

sqlite3_stmt* create_preparated_query(sqlite3* pDB, std::string_view query, std::shared_ptr<ILogger> logger)
{
	sqlite3_stmt* raw_stmt = nullptr;
	int rc = sqlite3_prepare_v3( pDB, query.data(), static_cast<int>(query.size()), SQLITE_PREPARE_PERSISTENT, &raw_stmt, nullptr);
	if (rc != SQLITE_OK)
	{
		logger->error("Failed to prepare statement (\'{}\'): {}", query.data(), sqlite3_errmsg(pDB));
		return nullptr;
	}

	return raw_stmt; // wrapped in smart pointer
}

void SqliteDatabase::CreateDatabase()
{
   /* create tables */
	m_prepared_statements.emplace("create_client_table", create_preparated_query(m_db.get(), sql::client::create_table, m_logger));
	execute_statament("create_client_table");
	m_prepared_statements.emplace("create_client_connection_table", create_preparated_query(m_db.get(), sql::client_connection::query_create_table, m_logger));
	execute_statament("create_client_connection_table");
	m_prepared_statements.emplace("create_event_table", create_preparated_query(m_db.get(), sql::event::query_create_table, m_logger));
	execute_statament("create_event_table");

   /* IClientRepository */
   m_prepared_statements.emplace("insert_client", create_preparated_query(m_db.get(), sql::client::insert_client, m_logger));
   m_prepared_statements.emplace("remove_client_by_id", create_preparated_query(m_db.get(), sql::client::remove_client_by_id, m_logger));
   m_prepared_statements.emplace("remove_all", create_preparated_query(m_db.get(), sql::client::remove_all, m_logger));
   m_prepared_statements.emplace("increment_client_connection_count", create_preparated_query(m_db.get(), sql::client::increment_client_connection_count, m_logger));
   m_prepared_statements.emplace("increment_client_event_count", create_preparated_query(m_db.get(), sql::client::increment_client_event_count, m_logger));
   m_prepared_statements.emplace("select_by_id", create_preparated_query(m_db.get(), sql::client::select_by_id, m_logger));
   m_prepared_statements.emplace("select_by_ip", create_preparated_query(m_db.get(), sql::client::select_by_ip, m_logger));
   m_prepared_statements.emplace("count_all", create_preparated_query(m_db.get(), sql::client::count_all, m_logger));
   m_prepared_statements.emplace("select_all", create_preparated_query(m_db.get(), sql::client::select_all, m_logger));
}

// IDatabase interface
bool SqliteDatabase::connect(const std::string& connection_string)
{
	if (m_db) {
		m_logger->info("Database already connected");
		CreateDatabase();
		return true;
	}

	if (!connection_string.empty())
	{
		m_db_path = connection_string;
		m_logger->info("Connecting to SQLite database at {}", connection_string);
	}
	else {
		m_logger->info("No connection string provided, using default");
	}
	if (m_db_path.empty()) {
		m_db_path = "::memory";
		m_logger->info("Using in-memory SQLite database");
	}

	sqlite3* rawDb = nullptr;
	if (sqlite3_open(m_db_path.c_str(), &rawDb) != SQLITE_OK)
	{
		m_logger->error("Failed to open SQLite database: {}", sqlite3_errmsg(rawDb));
	}
	m_db.reset(rawDb);
	
	CreateDatabase();
	return true;
}

void SqliteDatabase::disconnect()
{
	if (m_db) {
		m_db.reset();
		m_logger->info("Disconnected from SQLite database");
	}
	else {
		m_logger->info("No active database connection to disconnect");
	}
}

// IClientRepository interface
int SqliteDatabase::add_client(int client_ip)
{
	auto stmt = m_prepared_statements["insert_client"].get();
   sqlite3_stmt_resetter reset(stmt);

	if (!stmt) 
   {
		m_logger->error("Failed to prepare statement for adding client");
		return -1;
	}

	sqlite3_bind_int(stmt, 1, client_ip);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) 
   {
		m_logger->error("Failed to add client: {}", sqlite3_errmsg(m_db.get()));
		return -1;
	}
	int client_id = static_cast<int>(sqlite3_last_insert_rowid(m_db.get()));
	m_logger->info("Client added with ID: {}", client_id);

	return client_id;
}

bool SqliteDatabase::remove_client(int client_id)
{
   auto stmt = m_prepared_statements["insert_client"].get();
   sqlite3_stmt_resetter reset(stmt);

   if (!stmt) {
		m_logger->error("Failed to prepare statement for removing client");
		return false;
	}

   sqlite3_bind_int(stmt, 1, client_id);
	int rc = sqlite3_step(stmt);
	if (rc == SQLITE_DONE) 
   {
		return true;
	}
   
	m_logger->error("Failed to remove client: {}", sqlite3_errmsg(m_db.get()));
	return false;
}

bool SqliteDatabase::remove_all_clients()
{
	return false;
}

bool SqliteDatabase::increment_connection_count(int client_id)
{
	return false;
}

bool SqliteDatabase::increment_event_count(int client_id)
{
	return false;
}

IClientRepository::client SqliteDatabase::get_client_by_id(int client_id)
{
	return client();
}

IClientRepository::client SqliteDatabase::get_client_by_ip(int client_ip)
{
	return client();
}

int SqliteDatabase::get_client_id_by_ip(int client_ip)
{
	return 0;
}

int SqliteDatabase::get_client_ip_by_id(int client_id)
{
	return 0;
}

int SqliteDatabase::get_client_count()
{
	return 0;
}

std::vector<IClientRepository::client> SqliteDatabase::get_all_clients()
{
	return std::vector<client>();
}
