#include "handy_variant.hpp"
#include <gtest/gtest.h>
#include <memory>

using namespace std::string_literals;
using namespace handy_variant;
using handy_variant::variant_map;

class VariantTest : public ::testing::Test
{
protected:
    virtual void SetUp() {}
    // virtual void TearDown() {}
};

TEST_F(VariantTest, index_of)
{
    std::variant<int, float, std::string> v;
    ASSERT_EQ(index_of<int>(v), 0);
    ASSERT_EQ(index_of<float>(v), 1);
    ASSERT_EQ(index_of<std::string>(v), 2);
}

TEST_F(VariantTest, basic_cast)
{
    std::variant<int, float> v;
    v = 1;
    ASSERT_EQ(variant_cast<int>(v), 1);
    ASSERT_EQ(variant_cast<float>(v), 1.0f);
}

TEST_F(VariantTest, basic_string_cast)
{
    std::variant<int, bool, std::string> v;
    v = 1;
    ASSERT_EQ(variant_cast<int>(v), 1);
    ASSERT_EQ(variant_cast<std::string>(v), "1");

    v = true;
    ASSERT_EQ(variant_cast<int>(v), 1);
    ASSERT_EQ(variant_cast<std::string>(v), "true");

    v = "true"s;
    ASSERT_EQ(variant_cast<bool>(v), true);
    ASSERT_THROW(variant_cast<int>(v), std::invalid_argument);
    ASSERT_EQ(variant_cast<int>(v, 0), 0);
}

TEST_F(VariantTest, variant_to_variant)
{
    std::variant<int, bool, std::string> src;
    std::variant<int, bool> dst;

    src = 1;
    dst = variant_to_variant<int, bool>(src);
    ASSERT_EQ(variant_cast<int>(dst), 1);

    // Note: if the s literal is not used, it is stored as an int type.
    src = "test"s;
    ASSERT_THROW((variant_to_variant<int, bool>(src)), std::invalid_argument);
}

class VariantMapTest : public ::testing::Test
{
protected:
    virtual void SetUp() {}
    // virtual void TearDown() {}
};

TEST_F(VariantMapTest, basic)
{
    using int_float = variant_map<int, float, std::string>;

    int_float map;
    map.set<int>("exists", 1);

    ASSERT_EQ(map.index_of<int>(), 0);
    ASSERT_EQ(map.index_of<float>(), 1);
    ASSERT_EQ(map.index_of<std::string>(), 2);

    ASSERT_EQ(map.index_at("exists"), 0);

    ASSERT_EQ(map.get<int>("exists", 0), 1);
    ASSERT_THROW(map.cast_get<int>("not exists"), std::invalid_argument);
    ASSERT_EQ(map.get<int>("not exists", 3), 3);
    ASSERT_EQ(map.cast_get<float>("exists", 0), 1.0f);
    ASSERT_EQ(map.cast_get<float>("not exists", 0), 0.0f);
    ASSERT_EQ(map.cast_get<std::string>("exists", ""), "1");
}

TEST_F(VariantMapTest, basic_ptr)
{
    using int_ptr = variant_map<int, std::shared_ptr<int>>;

    int_ptr map;
    map.set<int>("exists", 1);

    ASSERT_EQ(map.get<int>("exists", 0), 1);
    ASSERT_EQ(map.get<int>("not exists", 3), 3);
    ASSERT_THROW(map.cast_get<std::shared_ptr<int>>("exists"), std::invalid_argument);
    ASSERT_EQ(map.cast_get<std::shared_ptr<int>>("exists", nullptr), nullptr);
}

class A
{
public:
    A(){};
    virtual ~A(){};

public:
    virtual const char *virtual_method() const { return "base_method"; };
};

class B : public A
{
public:
    B() : A(){};
    virtual ~B(){};

public:
    virtual const char *virtual_method() const override { return "extend_method"; };
};

template <>
struct _basic_cast<std::shared_ptr<B>, std::shared_ptr<A>, true>
{
    static std::shared_ptr<A> vl(const std::shared_ptr<B> &v)
    {
        return std::dynamic_pointer_cast<A>(v);
    };
};

TEST_F(VariantMapTest, cast_ptr)
{
    using ptr = variant_map<std::shared_ptr<A>, std::shared_ptr<B>>;

    ptr map;
    map.set<std::shared_ptr<A>>("A", std::make_shared<A>());
    map.set<std::shared_ptr<B>>("B", std::make_shared<B>());

    ASSERT_EQ(map.cast_get<std::shared_ptr<A>>("A")->virtual_method(), "base_method");
    ASSERT_EQ(map.cast_get<std::shared_ptr<B>>("B")->virtual_method(), "extend_method");
    ASSERT_EQ(map.cast_get<std::shared_ptr<A>>("B")->virtual_method(), "extend_method");
}