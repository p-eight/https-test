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

    /* transaction */
    m_prepared_statements.emplace("begin_transaction", create_preparated_query(m_db.get(), sql::transactions::begin_transaction, m_logger));
    m_prepared_statements.emplace("commit_transaction", create_preparated_query(m_db.get(), sql::transactions::commit_transaction, m_logger));
    m_prepared_statements.emplace("rollback_transaction", create_preparated_query(m_db.get(), sql::transactions::rollback_transaction, m_logger));

   /* IClientRepository */
   m_prepared_statements.emplace("insert_client", create_preparated_query(m_db.get(), sql::client::insert_client, m_logger));
   m_prepared_statements.emplace("remove_client_by_id", create_preparated_query(m_db.get(), sql::client::remove_client_by_id, m_logger));
   m_prepared_statements.emplace("remove_all_clients", create_preparated_query(m_db.get(), sql::client::remove_all, m_logger));
   m_prepared_statements.emplace("increment_client_connection_count", create_preparated_query(m_db.get(), sql::client::increment_client_connection_count, m_logger));
   m_prepared_statements.emplace("increment_client_event_count", create_preparated_query(m_db.get(), sql::client::increment_client_event_count, m_logger));
   m_prepared_statements.emplace("select_client_by_id", create_preparated_query(m_db.get(), sql::client::select_client_by_id, m_logger));
   m_prepared_statements.emplace("select_client_by_ip", create_preparated_query(m_db.get(), sql::client::select_client_by_ip, m_logger));
   m_prepared_statements.emplace("select_client_ip_by_id", create_preparated_query(m_db.get(), sql::client::select_client_ip_by_id, m_logger));
   m_prepared_statements.emplace("select_client_id_by_ip", create_preparated_query(m_db.get(), sql::client::select_client_id_by_ip, m_logger));
   m_prepared_statements.emplace("count_all", create_preparated_query(m_db.get(), sql::client::count_all, m_logger));
   m_prepared_statements.emplace("select_all", create_preparated_query(m_db.get(), sql::client::select_all, m_logger));
}

// IDatabase interface
bool SqliteDatabase::connect(const std::string& connection_string)
{
	if (m_db) {
		m_logger->info("Database already connected");
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
		m_db_path = ":memory:";
		m_logger->info("Using in-memory SQLite database");
	}
    return this->connect();
}

bool SqliteDatabase::connect()
{
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
    if (!begin_transaction())
    {
        return -1;
    }

    auto stmt = m_prepared_statements["insert_client"].get();
    sqlite3_stmt_resetter reset(stmt);

    if (!stmt)
    {
        m_logger->error("Failed to prepare statement for adding client");
        rollback_transaction();
        return -1;
    }

    sqlite3_bind_int(stmt, 1, client_ip);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        m_logger->error("Failed to add client: {}", sqlite3_errmsg(m_db.get()));
        rollback_transaction();
        return -1;
    }
    int client_id = static_cast<int>(sqlite3_last_insert_rowid(m_db.get()));
    m_logger->info("Client added with ID: {}", client_id);

    if (!commit_transaction()) 
    {
        return -1;
    }

    return client_id;
}

bool SqliteDatabase::remove_client(int client_id)
{
    if (!begin_transaction())
    {
        return false;
    }

   auto stmt = m_prepared_statements["remove_client_by_id"].get();
   sqlite3_stmt_resetter reset(stmt);

   if (!stmt) {
		m_logger->error("Failed to prepare statement for removing client");
        rollback_transaction();
		return false;
	}

   sqlite3_bind_int(stmt, 1, client_id);
	int rc = sqlite3_step(stmt);
	if (rc == SQLITE_DONE) 
    {
		return commit_transaction();
	}
   
	m_logger->error("Failed to remove client: {} - ", client_id, sqlite3_errmsg(m_db.get()));

    rollback_transaction();
	return false;
}

