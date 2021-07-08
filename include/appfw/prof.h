#ifndef APPFW_PROF_H
#define APPFW_PROF_H
#include <map>
#include <vector>
#include <set>
#include <appfw/utils.h>
#include <appfw/timer.h>

namespace appfw {

class Prof;
class ProfData;
struct ProfNode;

/**
 * A profiler section object.
 * 
 * It should be used like this
 *   void someFunc() {
 *     appfw::Prof prof("Some Func");
 *     ...
 *     doSomething();
 *   }
 * 
 *   void doSomething() {
 *     appfw::Prof prof("Do Somthing");
 *     ...
 *   }
 * 
 * It will be displayed in the profiler as
 *   Some Func - 1.345 ms
 *       Do Somthing - 0.300 ms
 *       (time lost) - 1.045 ms // total - sum of susbections
 * 
 * Before it is used, ProfData needs to be enabled
 *   ProfData m_ProfData;
 *   m_ProfData.setName("Main Loop");
 *   void mainLoopTick() {
 *     m_ProfData.begin();
 *     ...
 *     someFunc();
 *     ...
 *     m_ProfData.end();
 *   }
 * 
 * ProfData shouldn't be a local variable since profiler uses previous values to reduce jitter.
 * 
 * NOT THREAD SAFE
 * (but can be made by turning g_pCurProfData into a thread_local and adding locks)
 */
class Prof : appfw::NoMove {
public:
    /**
     * Constructs a profiler subsection.
     * @param   name    Name of the subsection. Must be unique in a given subsection. Must be a constant pointer (e.g. global buffer or string literal)
     */
    Prof(const char *name);
    ~Prof();

    /**
     * Returns the name of the subsection.
     */
    inline const char *name() { return m_Name; }

    /**
     * Stops time measuring
     */
    inline void stop() { m_Timer.stop(); }

    /**
     * Returns the elapsed time.
     */
    inline double time() { return m_Timer.dseconds(); }

private:
    const char *m_Name;
    Timer m_Timer;

    ProfNode *m_pPrevNode = nullptr;
    size_t m_uPrevHash = 0;

    friend class ProfData;
};

/**
 * Data of a profiler section
 */
struct ProfSection {
    size_t uHash = 0;
    const char *name = nullptr;
    unsigned uFrame = 0;

    //! Time of the section.
    //! 0 - exact latest time
    //! 1 - rolling avg time
    double flTime[2] = {0, 0};
};

/**
 * Profiler section node in the tree.
 */
struct ProfNode {
    ProfSection *pSection = nullptr;
    std::vector<ProfNode> children;
};

/**
 * Root profiler section.
 */
class ProfData : appfw::NoMove {
public:
    ProfData();
    ~ProfData();

    /**
     * Returns the name of this ProfData instance.
     */
    inline const char *getName() { return m_Name; }

    /**
     * Sets the name. Must be a constant pointer.
     */
    inline void setName(const char *name) { m_Name = name; }

    /**
     * Activates this ProfData.
     */
    void begin();

    /**
     * Deactivates this ProfData.
     */
    void end();

    /**
     * Called in Prof on enter into a subsection.
     */
    void subsectionEnter(Prof &prof);

    /**
     * Called on exit from a subsection.
     */
    void subsectionExit(Prof &prof, double time);

    /**
     * Returns a reference to the root node of previous frame.
     */
    inline const ProfNode &getPrevRootNode() { return m_PrevRootNode; }

    /**
     * Returns a pointer to a section. May be null.
     */
    ProfSection *getSectionByHash(size_t hash);

    /**
     * Returns the list of all ProfData instances.
     */
    static std::set<ProfData *> &getDataList();

    /**
     * Returns minimum lost time to be printed in seconds.
     */
    static double getMinLostTime();

private:
    static constexpr double NEW_PART = 0.2;

    const char *m_Name = nullptr;
    unsigned m_uFrame = 0;
    std::map<size_t, ProfSection> m_Sections;
    ProfNode m_RootNode;
    ProfNode m_PrevRootNode;
    Timer m_RootTimer;

    ProfNode *m_pCurNode = nullptr;
    size_t m_uCurHash = 0;
};

} // namespace appfw

#endif
