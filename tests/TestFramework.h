// Universal Test Framework for RGLite

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <sstream>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <cstring>

namespace testing {

// Assertion exception
class AssertionException : public std::exception {
private:
    std::string message_;
    
public:
    AssertionException(const std::string& file, int line, const std::string& message)
        : message_(file + ":" + std::to_string(line) + ": " + message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
};

// Test result status
enum class TestResult {
    SUCCESS,
    FAILURE,
    SKIPPED
};

// Test fixture base class
class TestFixture {
public:
    virtual ~TestFixture() = default;
    
    // Setup and teardown methods
    virtual void SetUp() {}
    virtual void TearDown() {}
};

// Interface for fixtures that have TestBody method
class TestFixtureWithBody {
public:
    virtual void TestBody() = 0;
    virtual ~TestFixtureWithBody() = default;
};

// Test case information
struct TestCaseInfo {
    std::string name;
    std::string suite_name;
    std::function<void()> test_func;
    std::function<std::unique_ptr<TestFixture>()> fixture_factory;
    TestResult result;
    std::string failure_message;
    std::string param_info; // For parameterized tests
};

// Test registry for automatic test discovery
class TestRegistry {
private:
    static std::vector<TestCaseInfo>& GetTestCases() {
        static std::vector<TestCaseInfo> test_cases;
        return test_cases;
    }
    
    static testing::TestFixture*& GetCurrentFixtureInstance() {
        static testing::TestFixture* current_fixture = nullptr;
        return current_fixture;
    }
    
public:
    static void RegisterTest(const std::string& suite_name, const std::string& test_name, 
                            std::function<void()> test_func,
                            std::function<std::unique_ptr<TestFixture>()> fixture_factory = nullptr) {
        GetTestCases().push_back({
            test_name,
            suite_name,
            test_func,
            fixture_factory,
            TestResult::SUCCESS,
            ""
        });
    }
    
    static const std::vector<TestCaseInfo>& GetAllTests() {
        return GetTestCases();
    }
    
    static void Clear() {
        GetTestCases().clear();
    }
    
    static void SetCurrentFixture(testing::TestFixture* fixture) {
        GetCurrentFixtureInstance() = fixture;
    }
    
    static testing::TestFixture* GetCurrentFixture() {
        return GetCurrentFixtureInstance();
    }
};

// Command line argument parser
class CommandLineParser {
private:
    std::vector<std::string> args_;
    std::map<std::string, std::string> options_;
    std::vector<std::string> positional_;
    
public:
    CommandLineParser(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) {
            args_.push_back(argv[i]);
        }
        Parse();
    }
    
    void Parse() {
        for (size_t i = 0; i < args_.size(); ++i) {
            const std::string& arg = args_[i];
            if (arg.size() >= 2 && arg[0] == '-') {
                if (arg[1] == '-') {
                    // Long option: --option=value or --option value
                    std::string option = arg.substr(2);
                    size_t eq_pos = option.find('=');
                    if (eq_pos != std::string::npos) {
                        std::string key = option.substr(0, eq_pos);
                        std::string value = option.substr(eq_pos + 1);
                        options_[key] = value;
                    } else {
                        options_[option] = "";
                    }
                } else {
                    // Short option: -o value or -ovalue
                    std::string option = arg.substr(1);
                    if (option.size() > 1 && option[1] != '=') {
                        // Single character option with value: -o value
                        options_[std::string(1, option[0])] = option.substr(1);
                    } else {
                        options_[option] = "";
                    }
                }
            } else {
                positional_.push_back(arg);
            }
        }
    }
    
    bool HasOption(const std::string& option) const {
        return options_.find(option) != options_.end();
    }
    
    std::string GetOption(const std::string& option, const std::string& default_value = "") const {
        auto it = options_.find(option);
        if (it != options_.end()) {
            return it->second;
        }
        return default_value;
    }
    
    const std::vector<std::string>& GetPositionalArgs() const {
        return positional_;
    }
    
