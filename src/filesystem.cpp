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
fs::path appfw::FileSystem::getFilePath(std::string_view name) const {
    std::shared_lock lock(m_Mutex);
    auto [tag, path] = parseVirtualName(name);

    if (path.empty()) {
        // Empty path is only allowed for directiories to point to the root
        throw InvalidFilePathException(name);
    }

    fs::path existingPath = findExistingFile(name, tag, path);

    if (!existingPath.empty()) {
        return existingPath;
    }

    const SearchGroup &g = findGroup(tag, name);

    if (g.paths.empty()) {
        throw std::logic_error("search group is empty");
    }

    return g.paths[0] / path;
}

fs::path appfw::FileSystem::findExistingFile(std::string_view name) const {
    fs::path p = findExistingFile(name, std::nothrow);

    if (p.empty()) {
        throw FileNotFoundException(name);
    }

    return p;
}

fs::path appfw::FileSystem::findExistingFile(std::string_view name, std::nothrow_t) const {
    auto [tag, path] = parseVirtualName(name);
    return findExistingFile(name, tag, path);
}

std::set<std::string> appfw::FileSystem::getFileList(std::string_view name) const {
    std::shared_lock lock(m_Mutex);
    std::set<std::string> fileList;
    auto [tag, path] = parseVirtualName(name);
    const SearchGroup &g = findGroup(tag, name);
    
    for (const fs::path &rootPath : g.paths) {
        auto directory = fs::directory_iterator(rootPath / path);

        for (auto &dirEntry : directory) {
            fileList.insert(dirEntry.path().filename().u8string());
        }
    }

    return fileList;
}

std::map<std::string, fs::directory_entry>
appfw::FileSystem::getDirEntries(std::string_view name) const {
    std::shared_lock lock(m_Mutex);
    std::map<std::string, fs::directory_entry> fileList;
    auto [tag, path] = parseVirtualName(name);
    const SearchGroup &g = findGroup(tag, name);

    for (const fs::path &rootPath : g.paths) {
        fs::path curPath = rootPath / path;
        if (!fs::is_directory(curPath)) {
            continue;
        }

        for (auto &dirEntry : fs::directory_iterator(curPath)) {
            std::string dirEntryName = dirEntry.path().filename().u8string();
            auto it = fileList.find(dirEntryName);

            if (it == fileList.end()) {
                fileList.insert({dirEntryName, dirEntry});
            }
        }
    }

    return fileList;
}

void appfw::FileSystem::addSearchPath(const fs::path &path, std::string_view tag) {
    std::unique_lock lock(m_Mutex);
    SearchGroup &g = findOrAddGroup(tag);

    fs::path pathToAdd = fs::absolute(path);

    if (!fs::is_directory(pathToAdd)) {
        throw InvalidFilePathException(path.u8string());
    }

    g.paths.push_back(pathToAdd);
}

void appfw::FileSystem::addSearchPathToFront(const fs::path &path, std::string_view tag) {
    std::unique_lock lock(m_Mutex);
    SearchGroup &g = findOrAddGroup(tag);

    fs::path pathToAdd = fs::absolute(path);

    if (!fs::is_directory(pathToAdd)) {
        throw InvalidFilePathException(path.u8string());
    }

    g.paths.insert(g.paths.begin(), pathToAdd);
}

const appfw::FileSystem::SearchGroup &appfw::FileSystem::findGroup(std::string_view tag,
                                                             std::string_view vpath) const {
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
                                             const fs::path &path) const {
    std::shared_lock lock(m_Mutex);
    const SearchGroup &g = findGroup(tag, vpath);

    for (const fs::path &spath : g.paths) {
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

    // path can be empty if it points to the root
    // path must not begin with a slash
    bool pathValid = path.empty() || path[0] != '/';
    if (tag.empty() || !pathValid) {
        throw InvalidFilePathException(name);
    }

    return {tag, fs::u8path(path)};
}
