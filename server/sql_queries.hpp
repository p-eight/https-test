#pragma once
#include <string_view>

namespace sql
{
    namespace transactions
    {
        inline constexpr std::string_view begin_transaction = "BEGIN TRANSACTION";
        inline constexpr std::string_view commit_transaction = "COMMIT";
        inline constexpr std::string_view rollback_transaction = "ROLLBACK";
    }

   namespace client
   {

      inline constexpr std::string_view create_table = R"sql(
         CREATE TABLE IF NOT EXISTS client (
         id INTEGER PRIMARY KEY,
         ip INTEGER NOT NULL UNIQUE,
         connection_count INTEGER DEFAULT 0,
         event_count INTEGER DEFAULT 0,
         created_at INTEGER NOT NULL DEFAULT (strftime('%s','now'))
         );
         )sql";

      inline constexpr std::string_view insert_client = R"sql(
         INSERT INTO client (ip) VALUES (?);
         )sql";
      
      inline constexpr std::string_view remove_client_by_id = R"sql(
         DELETE FROM client WHERE id = ?;
         )sql";
      
      inline constexpr std::string_view remove_all = R"sql(
         DELETE FROM client;
         )sql";
      
      inline constexpr std::string_view increment_client_connection_count = R"sql(
         UPDATE client SET connection_count = connection_count + 1 WHERE id = ?;
         )sql";
      
      inline constexpr std::string_view increment_client_event_count = R"sql(
         UPDATE client SET event_count = event_count + 1 WHERE id = ?;
         )sql";

      inline constexpr std::string_view select_client_by_id = R"sql(
         SELECT id, ip, connection_count, event_count FROM client WHERE id = ?;
         )sql";

      inline constexpr std::string_view select_client_by_ip = R"sql(
         SELECT id, ip, connection_count, event_count FROM client WHERE ip = ?;
         )sql";

      inline constexpr std::string_view select_client_ip_by_id = R"sql(
         SELECT ip FROM client WHERE id = ?;
         )sql";

      inline constexpr std::string_view select_client_id_by_ip = R"sql(
         SELECT id FROM client WHERE ip = ?;
         )sql";
      
      inline constexpr std::string_view count_all = R"sql(
         SELECT COUNT(*) FROM client;
         )sql";
      
      inline constexpr std::string_view select_all  = R"sql(
         SELECT id, ip, connection_count, event_count FROM client;
         )sql";
}

   namespace client_connection
   {
      inline constexpr std::string_view create_table = R"sql(
         CREATE TABLE IF NOT EXISTS ClientConnection (
         id INTEGER PRIMARY KEY,
         client_id INTEGER NOT NULL,
         connection_timestamp INTEGER NOT NULL DEFAULT (strftime('%s','now')),
         FOREIGN KEY (client_id) REFERENCES client(id) ON DELETE CASCADE
         );
         )sql";

      inline constexpr std::string_view insert_client_connection = R"sql(
            INSERT INTO ClientConnection (client_id) VALUES (?);
            )sql";
      inline constexpr std::string_view remove_connection_by_id = R"sql(
         DELETE FROM ClientConnection WHERE id = ?;
         )sql";

      inline constexpr std::string_view remove_client_connections = R"sql(
         DELETE FROM ClientConnection WHERE client_id = ?;
         )sql";

      inline constexpr std::string_view remove_all_connections = R"sql(
         DELETE FROM ClientConnection;
         )sql";

      inline constexpr std::string_view get_client_connection_count = R"sql(
            SELECT COUNT(*) FROM ClientConnection WHERE client_id = ?;
            )sql";

   }

   namespace event
   {
      inline constexpr std::string_view create_table = R"sql(
         CREATE TABLE IF NOT EXISTS Event (
         id INTEGER PRIMARY KEY,
         client_id INTEGER NOT NULL,
         connection_id INTEGER NOT NULL,
         timestamp INTEGER NOT NULL DEFAULT (strftime('%s','now')),
         event_data TEXT NOT NULL,
         event_response TEXT NOT NULL,
         FOREIGN KEY (client_id) REFERENCES client(id) ON DELETE CASCADE,
         FOREIGN KEY (connection_id) REFERENCES ClientConnection(id) ON DELETE CASCADE
         );
         )sql";
   }

} // namespace sql
