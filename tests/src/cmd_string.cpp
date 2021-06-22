#include <appfw/cmd_string.h>
#include <doctest/doctest.h>

namespace {

struct CmdStringTest {
    std::string_view str;
    std::vector<std::vector<std::string>> args;
};

CmdStringTest g_TestData[] = {
    CmdStringTest{"test", {{"test"}}},
    CmdStringTest{"test;", {{"test"}}},
    CmdStringTest{"      test   asdf", {{"test", "asdf"}}},
    CmdStringTest{"      test   asdf   ", {{"test", "asdf"}}},
    CmdStringTest{"      test   asdf   ;", {{"test", "asdf"}}},
    CmdStringTest{"test;asdf", {{"test"}, {"asdf"}}},
    CmdStringTest{"test;asdf", {{"test"}, {"asdf"}}},
    CmdStringTest{"test; asdf", {{"test"}, {"asdf"}}},
    CmdStringTest{"test; asdf;", {{"test"}, {"asdf"}}},
    CmdStringTest{"test; asdf; ", {{"test"}, {"asdf"}}},
    CmdStringTest{"test; asdf;    ", {{"test"}, {"asdf"}}},
    CmdStringTest{"test;;;;; ;; ;; ;   asdf", {{"test"}, {"asdf"}}},
    CmdStringTest{"", {}},
    CmdStringTest{"       ", {}},
    CmdStringTest{";;;;;;;;;", {}},
    CmdStringTest{" ; ; ;; ;;;;; ", {}},
};

}

TEST_CASE("appfw::CmdString::parseString") {
    using appfw::CmdString;

    for (size_t i = 0; i < std::size(g_TestData); i++) {
        CmdStringTest &t = g_TestData[i];
        auto parsed = CmdString::parse(t.str);
        
        REQUIRE(parsed.size() == t.args.size());

        for (size_t j = 0; j < parsed.size(); j++) {
            CHECK(parsed[j].getArgs() == t.args[j]);
        }
    }
}
