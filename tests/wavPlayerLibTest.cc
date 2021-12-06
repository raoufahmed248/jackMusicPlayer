#include <gtest/gtest.h>

#include "wavPlayer.h"

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
  wavPlayer test("test.wav",10,10);
  EXPECT_EQ(test.returnFour(), 4);
  
}