bool SqliteDatabase::remove_all_clients()
{
    if (!begin_transaction())
    {
        return false;
    }

	auto stmt = m_prepared_statements["remove_all_clients"].get();
	sqlite3_stmt_resetter reset(stmt);

	if (!stmt) {
		m_logger->error("Failed to prepare statement for removing all clients");
        rollback_transaction();
		return false;
	}

	int rc = sqlite3_step(stmt);
	if (rc == SQLITE_DONE)
	{
		return commit_transaction();
	}

	m_logger->error("Failed to remove all clients: {}", sqlite3_errmsg(m_db.get()));
    rollback_transaction();
	return false;
}

bool SqliteDatabase::increment_connection_count(int client_id)
{
    if (!begin_transaction())
    {
        m_logger->error("Failed to begin transaction for increment connection count");
        return false;
    }

    auto stmt = m_prepared_statements["increment_client_connection_count"].get();
    sqlite3_stmt_resetter reset(stmt);
    if (!stmt)
    {
        m_logger->error("Failed to prepare statement for increment connection count");
        rollback_transaction();
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);
    auto rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        m_logger->error("Failed to increment connection count for client {}: {}", client_id, sqlite3_errmsg(m_db.get()));
        rollback_transaction();
        return false;
    }

    if (!commit_transaction())
    {
        m_logger->error("Failed to commit transaction for increment connection count");
        return false;
    }

    return true;
}

bool SqliteDatabase::increment_event_count(int client_id)
{
    if (!begin_transaction())
    {
        m_logger->error("Failed to begin transaction for increment event count");
        return false;
    }

    auto stmt = m_prepared_statements["increment_client_event_count"].get();
    sqlite3_stmt_resetter reset(stmt);
    if (!stmt)
    {
        m_logger->error("Failed to prepare statement for increment event count");
        rollback_transaction();
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);
    auto rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        m_logger->error("Failed to increment event count for client {}: {}", client_id, sqlite3_errmsg(m_db.get()));
        rollback_transaction();
        return false;
    }

    if (!commit_transaction())
    {
        m_logger->error("Failed to commit transaction for increment event count");
        return false;
    }

    return true;
}

IClientRepository::client SqliteDatabase::get_client_by_id(int client_id)
{
    auto stmt = m_prepared_statements["select_client_by_id"].get();
    sqlite3_stmt_resetter reset(stmt);
    if (!stmt)
    {
        m_logger->error("Failed to prepare statement for selecting client by ID");
        return client();
    }
    sqlite3_bind_int(stmt, 1, client_id);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW)
    {
        m_logger->error("Failed to select client by ID {}: {}", client_id, sqlite3_errmsg(m_db.get()));
        return client();
    }
    IClientRepository::client client;
    client.id = sqlite3_column_int(stmt, 0);
    client.ip = sqlite3_column_int(stmt, 1);
    client.connection_count = sqlite3_column_int(stmt, 2);
    client.event_count = sqlite3_column_int(stmt, 3);
	return client;
}

IClientRepository::client SqliteDatabase::get_client_by_ip(int client_ip)
{
    auto stmt = m_prepared_statements["select_client_by_ip"].get();
    sqlite3_stmt_resetter reset(stmt);
    if (!stmt)
    {
        m_logger->error("Failed to prepare statement for selecting client by ID");
        return client();
    }
    sqlite3_bind_int(stmt, 1, client_ip);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW)
    {
        m_logger->error("Failed to select client by ID {}: {}", client_ip, sqlite3_errmsg(m_db.get()));
        return client();
    }
    IClientRepository::client client;
    client.id = sqlite3_column_int(stmt, 0);
    client.ip = sqlite3_column_int(stmt, 1);
    client.connection_count = sqlite3_column_int(stmt, 2);
    client.event_count = sqlite3_column_int(stmt, 3);
    return client;
}

int SqliteDatabase::get_client_id_by_ip(int client_ip)
{
    auto stmt = m_prepared_statements["select_client_id_by_ip"].get();
    sqlite3_stmt_resetter reset(stmt);
    if (!stmt)
    {
        m_logger->error("Failed to prepare statement for selecting client by ID");
        return -1;
    }
    sqlite3_bind_int(stmt, 1, client_ip);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW)
    {
        m_logger->error("Failed to select client by ID {}: {}", client_ip, sqlite3_errmsg(m_db.get()));
        return -1;
    }
    auto client_id = sqlite3_column_int(stmt, 0);

    return client_id;
}