    void PrintHelp() const {
        // Help output - kept as it's user-requested information, not debug info
        std::cout << "Test Framework Usage:" << std::endl;
        std::cout << "  --help, -h              Show this help message" << std::endl;
        std::cout << "  --list-tests, -l        List all available tests" << std::endl;
        std::cout << "  --filter=pattern, -f    Run only tests matching pattern" << std::endl;
        std::cout << "  --verbose, -v           Enable verbose output" << std::endl;
        std::cout << "  --output=format         Set output format (text, xml, json)" << std::endl;
        std::cout << "  --repeat=count          Repeat tests specified number of times" << std::endl;
    }
};

// Test runner
class TestRunner {
private:
    int total_tests_ = 0;
    int passed_tests_ = 0;
    int failed_tests_ = 0;
    int skipped_tests_ = 0;
    bool verbose_ = false;
    std::string output_format_ = "text";
    int repeat_count_ = 1;
    std::vector<std::string> failed_test_names_; // Store names of failed tests
    
public:
    void RunAllTests(int argc = 0, char* argv[] = nullptr) {
        CommandLineParser parser(argc, argv);
        
        // Handle help option
        if (parser.HasOption("help") || parser.HasOption("h")) {
            parser.PrintHelp();
            return;
        }
        
        // Handle list tests option
        if (parser.HasOption("list-tests") || parser.HasOption("l")) {
            ListTests();
            return;
        }
        
        // Parse other options
        verbose_ = parser.HasOption("verbose") || parser.HasOption("v");
        output_format_ = parser.GetOption("output", "text");
        repeat_count_ = std::max(1, std::stoi(parser.GetOption("repeat", "1")));
        
        std::string filter_pattern = parser.GetOption("filter", "");
        if (filter_pattern.empty() && parser.HasOption("f")) {
            filter_pattern = parser.GetOption("f", "");
        }
        
        const auto& test_cases = TestRegistry::GetAllTests();
        
        // Filter tests if pattern is provided
        std::vector<TestCaseInfo> filtered_cases;
        if (!filter_pattern.empty()) {
            for (const auto& test_case : test_cases) {
                std::string full_name = test_case.suite_name + "." + test_case.name;
                if (full_name.find(filter_pattern) != std::string::npos) {
                    filtered_cases.push_back(test_case);
                }
            }
        } else {
            filtered_cases = test_cases;
        }
        
        total_tests_ = static_cast<int>(filtered_cases.size());
        
        // Progress and status output - kept as it's user-requested information, not debug info
        if (verbose_) {
            std::cout << "[==========] Running " << total_tests_ << " test(s)." << std::endl;
            if (!filter_pattern.empty()) {
                std::cout << "[  FILTER  ] Pattern: " << filter_pattern << std::endl;
            }
        } else {
            // In non-verbose mode, just show a simple progress indicator
            std::cout << "Running " << total_tests_ << " test(s)..." << std::endl;
        }
        
        for (int repeat = 0; repeat < repeat_count_; ++repeat) {
            if (repeat_count_ > 1 && verbose_) {
                // Repeat iteration output - kept as it's user-requested information, not debug info
                std::cout << "[  REPEAT  ] Iteration " << (repeat + 1) << " of " << repeat_count_ << std::endl;
            }
            
            for (const auto& test_case : filtered_cases) {
                RunTest(test_case);
            }
        }
        
        PrintSummary();
    }
    
    void ListTests() {
        const auto& test_cases = TestRegistry::GetAllTests();
        // Test listing output - kept as it's user-requested information, not debug info
        std::cout << "Available tests (" << test_cases.size() << "):" << std::endl;
        for (const auto& test_case : test_cases) {
            std::cout << "  " << test_case.suite_name << "." << test_case.name;
            if (!test_case.param_info.empty()) {
                std::cout << " (" << test_case.param_info << ")";
            }
            std::cout << std::endl;
        }
    }
    
    void RunTest(const TestCaseInfo& test_case) {
        if (verbose_) {
            // Test start output - kept as it's user-requested information, not debug info
            std::cout << "[ RUN      ] " << test_case.suite_name << "." << test_case.name << std::endl;
        }
        
        try {
            // Create fixture if needed
            std::unique_ptr<TestFixture> fixture;
            if (test_case.fixture_factory) {
                fixture = test_case.fixture_factory();
                if (fixture) {
                    fixture->SetUp();
                    // Set the current fixture instance for TEST_F tests
                    TestRegistry::SetCurrentFixture(fixture.get());
                }
            }
            
            // Run the test
            test_case.test_func();
            
            // Cleanup fixture
            if (fixture) {
                fixture->TearDown();
                // Clear the current fixture instance
                TestRegistry::SetCurrentFixture(nullptr);
            }
            
            if (verbose_) {
                // Test success output - kept as it's user-requested information, not debug info
                std::cout << "[       OK ] " << test_case.suite_name << "." << test_case.name << std::endl;
            }
            passed_tests_++;
        } catch (const testing::AssertionException& e) {
            if (verbose_) {
                // Test failure output - kept as it's user-requested information, not debug info
                std::cout << "[  FAILED  ] " << test_case.suite_name << "." << test_case.name << std::endl;
                std::cout << "Assertion failed: " << e.what() << std::endl;
            }
            // Store the failed test name
            failed_test_names_.push_back(test_case.suite_name + "." + test_case.name);
            failed_tests_++;
        } catch (const std::exception& e) {
            if (verbose_) {
                // Test exception output - kept as it's user-requested information, not debug info
                std::cout << "[  FAILED  ] " << test_case.suite_name << "." << test_case.name << std::endl;
                std::cout << "Unexpected exception: " << e.what() << std::endl;
            }
            // Store the failed test name
            failed_test_names_.push_back(test_case.suite_name + "." + test_case.name);
            failed_tests_++;
        }
    }
    
