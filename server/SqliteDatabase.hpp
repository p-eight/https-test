#pragma once
#include <sqlite/sqlite3.h>
#include "IDatabase.hpp"
#include "IClientRepository.hpp"
#include "IClientConnectionRepository.hpp"
#include "IEventRepository.hpp"
#include "ILogger.hpp"
#include <memory>
#include <stdexcept>
#include <unordered_map>

class SqliteDatabase : public IDatabase, public IClientRepository
{
public:

	SqliteDatabase(std::shared_ptr<ILogger> _log, std::string db_path = "::memory") : m_logger(_log), m_db_path(db_path)
	{
		if (!m_logger)
		{
			throw std::runtime_error("Logger cant be null");
		}
	};

	~SqliteDatabase() override 
	{
		disconnect();
	}

	// IDatabase interface
	bool connect(const std::string& connection_string) override;
	void disconnect() override;

	//IClientRepository interface
	int add_client(int client_ip) override;
	bool remove_client(int client_id) override;
	bool remove_all_clients() override;
	bool increment_connection_count(int client_id) override;
	bool increment_event_count(int client_id) override;
	client get_client_by_id(int client_id) override;
	client get_client_by_ip(int client_ip) override;
	int get_client_id_by_ip(int client_ip) override;
	int get_client_ip_by_id(int client_id) override;
	int get_client_count() override;
	std::vector<client> get_all_clients() override;

	/*//IClientConnectionRepository interface
	int add_client_connection(int client_id, int connection_timestamp) = 0;
	bool remove_client_connection(int connection_id) = 0;
	bool remove_all_client_connections() = 0;
	int get_client_connection_count(int client_id) = 0;

	//IEventRepository interface
	int add_event(int client_id, int connection_id, int timestamp, const std::string& event_data) = 0;
	bool remove_event(int event_id) = 0;
	bool remove_all_events() = 0;
	bool remove_events_by_client(int client_id) = 0;
	std::vector<Event> get_events_by_client(int client_id) = 0;*/



private:
	struct sqlite3_Deleter {
		void operator()(sqlite3* db) const {
			if (db) {
				sqlite3_close(db);
			}
		}
	};
	struct sqlite3_stmt_Deleter {
		void operator()(sqlite3_stmt* stmt) const {
			if (stmt) sqlite3_finalize(stmt);
		}
	};

	using sqlite3_ptr = std::unique_ptr<sqlite3, sqlite3_Deleter>;
	using sqlite3_stmt_ptr = std::unique_ptr<sqlite3_stmt, sqlite3_stmt_Deleter>;
	using sqlite3_stmt_map = std::unordered_map<std::string, sqlite3_stmt_ptr>;

	sqlite3_ptr m_db;
	std::string m_db_path;
	std::shared_ptr<ILogger> m_logger;
	sqlite3_stmt_map m_prepared_statements;

	void CreateDatabase();
	bool execute_statament(const std::string& statement)
	{
		int rc = sqlite3_step(m_prepared_statements[statement].get());
		if (rc != SQLITE_DONE) {
			m_logger->error("Failed to execute statement ({}) : {}", statement, sqlite3_errmsg(m_db.get()));
			return false;
		}
		return true;
	}
};

class sqlite3_stmt_resetter {
    sqlite3_stmt* stmt;

public:
    explicit sqlite3_stmt_resetter(sqlite3_stmt* s) : stmt(s) {}
    ~sqlite3_stmt_resetter() {
        if (stmt) {
            sqlite3_reset(stmt);
        }
    }

    // Optional: allow access to the statement
    sqlite3_stmt* get() const { return stmt; }
    operator sqlite3_stmt*() const { return stmt; }
};