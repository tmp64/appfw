#ifndef APPFW_TIMER_H
#define APPFW_TIMER_H
#include <chrono>

namespace appfw {

/**
 * A timer. Counts time from start() to stop()
 * Based on https://gist.github.com/mcleary/b0bf4fa88830ff7c882d
 */
class Timer {
public:
    inline void start() {
        m_StartTime = std::chrono::steady_clock::now();
        m_bRunning = true;
    }

    inline void stop() {
        m_EndTime = std::chrono::steady_clock::now();
        m_bRunning = false;
    }

    /**
     * Returns elapsed nanoseconds.
     */
    inline long long ns() {
        std::chrono::time_point<std::chrono::steady_clock> endTime;

        if (m_bRunning) {
            endTime = std::chrono::steady_clock::now();
        } else {
            endTime = m_EndTime;
        }

        return std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - m_StartTime).count();
    }

    /**
     * Returns elapsed microseconds.
     */
    inline long long ms() {
        std::chrono::time_point<std::chrono::steady_clock> endTime;

        if (m_bRunning) {
            endTime = std::chrono::steady_clock::now();
        } else {
            endTime = m_EndTime;
        }

        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count();
    }

    /**
     * Returns elapsed milliseconds.
     */
    inline long long us() {
        std::chrono::time_point<std::chrono::steady_clock> endTime;

        if (m_bRunning) {
            endTime = std::chrono::steady_clock::now();
        } else {
            endTime = m_EndTime;
        }

        return std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_StartTime).count();
    }

    /**
     * Returns elapsed seconds as a double.
     */
    inline double dseconds() { return ms() / 1000000.0; }

    /**
     * Returns elapsed seconds as a float.
     */
    inline float fseconds() { return ms() / 1000000.0f; }

private:
    std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
    std::chrono::time_point<std::chrono::steady_clock> m_EndTime;
    bool m_bRunning = false;
};

} // namespace appfw

#endif