int SqliteDatabase::get_client_ip_by_id(int client_id)
{
    auto stmt = m_prepared_statements["select_client_ip_by_id"].get();
    sqlite3_stmt_resetter reset(stmt);
    if (!stmt)
    {
        m_logger->error("Failed to prepare statement for selecting client by ID");
        return -1;
    }
    sqlite3_bind_int(stmt, 1, client_id);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW)
    {
        m_logger->error("Failed to select client by ID {}: {}", client_id, sqlite3_errmsg(m_db.get()));
        return -1;
    }
    auto client_ip = sqlite3_column_int(stmt, 0);

    return client_ip;
}

int SqliteDatabase::get_client_count()
{
    auto stmt = m_prepared_statements["count_all"].get();
	sqlite3_stmt_resetter reset(stmt);

	if (!stmt)
	{
		m_logger->error("Failed to prepare statement for count all clients");
		return -1;
	}

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW)
	{
		m_logger->error("Failed to count all clients: {}", sqlite3_errmsg(m_db.get()));
		return -1;
	}
	
	int client_count = sqlite3_column_int(stmt, 0);
	m_logger->trace("Clients count on database : {}", client_count);

	return client_count;
}

std::vector<IClientRepository::client> SqliteDatabase::get_all_clients()
{
    std::vector<IClientRepository::client> clients;
    auto stmt = m_prepared_statements["select_all"].get();
    sqlite3_stmt_resetter reset(stmt);
	if (!stmt)
	{
        m_logger->error("Failed to prepare statement for selecting all clients");
        return clients;
	}
	int rc = sqlite3_step(stmt);
	while (SQLITE_ROW == rc)
	{
        IClientRepository::client c;
        c.id = sqlite3_column_int(stmt, 0);
        c.ip = sqlite3_column_int(stmt, 1);
        c.connection_count = sqlite3_column_int(stmt, 2);
        c.event_count = sqlite3_column_int(stmt, 3);
        clients.push_back(c);
        rc = sqlite3_step(stmt);
	}
    if (rc != SQLITE_DONE)
    {
        m_logger->error("Failed to select all clients: {}", sqlite3_errmsg(m_db.get()));
        clients.clear();
    }
    else 
    {
        m_logger->trace("Retrieved {} clients from database", clients.size());
    }

	return clients;
}

// Inline transaction management functions
inline bool SqliteDatabase::begin_transaction()
{
    auto stmt = m_prepared_statements["begin_transaction"].get();
    sqlite3_stmt_resetter reset(stmt);

    if (!stmt) {
        m_logger->error("Failed to prepare statement for begin transaction");
        return false;
    }

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        m_logger->error("Failed to begin transaction: {}", sqlite3_errmsg(m_db.get()));
        return false;
    }

    return true;
}

inline bool SqliteDatabase::commit_transaction()
{
    auto stmt = m_prepared_statements["commit_transaction"].get();
    sqlite3_stmt_resetter reset(stmt);

    if (!stmt) {
        m_logger->error("Failed to prepare statement for commit transaction");
        return false;
    }

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        m_logger->error("Failed to commit transaction: {}", sqlite3_errmsg(m_db.get()));
        return false;
    }

    return true;
}

inline bool SqliteDatabase::rollback_transaction()
{
    auto stmt = m_prepared_statements["rollback_transaction"].get();
    sqlite3_stmt_resetter reset(stmt);

    if (!stmt) {
        m_logger->error("Failed to prepare statement for rollback transaction");
        return false;
    }

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        m_logger->error("Failed to rollback transaction: {}", sqlite3_errmsg(m_db.get()));
        return false;
    }

    return true;
}

//int SqliteDatabase::add_client_connection(int client_id, int connection_timestamp)
//{
//
//}
//
//bool SqliteDatabase::remove_connection(int connection_id)
//{
//
//}
//
//bool SqliteDatabase::remove_client_connections(int connection_id)
//{
//
//}
//
//bool SqliteDatabase::remove_all_client_connections()
//{
//
//}
//
//int SqliteDatabase::get_client_connection_count(int client_id)
//{
//
//}
