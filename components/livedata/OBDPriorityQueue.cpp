#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "OBDPriorityQueue.hpp"
#include "OBDLiveDataRequest.hpp"

OBDPriorityQueue::OBDPriorityQueue()
{
    mtx = xSemaphoreCreateMutex();
}

void OBDPriorityQueue::push(const OBDTask &task)
{
    xSemaphoreTake(mtx, portMAX_DELAY);
    pq.push(task);
    xSemaphoreGive(mtx);
}

void OBDPriorityQueue::clear()
{
    pq = std::priority_queue<OBDTask, std::vector<OBDTask>, TaskComparator>();
}

std::optional<OBDTask> OBDPriorityQueue::pop()
{
    std::optional<OBDTask> result;
    xSemaphoreTake(mtx, portMAX_DELAY);
    if (!pq.empty())
    {
        result = pq.top();
        pq.pop();
    }
    xSemaphoreGive(mtx);
    return result;
}

#include "esp_log.h"

static constexpr const char *TAG = "OBDPriorityQueue";

void OBDPriorityQueue::printQueue()
{
    xSemaphoreTake(mtx, portMAX_DELAY);

    // copy queue
    auto temp = pq;

    ESP_LOGI(TAG, "---- Queue contents ----");
    while (!temp.empty())
    {
        const OBDTask &t = temp.top();
        ESP_LOGI(TAG, "PID: 0x%02X | interval: %d ms | nextDue: %d",
                 t.request->pid(), t.priorityMargin, t.priorityTick);
        temp.pop();
    }
    ESP_LOGI(TAG, "-------------------------");

    xSemaphoreGive(mtx);
}

bool OBDPriorityQueue::empty()
{
    xSemaphoreTake(mtx, portMAX_DELAY);
    bool res = pq.empty();
    xSemaphoreGive(mtx);
    return res;
}
