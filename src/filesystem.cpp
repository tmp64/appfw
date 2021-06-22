#include <appfw/utils.h>
#include <appfw/filesystem.h>

//--------------------------------------------------------
// FileNotFoundException
//--------------------------------------------------------
appfw::FileNotFoundException::FileNotFoundException(std::string_view path)
    : std::runtime_error("file '" + std::string(path) + "' not found") {
    m_FilePath = path;
}

const std::string &appfw::FileNotFoundException::getFilePath() const {
    return m_FilePath;
}

//--------------------------------------------------------
// SearchGroupNotFoundException
//--------------------------------------------------------
appfw::SearchGroupNotFoundException::SearchGroupNotFoundException(std::string_view path)
    : std::runtime_error("file's '" + std::string(path) + "' search group not found") {
    m_FilePath = path;
}

const std::string &appfw::SearchGroupNotFoundException::getFilePath() const {
    return m_FilePath;
}

//--------------------------------------------------------
// InvalidFileNameException
//--------------------------------------------------------
appfw::InvalidFilePathException::InvalidFilePathException(std::string_view path)
    : std::runtime_error("file name '" + std::string(path) + "' is not valid") {
    m_FilePath = path;
}

const std::string &appfw::InvalidFilePathException::getFilePath() const {
    return m_FilePath;
}

//--------------------------------------------------------
// FileSystem
//--------------------------------------------------------
fs::path appfw::FileSystem::getFilePath(std::string_view name) {
    auto [tag, path] = parseVirtualName(name);
    fs::path existingPath = findExistingFile(name, tag, path);

    if (!existingPath.empty()) {
        return existingPath;
    }

    SearchGroup &g = findGroup(tag, name);

    if (g.paths.empty()) {
        throw std::logic_error("search group is empty");
    }

    return g.paths[0] / path;
}

fs::path appfw::FileSystem::findExistingFile(std::string_view name) {
    fs::path p = findExistingFile(name, std::nothrow);

    if (p.empty()) {
        throw FileNotFoundException(name);
    }

    return p;
}

fs::path appfw::FileSystem::findExistingFile(std::string_view name, std::nothrow_t) {
    auto [tag, path] = parseVirtualName(name);
    return findExistingFile(name, tag, path);
}

void appfw::FileSystem::addSearchPath(const fs::path &path, std::string_view tag) {
    SearchGroup &g = findOrAddGroup(tag);

    fs::path pathToAdd = fs::absolute(path);

    if (!fs::is_directory(pathToAdd)) {
        throw InvalidFilePathException(path.u8string());
    }

    g.paths.push_back(pathToAdd);
}

void appfw::FileSystem::addSearchPathToFront(const fs::path &path, std::string_view tag) {
    SearchGroup &g = findOrAddGroup(tag);

    fs::path pathToAdd = fs::absolute(path);

    if (!fs::is_directory(pathToAdd)) {
        throw InvalidFilePathException(path.u8string());
    }

    g.paths.insert(g.paths.begin(), pathToAdd);
}

appfw::FileSystem::SearchGroup &appfw::FileSystem::findGroup(std::string_view tag,
                                                             std::string_view vpath) {
    for (auto &i : m_Groups) {
        if (i.tag == tag) {
            return i;
        }
    }

    throw SearchGroupNotFoundException(vpath.empty() ? tag : vpath);
}

appfw::FileSystem::SearchGroup &appfw::FileSystem::findOrAddGroup(std::string_view tag) {
    for (auto &i : m_Groups) {
        if (i.tag == tag) {
            return i;
        }
    }

    // Create new one
    m_Groups.emplace_back();
    SearchGroup &g = *m_Groups.rbegin();
    g.tag = tag;

    return g;
}

fs::path appfw::FileSystem::findExistingFile(std::string_view vpath, std::string_view tag,
                                             const fs::path &path) {
    SearchGroup &g = findGroup(tag, vpath);

    for (fs::path &spath : g.paths) {
        fs::path fullPath = spath / path;
        if (fs::exists(fullPath)) {
            return fullPath;
        }
    }

    return fs::path();
}

std::pair<std::string_view, fs::path> appfw::FileSystem::parseVirtualName(std::string_view name) {
    size_t delimPos = name.find_first_of(':');
    if (delimPos == name.npos) {
        throw InvalidFilePathException(name);
    }

    std::string_view tag = name.substr(0, delimPos);
    std::string_view path = name.substr(delimPos + 1);

    if (tag.empty() || path.empty()) {
        throw InvalidFilePathException(name);
    }

    return {tag, fs::u8path(path)};
}
