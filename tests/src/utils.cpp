#include <appfw/utils.h>
#include <doctest/doctest.h>

TEST_CASE("appfw::convertValToString") {
    // Bool
    CHECK(appfw::convertValToString(false) == "0");
    CHECK(appfw::convertValToString(true) == "1");

    // Short
    CHECK(appfw::convertValToString((short)0) == "0");
    CHECK(appfw::convertValToString((short)42) == "42");
    CHECK(appfw::convertValToString((short)-42) == "-42");

    CHECK(appfw::convertValToString((unsigned short)0) == "0");
    CHECK(appfw::convertValToString((unsigned short)42) == "42");

    // Int
    CHECK(appfw::convertValToString((int)0) == "0");
    CHECK(appfw::convertValToString((int)42) == "42");
    CHECK(appfw::convertValToString((int)-42) == "-42");

    CHECK(appfw::convertValToString((unsigned int)0) == "0");
    CHECK(appfw::convertValToString((unsigned int)42) == "42");

    // Long
    CHECK(appfw::convertValToString((long)0) == "0");
    CHECK(appfw::convertValToString((long)42) == "42");
    CHECK(appfw::convertValToString((long)-42) == "-42");

    CHECK(appfw::convertValToString((unsigned long)0) == "0");
    CHECK(appfw::convertValToString((unsigned long)42) == "42");

    // Long long
    CHECK(appfw::convertValToString((long long)0) == "0");
    CHECK(appfw::convertValToString((long long)42) == "42");
    CHECK(appfw::convertValToString((long long)-42) == "-42");

    CHECK(appfw::convertValToString((unsigned long long)0) == "0");
    CHECK(appfw::convertValToString((unsigned long long)42) == "42");

    // String
    CHECK(appfw::convertValToString(std::string("")) == "");
    CHECK(appfw::convertValToString(std::string("Never Gonna Give You Up")) ==
          "Never Gonna Give You Up");
}

TEST_CASE("appfw::convertStringToVal") {
    bool b = false;
    short s = 0;
    unsigned short us = 0;
    int i = 0;
    unsigned int ui = 0;
    long l = 0;
    unsigned long ul = 0;
    long long ll = 0;
    unsigned long long ull = 0;

    // Bool
    CHECK(appfw::convertStringToVal("0", b));
    CHECK(b == false);
    CHECK(appfw::convertStringToVal("1", b));
    CHECK(b == true);

    // Short
    CHECK(appfw::convertStringToVal("0", s));
    CHECK(s == 0);
    CHECK(appfw::convertStringToVal("42", s));
    CHECK(s == 42);
    CHECK(appfw::convertStringToVal("-42", s));
    CHECK(s == -42);

    CHECK(appfw::convertStringToVal("0", us));
    CHECK(us == 0);
    CHECK(appfw::convertStringToVal("42", us));
    CHECK(us == 42);

    // Int
    CHECK(appfw::convertStringToVal("0", i));
    CHECK(i == 0);
    CHECK(appfw::convertStringToVal("42", i));
    CHECK(i == 42);
    CHECK(appfw::convertStringToVal("-42", i));
    CHECK(i == -42);

    CHECK(appfw::convertStringToVal("0", ui));
    CHECK(ui == 0);
    CHECK(appfw::convertStringToVal("42", ui));
    CHECK(ui == 42);

    // Long
    CHECK(appfw::convertStringToVal("0", l));
    CHECK(l == 0);
    CHECK(appfw::convertStringToVal("42", l));
    CHECK(l == 42);
    CHECK(appfw::convertStringToVal("-42", l));
    CHECK(l == -42);

    CHECK(appfw::convertStringToVal("0", ul));
    CHECK(ul == 0);
    CHECK(appfw::convertStringToVal("42", ul));
    CHECK(ul == 42);

    // Long long
    CHECK(appfw::convertStringToVal("0", ll));
    CHECK(ll == 0);
    CHECK(appfw::convertStringToVal("42", ll));
    CHECK(ll == 42);
    CHECK(appfw::convertStringToVal("-42", ll));
    CHECK(ll == -42);

    CHECK(appfw::convertStringToVal("0", ull));
    CHECK(ull == 0);
    CHECK(appfw::convertStringToVal("42", ull));
    CHECK(ull == 42);
}
