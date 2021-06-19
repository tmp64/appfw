#include <appfw/command_line.h>
#include <doctest/doctest.h>

namespace {

struct TestCase {
    std::vector<std::string> inputArgs;
    std::vector<std::pair<std::string, std::string>> args;
    std::vector<std::string> posArgs;
    std::vector<std::string> strflags;
    std::vector<char> charflags;
};

TestCase s_TestCases[] = {
    TestCase{{"execName", "--arg1", "test1", "--flag1", "--arg2", "test2", "posArg1"},
             {
                 {"--arg1", "test1"},
                 {"--arg2", "test2"},
             },
             {"posArg1"},
             {"--flag1"},
             {}},
    TestCase{
        {"execName", "--arg1", "test1", "-fdh", "--arg2", "test2", "posArg1", "--", "--posArg2"},
        {
            {"--arg1", "test1"},
            {"--arg2", "test2"},
        },
        {"posArg1", "--posArg2"},
        {},
        {'f', 'h', 'd'}},
    TestCase{{"execName"}, {}, {}, {}, {}},
};

}

TEST_CASE("appfw::CommandLine") {
    for (size_t i = 0; i < std::size(s_TestCases); i++) {
        TestCase &test = s_TestCases[i];
        appfw::CommandLine cmdLine;

        std::vector<const char *> inputArgs(test.inputArgs.size());
        for (size_t j = 0; j < inputArgs.size(); j++) {
            inputArgs[j] = test.inputArgs[j].c_str();
        }

        cmdLine.parseCommandLine(inputArgs.size(), inputArgs.data(), false);

        // Exec name
        CHECK(cmdLine.getCommandName() == test.inputArgs[0]);

        // Args
        for (auto &j : test.args) {
            CHECK(cmdLine.doesArgHaveValue(j.first));
            CHECK(cmdLine.getArgString(j.first) == j.second);
        }

        // Pos args
        CHECK(cmdLine.getPosArgs() == test.posArgs);

        // Str flags
        for (auto &j : test.strflags) {
            CHECK(cmdLine.isFlagSet(j));
        }

        // Char flags
        for (auto &j : test.charflags) {
            CHECK(cmdLine.isFlagSet("noname", j));
        }
    }
}
