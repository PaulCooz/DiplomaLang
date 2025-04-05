#include "abstract_syntax_tree.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace std;

string program = "a := 1 + 2 * 3"
                 "print a // output 7"
                 "a = (a - 1) * (a + 1)"
                 "print a // last output 48";

TEST(Basic, ThisIsMyFirstTest) {
  auto s = parseSyntaxTree({});
  EXPECT_EQ(s.size(), 0);
}