    void PrintSummary() {
        // Test summary output - kept as it's user-requested information, not debug info
        std::cout << "[==========] " << total_tests_ << " test(s) run." << std::endl;
        std::cout << "[  PASSED  ] " << passed_tests_ << " test(s)." << std::endl;
        if (failed_tests_ > 0) {
            std::cout << "[  FAILED  ] " << failed_tests_ << " test(s)." << std::endl;
            // Print the names of failed tests with better formatting
            std::cout << std::endl << "Failed tests:" << std::endl;
            for (size_t i = 0; i < failed_test_names_.size(); ++i) {
                std::cout << "  [" << (i + 1) << "] " << failed_test_names_[i] << std::endl;
            }
            std::cout << std::endl;
        }
        if (skipped_tests_ > 0) {
            std::cout << "[ SKIPPED  ] " << skipped_tests_ << " test(s)." << std::endl;
        }
    }
    
    int GetExitCode() const {
        return static_cast<int>(failed_tests_ > 0 ? 1 : 0);
    }
};

// Internal assertion implementation
namespace internal {

inline void AssertImpl(bool condition, const std::string& file, int line, 
                      const std::string& message) {
    if (!condition) {
        throw AssertionException(file, line, message);
    }
}

// Helper template to check if types are signed/unsigned integral types
template<typename T>
struct is_integral_type {
    static constexpr bool value = std::is_integral<T>::value;
};

// Specialized version for signed/unsigned comparisons
template<typename T1, typename T2>
inline typename std::enable_if<is_integral_type<T1>::value && is_integral_type<T2>::value && 
                              (std::is_signed<T1>::value != std::is_signed<T2>::value), void>::type
AssertEqualImpl(const T1& expected, const T2& actual, 
               const std::string& file, int line) {
    // For signed/unsigned comparisons, use the larger type to avoid warnings
    using CommonType = typename std::common_type<T1, T2>::type;
    if (static_cast<CommonType>(expected) != static_cast<CommonType>(actual)) {
        std::ostringstream oss;
        oss << "Expected: " << expected << "\n"
            << "Actual: " << actual;
        throw AssertionException(file, line, oss.str());
    }
}

// General version for other types
template<typename T1, typename T2>
inline typename std::enable_if<!is_integral_type<T1>::value || !is_integral_type<T2>::value || 
                              (std::is_signed<T1>::value == std::is_signed<T2>::value), void>::type
AssertEqualImpl(const T1& expected, const T2& actual, 
               const std::string& file, int line) {
    if (expected != actual) {
        std::ostringstream oss;
        oss << "Expected: " << expected << "\n"
            << "Actual: " << actual;
        throw AssertionException(file, line, oss.str());
    }
}

template<typename T1, typename T2>
inline void AssertNotEqualImpl(const T1& expected, const T2& actual, 
                              const std::string& file, int line) {
    if (expected == actual) {
        std::ostringstream oss;
        oss << "Expected not equal to: " << expected << "\n"
            << "Actual: " << actual;
        throw AssertionException(file, line, oss.str());
    }
}

} // namespace internal

} // namespace testing

// Test registration macros
#define TEST_SUITE(suite_name) \
    namespace suite_name##_test_suite

#define TEST_FIXTURE(suite_name, fixture_name) \
    class fixture_name : public testing::TestFixture

