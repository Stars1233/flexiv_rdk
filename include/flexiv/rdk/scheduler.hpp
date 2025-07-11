/**
 * @file scheduler.hpp
 * @copyright Copyright (C) 2016-2025 Flexiv Ltd. All Rights Reserved.
 */

#ifndef FLEXIV_RDK_SCHEDULER_HPP_
#define FLEXIV_RDK_SCHEDULER_HPP_

#include <string>
#include <functional>
#include <memory>

namespace flexiv {
namespace rdk {

/**
 * @class Scheduler
 * @brief Real-time scheduler that can simultaneously run multiple periodic tasks. Parameters for
 * each task are configured independently.
 */
class Scheduler
{
public:
    /**
     * @brief [Blocking] Instantiate a real-time scheduler.
     * @throw std::runtime_error if the initialization sequence failed.
     * @warning This constructor blocks until the initialization sequence is successfully finished.
     */
    Scheduler();
    virtual ~Scheduler();

    /**
     * @brief [Non-blocking] Add a new periodic task to the scheduler's task pool. Each task in the
     * pool is assigned to a dedicated thread with independent thread configuration.
     * @param[in] callback Callback function of user task.
     * @param[in] task_name A unique name for this task.
     * @param[in] interval Execution interval of this periodic task [ms]. The minimum available
     * interval is 1 ms, equivalent to 1 kHz loop frequency.
     * @param[in] priority Priority for this task thread, can be set to
     * min_priority()–max_priority() for real-time scheduling, or 0 for non-real-time scheduling.
     * When the priority is set to use real-time scheduling, this thread becomes a real-time thread
     * and can only be interrupted by threads with higher priority. When the priority is set to use
     * non-real-time scheduling (i.e. 0), this thread becomes a non-real-time thread and can be
     * interrupted by any real-time threads. The common practice is to set priority of the most
     * critical tasks to max_priority() or near, and set priority of other non-critical tasks to 0
     * or near. To avoid race conditions, the same priority should be assigned to only one task.
     * @param[in] cpu_affinity CPU core for this task thread to bind to, can be set to 2–(num_cores
     * - 1). This task thread will only run on the specified CPU core. If left with the default
     * value (-1), then this task thread will not bind to any CPU core, and the system will decide
     * which core to run this task thread on according to the system's own strategy. The common
     * practice is to bind the high-priority task to a dedicated spare core, and bind low-priority
     * tasks to other cores or just leave them unbound (cpu_affinity = -1).
     * @throw std::logic_error if the scheduler is already started or is not fully initialized yet.
     * @throw std::invalid_argument if the specified interval/priority/affinity is invalid or the
     * specified task name is duplicate.
     * @throw std::runtime_error if an error is triggered by the client computer.
     * @note Setting CPU affinity on macOS has no effect, as its Mach kernel takes full control of
     * thread placement so CPU binding is not supported.
     * @warning Calling this function after start() is not allowed.
     * @warning For maximum scheduling performance, setting CPU affinity to 0 or 1 is not allowed:
     * core 0 is usually the default core for system processes and can be crowded; core 1 is
     * reserved for the scheduler itself.
     */
    void AddTask(std::function<void(void)>&& callback, const std::string& task_name, int interval,
        int priority, int cpu_affinity = -1);

    /**
     * @brief [Blocking] Start all added tasks. A dedicated thread will be created for each added
     * task and the periodic execution will begin.
     * @throw std::logic_error if the scheduler is not initialized yet.
     * @throw std::runtime_error if failed to start the tasks.
     * @note This function blocks until all added tasks are started.
     */
    void Start();

    /**
     * @brief [Blocking] Stop all added tasks. The periodic execution will stop and all task threads
     * will be closed with the resources released.
     * @throw std::logic_error if the scheduler is not initialized or the tasks are not started yet.
     * @throw std::runtime_error if failed to stop the tasks.
     * @note Calling start() again can restart the added tasks.
     * @note This function blocks until all task threads have exited and resources are released.
     * @warning This function cannot be called from within a task thread.
     */
    void Stop();

    /**
     * @brief [Non-blocking] Maximum available priority for user tasks.
     * @return The maximum priority that can be set for a user task with real-time scheduling policy
     * when calling AddTask().
     */
    int max_priority() const;

    /**
     * @brief [Non-blocking] Minimum available priority for user tasks.
     * @return The minimum priority that can be set for a user task with real-time scheduling policy
     * when calling AddTask().
     */
    int min_priority() const;

    /**
     * @brief [Non-blocking] Number of tasks added to the scheduler.
     * @return Number of added tasks.
     */
    size_t num_tasks() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} /* namespace rdk */
} /* namespace flexiv */

#endif /* FLEXIV_RDK_SCHEDULER_HPP_ */
