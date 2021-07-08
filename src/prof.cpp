#include <functional>
#include <appfw/appfw.h>
#include <appfw/dbg.h>
#include <appfw/prof.h>

static ConVar<int> prof_min_lost_time("prof_min_lost_time", 15,
                                      "Minimum lost time to be printed in us");

static std::hash<const char *> s_Hash; // yes, hash of a pointer
static appfw::ProfData *s_pCurProfData = nullptr;

appfw::Prof::Prof(const char *name) {
    if (s_pCurProfData) {
        m_Name = name;
        m_Timer.start();
        s_pCurProfData->subsectionEnter(*this);
    }
}

appfw::Prof::~Prof() {
    if (s_pCurProfData) {
        m_Timer.stop();
        s_pCurProfData->subsectionExit(*this, time());
    }
}

appfw::ProfData::ProfData() {
    getDataList().insert(this);
}

appfw::ProfData::~ProfData() {
    getDataList().erase(this);
}

void appfw::ProfData::begin() {
    AFW_ASSERT(m_Name);
    AFW_ASSERT(!m_pCurNode);
    AFW_ASSERT(!s_pCurProfData);

    s_pCurProfData = this;
    m_RootTimer.start();

    // Remove old sections
    auto it = m_Sections.begin();
    while (it != m_Sections.end()) {
        if (it->second.uFrame != m_uFrame) {
            auto del = it++;
            m_Sections.erase(del);
        } else {
            ++it;
        }
    }

    // Move the root node
    m_PrevRootNode = std::move(m_RootNode);

    // Create new node
    m_uFrame++;
    m_RootNode = ProfNode();

    m_pCurNode = &m_RootNode;
    m_uCurHash = s_Hash(m_Name);

    m_RootNode.pSection = &m_Sections[m_uCurHash];
    m_RootNode.pSection->name = m_Name;
    m_RootNode.pSection->uHash = m_uCurHash;
    m_RootNode.pSection->uFrame = m_uFrame;
}

void appfw::ProfData::end() {
    AFW_ASSERT(m_pCurNode == &m_RootNode);
    AFW_ASSERT(s_pCurProfData == this);

    double *curTime = m_RootNode.pSection->flTime;
    curTime[0] = m_RootTimer.dseconds();
    curTime[1] = NEW_PART * curTime[0] + (1 - NEW_PART) * curTime[1];

    m_pCurNode = nullptr;
    s_pCurProfData = nullptr;
}

void appfw::ProfData::subsectionEnter(Prof &prof) {
    prof.m_pPrevNode = m_pCurNode;
    prof.m_uPrevHash = m_uCurHash;
    m_uCurHash = m_uCurHash ^ s_Hash(prof.m_Name);
    ProfSection *pSection = nullptr;

    auto it = m_Sections.find(m_uCurHash);
    if (it != m_Sections.end()) {
        pSection = &it->second;
        AFW_ASSERT(pSection->uHash == m_uCurHash);
    } else {
        auto it2 = m_Sections.insert({m_uCurHash, ProfSection()});
        pSection = &it2.first->second;
        pSection->name = prof.m_Name;
        pSection->uHash = m_uCurHash;
    }

    pSection->uFrame = m_uFrame;

    ProfNode newNode;
    newNode.pSection = pSection;
    m_pCurNode->children.push_back(std::move(newNode));

    m_pCurNode = &(*m_pCurNode->children.rbegin());
}

void appfw::ProfData::subsectionExit(Prof &prof, double time) {
    double *curTime = m_pCurNode->pSection->flTime;
    curTime[0] = time;
    curTime[1] = NEW_PART * time + (1 - NEW_PART) * curTime[1];

    m_pCurNode = prof.m_pPrevNode;
    m_uCurHash = prof.m_uPrevHash;
    prof.m_pPrevNode = nullptr;
}

appfw::ProfSection *appfw::ProfData::getSectionByHash(size_t hash) {
    auto it = m_Sections.find(hash);

    if (it != m_Sections.end()) {
        return &it->second;
    } else {
        return nullptr;
    }
}

std::set<appfw::ProfData *> &appfw::ProfData::getDataList() {
    static std::set<ProfData *> list;
    return list;
}

double appfw::ProfData::getMinLostTime() {
    return prof_min_lost_time.getValue() / 1000000.0;
}

static void printProfilerNode(const appfw::ProfNode &node, unsigned depth) {
    std::string out =
        fmt::format("{}\t\t{:.3f} ms:", node.pSection->name, node.pSection->flTime[1] * 1000);

    std::string spaces = std::string(depth * 2, ' ');

    if (depth == 0) {
        printn("{}", out);
    } else {
        printi("{}{}", spaces, out);
    }

    double timeSum = 0;

    for (const appfw::ProfNode &i : node.children) {
        printProfilerNode(i, depth + 1);
        timeSum += i.pSection->flTime[1];
    }

    double timeLost = node.pSection->flTime[1] - timeSum;
    if (!node.children.empty() && timeLost > appfw::ProfData::getMinLostTime()) {
        printi("{}(time lost)\t\t{:.3f} ms:", spaces, timeLost * 1000);
    }
}

ConCommand cmd_prof_print("prof_print", "Print profiling data for current frame", []() {
    auto &list = appfw::ProfData::getDataList();

    for (appfw::ProfData *i : list) {
        printProfilerNode(i->getPrevRootNode(), 0);
    }
});
