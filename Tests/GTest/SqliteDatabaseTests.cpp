#include "gtest/gtest.h"
#include "SqliteDatabase.hpp"
#include "ILoggerMock.hpp"


using ::testing::Matcher;
using ::testing::Truly;

namespace SqliteDatabaseTests
{
    TEST(SqliteDatabaseTest, DummyTest)
    {
        EXPECT_TRUE(true);
    }

    TEST(FmtSanity, FormatString) {
        std::string path = "my.db";
        auto result = fmt::format("Opening: {}", path);
        ASSERT_EQ(result, "Opening: my.db");
    }

    TEST(SqliteDatabaseTest, ThrowExceptionOnNullLogger)
    {
        EXPECT_THROW(SqliteDatabase db(nullptr, ""), std::runtime_error);
    }

    TEST(SqliteDatabaseTest, ConnectDisconnect)
    {
        std::string db_path = "test.db";

        auto logger = std::make_shared<ILoggerMock>();

        ON_CALL(*logger, info(::testing::_)).WillByDefault(::testing::Return());

        EXPECT_CALL(*logger, info(::testing::StartsWith("Connecting to SQLite database"))).Times(1);
        EXPECT_CALL(*logger, info(::testing::StartsWith("Disconnected from SQLite database"))).Times(1);

        SqliteDatabase db(logger, db_path);
        EXPECT_TRUE(db.connect(db_path));
    }

    class SqliteDatabaseClassTest : public ::testing::Test
    {
    protected:
        std::shared_ptr<ILoggerMock> m_logger;
        std::unique_ptr<SqliteDatabase> m_db;

        int client_ip1 = 123456789;
        int client_ip2 = 987654321;
        int client_ip3 = 987654320;

        void SetUp() override
        {
            m_logger = std::make_shared<ILoggerMock>();
            m_db = std::make_unique<SqliteDatabase>(m_logger, ":memory:");
            m_db->connect();
        };
    };

    TEST_F(SqliteDatabaseClassTest, AddsOneClient)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called
        
