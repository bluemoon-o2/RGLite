// Test file to verify TEST_F macro with more complex scenarios
#include <iostream>
#include <string>
#include "TestFramework.h"

// Complex test fixture with multiple members
class ComplexFixture : public testing::TestFixture {
public:
    std::string message;
    int counter;
    std::vector<int> numbers;
    
    void SetUp() override {
        message = "Hello, World!";
        counter = 100;
        numbers = {1, 2, 3, 4, 5};
    }
    
    void TearDown() override {
        message.clear();
        counter = 0;
        numbers.clear();
    }
    
    // Helper method that tests can use
    int sumNumbers() const {
        int sum = 0;
        for (int num : numbers) {
            sum += num;
        }
        return sum;
    }
};

// Test using TEST_F macro with ComplexFixture
TEST_F(ComplexTestSuite, TestMessage, ComplexFixture) {
    EXPECT_EQ(message, "Hello, World!");
    EXPECT_NE(message, "Goodbye");
}

TEST_F(ComplexTestSuite, TestCounter, ComplexFixture) {
    EXPECT_EQ(counter, 100);
    EXPECT_GT(counter, 50);
}

TEST_F(ComplexTestSuite, TestNumbers, ComplexFixture) {
    EXPECT_EQ(numbers.size(), 5);
    EXPECT_EQ(numbers[0], 1);
    EXPECT_EQ(numbers[4], 5);
    EXPECT_EQ(sumNumbers(), 15);
}

// Another test fixture to test multiple fixtures in the same program
class SimpleFixture : public testing::TestFixture {
public:
    int value;
    
    void SetUp() override {
        value = 42;
    }
    
    void TearDown() override {
        value = 0;
    }
};

TEST_F(SimpleTestSuite, TestValue, SimpleFixture) {
    EXPECT_EQ(value, 42);
}

int main(int argc, char* argv[]) {
    testing::TestRunner runner;
    runner.RunAllTests(argc, argv);
    return 0;
}