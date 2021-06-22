#ifndef APPFW_FILESYSTEM_H
#define APPFW_FILESYSTEM_H
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

namespace appfw {

/**
 * Exception thrown when file is not found by FIXME
 */
class FileNotFoundException : public std::runtime_error {
public:
    FileNotFoundException(std::string_view path);

    /**
     * Returns virtual path of the file that was not found.
     */
    const std::string &getFilePath() const;

private:
    std::string m_FilePath;
};

/**
 * Exception thrown when virtual file path contains a non-existant search group.
 */
class SearchGroupNotFoundException : public std::runtime_error {
public:
    SearchGroupNotFoundException(std::string_view path);

    /**
     * Returns virtual path of the file that was not found.
     */
    const std::string &getFilePath() const;

private:
    std::string m_FilePath;
};

/**
 * Exception thrown when virtual path is of an invalid format.
 */
class InvalidFilePathException : public std::runtime_error {
public:
    InvalidFilePathException(std::string_view path);

    /**
     * Returns the invalid virtual path.
     */
    const std::string &getFilePath() const;

private:
    std::string m_FilePath;
};

/**
 * Virtual filesystem that can check a list of directories for the file.
 * 
 * Virtual file path is in the following format:
 *     [tag]:[path]
 *   e.g.
 *     assets:maps/crossfire.bsp
 *     cfg:imgui.ini
 * 
 * The filesystem contains a list of search groups.
 * Each search group contains a list of paths (search paths).
 */
class FileSystem {
public:
    /**
     * Looks for existing file or returns a new path in the first search path.
     * @param   name    Virtual file name.
     */
    fs::path getFilePath(std::string_view name);

    /**
     * Looks for existing file or throws FileNotFoundException.
     * @param   name    Virtual file name.
     */
    fs::path findExistingFile(std::string_view name);

    /**
     * Looks for existing file or returns an empty path.
     * @param   name    Virtual file name.
     */
    fs::path findExistingFile(std::string_view name, std::nothrow_t);

    /**
     * Adds a search path to the end of the list (it will be searched after all others).
     */
    void addSearchPath(const fs::path &path, std::string_view tag);

    /**
     * Adds a search path to the front of the list (it will be searched first).
     */
    void addSearchPathToFront(const fs::path &path, std::string_view tag);

private:
    struct SearchGroup {
        std::string tag;
        std::vector<fs::path> paths;
    };

    std::vector<SearchGroup> m_Groups;

    /**
     * Find a group with specified tag or throws SearchGroupNotFoundException.
     */
    SearchGroup &findGroup(std::string_view tag, std::string_view vpath);

    /**
     * Find a group with specified tag or creates a new one. 
     */
    SearchGroup &findOrAddGroup(std::string_view tag);

    /**
     * Looks for existing file or returns an empty path.
     */
    fs::path findExistingFile(std::string_view vpath, std::string_view tag, const fs::path &path);

    /**
     * Converts the path to a search group name and fs::path
     */
    std::pair<std::string_view, fs::path> parseVirtualName(std::string_view name);
};

} // namespace appfw

#endif