        int client_id = m_db->add_client(client_ip1);
        EXPECT_EQ(client_id, 1); // Assuming first client gets ID 1
        EXPECT_EQ(m_db->get_client_count(), 1); // Assuming first client gets ID 1
    }

    TEST_F(SqliteDatabaseClassTest, RemovesOneClient)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called

        int client_id = m_db->add_client(client_ip1);
        EXPECT_EQ(client_id, 1); // Assuming first client gets ID 1

        EXPECT_TRUE(m_db->remove_client(client_id)); // Remove the client
        EXPECT_EQ(m_db->get_client_count(), 0); // Client count should be 0 after removal
    }

    TEST_F(SqliteDatabaseClassTest, AddsMultipleClients)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called

        int client_id1 = m_db->add_client(client_ip1);
        int client_id2 = m_db->add_client(client_ip2);
        EXPECT_EQ(client_id1, 1); // Assuming first client gets ID 1
        EXPECT_EQ(client_id2, 2); // Assuming second client gets ID 2
        EXPECT_EQ(m_db->get_client_count(), 2); // Two clients added
    }

    TEST_F(SqliteDatabaseClassTest, RemovesMultipleClients)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called

        int client_id1 = m_db->add_client(client_ip1);
        int client_id2 = m_db->add_client(client_ip2);
        int client_id3 = m_db->add_client(client_ip3);

        EXPECT_TRUE(m_db->remove_client(client_id1)); // Remove the client
        EXPECT_EQ(m_db->get_client_count(), 2); // Client count should be 2 after removal

        EXPECT_TRUE(m_db->remove_client(client_id2)); // Remove the client
        EXPECT_EQ(m_db->get_client_count(), 1); // Client count should be 2 after removal
    }

    TEST_F(SqliteDatabaseClassTest, RemovesAllClients)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called

        int client_id1 = m_db->add_client(client_ip1);
        int client_id2 = m_db->add_client(client_ip2);
        int client_id3 = m_db->add_client(client_ip3);

        EXPECT_TRUE(m_db->remove_all_clients());
        EXPECT_EQ(m_db->get_client_count(), 0); // Client count should be 0 after removal
    }

    TEST_F(SqliteDatabaseClassTest, AddsMillionClients)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called
        EXPECT_CALL(*m_logger, info(::testing::_)).Times(1001);
        for (int i = 0; i < 1000; ++i)
        {
            int client_id = m_db->add_client(client_ip1 + i);
            EXPECT_EQ(client_id, i + 1); // Assuming first client gets ID 1
        }
        EXPECT_EQ(m_db->get_client_count(), 1000);
    }

    TEST_F(SqliteDatabaseClassTest, IdAfterRemoveClient)
    {
        int client_id1 = m_db->add_client(client_ip1);
        EXPECT_TRUE(m_db->remove_client(client_id1));

        int client_id2 = m_db->add_client(client_ip2);
        EXPECT_EQ(client_id2, 1);
        EXPECT_EQ(m_db->get_client_count(), 1);

        int client_id3 = m_db->add_client(client_ip3);
        EXPECT_EQ(client_id3, 2);
        EXPECT_EQ(m_db->get_client_count(), 2);
    }

    TEST_F(SqliteDatabaseClassTest, GetClientById)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called
        int client_id = m_db->add_client(client_ip1);
        auto client = m_db->get_client_by_id(client_id);
        EXPECT_EQ(client.id, client_id);
        EXPECT_EQ(client.ip, client_ip1);
    }

    TEST_F(SqliteDatabaseClassTest, GetClientByIp)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called
        int client_id = m_db->add_client(client_ip1);
        auto client = m_db->get_client_by_ip(client_ip1);
        EXPECT_EQ(client.id, client_id);
        EXPECT_EQ(client.ip, client_ip1);
    }

    TEST_F(SqliteDatabaseClassTest, GetAllClients)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called
        m_db->add_client(client_ip1);
        m_db->add_client(client_ip2);
        m_db->add_client(client_ip3);
        auto clients = m_db->get_all_clients();
        EXPECT_EQ(clients.size(), 3);
        EXPECT_EQ(clients[0].ip, client_ip1);
        EXPECT_EQ(clients[1].ip, client_ip2);
        EXPECT_EQ(clients[2].ip, client_ip3);
    }

    TEST_F(SqliteDatabaseClassTest, IncrementClientConnections)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called
        auto client_id = m_db->add_client(client_ip1);
        for (auto i = 0; i < 10; ++i)
        {
            EXPECT_TRUE(m_db->increment_connection_count(client_id));
        }

        auto client = m_db->get_client_by_id(client_id);
        EXPECT_EQ(client.ip, client_ip1);
        EXPECT_EQ(client.connection_count, 10);
    }

    TEST_F(SqliteDatabaseClassTest, IncrementClientEvents)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called
        auto client_id = m_db->add_client(client_ip1);
        for (auto i = 0; i < 10; ++i)
        {
            EXPECT_TRUE(m_db->increment_event_count(client_id));
        }

        auto client = m_db->get_client_by_id(client_id);
        EXPECT_EQ(client.event_count, 10);
    }

    TEST_F(SqliteDatabaseClassTest, GetClientIDByIP)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called
        auto id = m_db->add_client(client_ip1);
        auto get_id = m_db->get_client_id_by_ip(client_ip1);
        EXPECT_EQ(id, get_id);
    }

    TEST_F(SqliteDatabaseClassTest, GetClientIPByID)
    {
        EXPECT_CALL(*m_logger, error(::testing::_)).Times(0); // fail if called
        auto client_id = m_db->add_client(client_ip1);
        auto client_ip = m_db->get_client_ip_by_id(client_id);
        EXPECT_EQ(client_ip, client_ip1);
    }
}