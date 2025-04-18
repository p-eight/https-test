#include "pch.h"

namespace DefaultCollectionTest
{ 
	TEST(ADefaultCollectionTest, TestTrue) {
	  EXPECT_EQ(1, 1);
	  EXPECT_TRUE(true);

	}

	TEST(ADefaultCollectionTest, TestFalse) {
		EXPECT_NE(1, 2);
		EXPECT_FALSE(false);
	}
}