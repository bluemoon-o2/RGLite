// Test file to verify TEST_F macro and fixture functionality
#include <iostream>
#include <string>
#include "TestFramework.h"

// Simple test fixture
class SimpleFixture : public testing::TestFixture {
public:
    int counter;
    
    void SetUp() override {
        counter = 42;
    }
    
    void TearDown() override {
        counter = 0;
    }
};

// Test using TEST_F macro with SimpleFixture
TEST_F(SimpleTestSuite, SimpleTest, SimpleFixture) {
    EXPECT_EQ(counter, 42);
}

int main(int argc, char* argv[]) {
    testing::TestRunner runner;
    runner.RunAllTests(argc, argv);
    return 0;
}