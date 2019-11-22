#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include "common.h"

struct Timestamp
{
    Timestamp() { start(); }
    void start() { t = std::chrono::steady_clock::now(); }
    INT64 restart() { INT64 v = elapsed(); start(); return v; }
    INT64 elapsedNs() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - t).count();
    }
    INT64 elapsed() const { return elapsedNs() / 1000000; }
    INT64 elapsedNsSince(const Timestamp &other) const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(t - other.t).count();
    }
    INT64 elapsedSince(const Timestamp &other) const { return elapsedNsSince(other) / 1000000; }
    std::chrono::steady_clock::time_point t;
};

#endif
