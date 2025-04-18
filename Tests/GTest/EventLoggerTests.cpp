#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "MockDatabase.hpp"
#include "EventLogger.hpp"

using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;

namespace EventLoggerTests {

    TEST(EventLoggerTest, ConstructorThrowsOnNullSharedPtr) {
        std::shared_ptr<IDatabase> null_db;
        EXPECT_THROW({
            EventLogger logger(null_db);
            }, std::invalid_argument);
    }
    TEST(EventLoggerTest, GetClientIdReturnsCorrectId) {
        auto mockDb = std::make_shared<NiceMock<MockDatabase>>();
        EXPECT_CALL(*mockDb, get_client_id_by_ip(12345))
            .Times(1)
            .WillOnce(Return(42));

        EventLogger logger(mockDb);
        EXPECT_EQ(logger.get_client_id(12345), 42);
    }

    TEST(EventLoggerTest, AddClientReturnsClientId) {
        auto mockDb = std::make_shared<NiceMock<MockDatabase>>();
        EXPECT_CALL(*mockDb, add_client(1234)).WillOnce(Return(3));

        EventLogger logger(mockDb);
        EXPECT_EQ(logger.add_client(1234), 3);
    }

    TEST(EventLoggerTest, GetUnexistentClientReturnsError) {
        auto mockDb = std::make_shared<NiceMock<MockDatabase>>();
		EXPECT_CALL(*mockDb, get_client_id_by_ip(12345))
			.Times(1)
			.WillOnce(Return(-1));

		EventLogger logger(mockDb);
		EXPECT_EQ(logger.get_client_id(12345), -1);
    }

    TEST(EventLoggerTest, AddClientAndGetClientReturnEqual) {
		auto mockDb = std::make_shared<NiceMock<MockDatabase>>();
        EXPECT_CALL(*mockDb, add_client(1234)).WillOnce(Return(3));
		EXPECT_CALL(*mockDb, get_client_id_by_ip(1234))
			.Times(1)
			.WillOnce(Return(3));

        EventLogger logger(mockDb);
		auto client_id = logger.add_client(1234);
		auto client_id2 = logger.get_client_id(1234);
		EXPECT_EQ(client_id, client_id2);
    }

    TEST(EventLoggerTest, LogEventRetursSucess) {
        auto mockDb = std::make_shared<NiceMock<MockDatabase>>();
        EXPECT_CALL(*mockDb, get_client_id_by_ip(1234))
            .Times(1)
            .WillOnce(Return(3));
        EXPECT_CALL(*mockDb, add_event(_)).WillOnce(Return(5));
		EventLogger logger(mockDb);
		auto client_id = logger.add_client(1234);
		auto event_id = logger.log_event(1234, 1, "test_event");
		EXPECT_EQ(event_id, 5);
    }
}