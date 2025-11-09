#pragma once
#include <queue>
#include <vector>
#include <optional>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "OBDTask.hpp"

struct TaskComparator
{
    bool operator()(const OBDTask &a, const OBDTask &b) const
    {
        return a.priorityTick > b.priorityTick;
    }
};

class OBDPriorityQueue
{
public:
    void printQueue();
    OBDPriorityQueue();
    void clear();

    void push(const OBDTask &task);
    std::optional<OBDTask> pop();
    bool empty();

private:
    SemaphoreHandle_t mtx;
    std::priority_queue<
        OBDTask,
        std::vector<OBDTask>,
        TaskComparator>
        pq;
};
