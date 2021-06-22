#include <appfw/filesystem.h>
#include <doctest/doctest.h>

TEST_CASE("appfw::FileSystem") {
    appfw::FileSystem filesystem;

    fs::path workdir = fs::current_path() / "filesystem";

    filesystem.addSearchPath(workdir / fs::u8path("group1"), "group1");
    filesystem.addSearchPath(workdir / fs::u8path("group2_path1"), "group2");
    filesystem.addSearchPath(workdir / fs::u8path("group2_path2"), "group2");

    CHECK(filesystem.getFilePath("group1:file.txt") == (workdir / "group1/file.txt"));
    CHECK(filesystem.getFilePath("group2:file.txt") == (workdir / "group2_path1/file.txt"));
    CHECK(filesystem.getFilePath("group1:file_not_found.txt") ==
          (workdir / "group1/file_not_found.txt"));
    CHECK(filesystem.getFilePath("group2:file_not_found.txt") ==
          (workdir / "group2_path1/file_not_found.txt"));
    CHECK(filesystem.findExistingFile("group2:file_not_found.txt", std::nothrow) == fs::path());
    CHECK_THROWS_AS(filesystem.findExistingFile("group1:file_not_found.txt"),
                    appfw::FileNotFoundException);

    // Invalid groups
    CHECK_THROWS_AS(filesystem.getFilePath("groupXXX:file.txt"),
                    appfw::SearchGroupNotFoundException);

    // Invalid name
    CHECK_THROWS_AS(filesystem.getFilePath(""), appfw::InvalidFilePathException);
    CHECK_THROWS_AS(filesystem.getFilePath(":"), appfw::InvalidFilePathException);
    CHECK_THROWS_AS(filesystem.getFilePath("group:"), appfw::InvalidFilePathException);
    CHECK_THROWS_AS(filesystem.getFilePath(":name"), appfw::InvalidFilePathException);
}
