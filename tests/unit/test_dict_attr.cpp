// test_dict_attr.cpp - Tests for dictionary attribute assignment behavior
#include "TestFramework.h"
#include "RGLite.h"
#include <memory>

using namespace rglite;

// Test fixture for dict attribute assignment tests
class DictAttrTestFixture : public testing::TestFixture {
protected:
    std::shared_ptr<Compiler> compiler;
    void SetUp() override { compiler = createCompiler(); }
    void TearDown() override { compiler.reset(); }
};

// Assigning a non-method attribute should create/update corresponding key
TEST_F(DictAttrTestFixture, AssignAttributeCreatesKey, DictAttrTestFixture) {
    std::string source = R"(
d = {}
d.a = 10
print(d.a)
)";
    bool ok = (compiler->execute(source) == 0);
    EXPECT_TRUE(ok);
}

// Assigning to a dict method name should be disallowed
TEST_F(DictAttrTestFixture, AssignMethodAttributeDisallowed, DictAttrTestFixture) {
    std::string source = R"(
d = {}
d.keys = 123
)";
    bool ok = (compiler->execute(source) == 0);
    EXPECT_FALSE(ok);
}

// Main test runner
RUN_ALL_TESTS()