#define TEST(suite_name, test_name) \
    class suite_name##_##test_name##_Test : public testing::TestFixture { \
    public: \
        void TestBody(); \
    }; \
    static void suite_name##_##test_name##_TestBody() { \
        suite_name##_##test_name##_Test test; \
        test.TestBody(); \
    } \
    static struct suite_name##_##test_name##_Test_Register { \
        suite_name##_##test_name##_Test_Register() { \
            testing::TestRegistry::RegisterTest(#suite_name, #test_name, \
                suite_name##_##test_name##_TestBody, \
                []() -> std::unique_ptr<testing::TestFixture> { \
                    return std::make_unique<suite_name##_##test_name##_Test>(); \
                }); \
        } \
    } suite_name##_##test_name##_test_register; \
    void suite_name##_##test_name##_Test::TestBody()

#define TEST_F(suite_name, test_name, fixture_class) \
class suite_name##_##test_name##_Test : public fixture_class { \
public: \
    void TestBody(); \
}; \
 \
static void suite_name##_##test_name##_TestBody() { \
    /* Get the fixture instance created by the framework and cast to our test class */ \
    auto* fixture = dynamic_cast<suite_name##_##test_name##_Test*>(testing::TestRegistry::GetCurrentFixture()); \
    if (fixture) { \
        fixture->TestBody(); \
    } \
} \
 \
static struct suite_name##_##test_name##_Test_Register { \
    suite_name##_##test_name##_Test_Register() { \
        testing::TestRegistry::RegisterTest( \
            #suite_name, \
            #test_name, \
            suite_name##_##test_name##_TestBody, \
            []() -> std::unique_ptr<testing::TestFixture> { \
                return std::make_unique<suite_name##_##test_name##_Test>(); \
            } \
        ); \
    } \
} suite_name##_##test_name##_test_register; \
 \
void suite_name##_##test_name##_Test::TestBody()

// Assertion macros
#define EXPECT_TRUE(condition) \
    testing::internal::AssertImpl(!!(condition), __FILE__, __LINE__, \
        "Expected: " #condition " to be true, but it's false")

#define EXPECT_FALSE(condition) \
    testing::internal::AssertImpl(!(condition), __FILE__, __LINE__, \
        "Expected: " #condition " to be false, but it's true")

#define EXPECT_EQ(expected, actual) \
    testing::internal::AssertEqualImpl(expected, actual, __FILE__, __LINE__)

#define EXPECT_NE(expected, actual) \
    testing::internal::AssertNotEqualImpl(expected, actual, __FILE__, __LINE__)

#define EXPECT_GT(val1, val2) \
    testing::internal::AssertImpl((val1) > (val2), __FILE__, __LINE__, \
        "Expected: " #val1 " > " #val2 " but it's not")

#define EXPECT_STREQ(expected, actual) \
    testing::internal::AssertEqualImpl(std::string(expected), std::string(actual), __FILE__, __LINE__)

#define EXPECT_STRNE(expected, actual) \
    testing::internal::AssertNotEqualImpl(std::string(expected), std::string(actual), __FILE__, __LINE__)

#define EXPECT_THROW(statement, exception_type) \
    try { \
        statement; \
        testing::internal::AssertImpl(false, __FILE__, __LINE__, \
            "Expected: " #statement " to throw " #exception_type ", but it didn't"); \
    } catch (const exception_type&) { \
        /* Expected */ \
    } catch (...) { \
        testing::internal::AssertImpl(false, __FILE__, __LINE__, \
            "Expected: " #statement " to throw " #exception_type ", but it threw a different exception"); \
    }

#define EXPECT_NO_THROW(statement) \
    try { \
        statement; \
    } catch (...) { \
        testing::internal::AssertImpl(false, __FILE__, __LINE__, \
            "Expected: " #statement " not to throw, but it did"); \
    }

// Fatal assertion macros (stop test execution on failure)
#define ASSERT_TRUE(condition) \
    EXPECT_TRUE(condition); \
    if (!(condition)) { return; }

#define ASSERT_FALSE(condition) \
    EXPECT_FALSE(condition); \
    if ((condition)) { return; }

#define ASSERT_EQ(expected, actual) \
    EXPECT_EQ(expected, actual); \
    if ((expected) != (actual)) { return; }

#define ASSERT_NE(expected, actual) \
    EXPECT_NE(expected, actual); \
    if ((expected) == (actual)) { return; }

// Main function macro
#define RUN_ALL_TESTS() \
    int main(int argc, char* argv[]) { \
        testing::TestRunner runner; \
        runner.RunAllTests(argc, argv); \
        return runner.GetExitCode(); \
    }

#endif // TEST_FRAMEWORK_H