#include "TestFramework.h"
#include "RGLite.h"
#include <memory>

using namespace rglite;

// Test fixture for docstring tests
class DocstringTestFixture : public testing::TestFixture {
protected:
    std::shared_ptr<Compiler> compiler;
    void SetUp() override { compiler = createCompiler(); }
    void TearDown() override { compiler.reset(); }
};

TEST_F(DocstringTestFixture, FunctionDocstringAccessible, DocstringTestFixture) {
    std::string source = R"(
def f(a):
    """Function f docstring"""
    return a + 1

print(f.__doc__.length)
)";
    bool result = (compiler->execute(source) == 0);
    EXPECT_TRUE(result);
}

TEST_F(DocstringTestFixture, FunctionWithoutDocstringIsNil, DocstringTestFixture) {
    std::string source = R"(
def g():
    return 42

print(isnil(g.__doc__))
)";
    bool result = (compiler->execute(source) == 0);
    EXPECT_TRUE(result);
}

TEST_F(DocstringTestFixture, ModuleDocstringAccessible, DocstringTestFixture) {
    std::string source = R"(
"""Module docstring here"""
x = 3
print(__doc__.length)
print(x)
)";
    bool result = (compiler->execute(source) == 0);
    EXPECT_TRUE(result);
}

// Main test runner
RUN_ALL_TESTS()
