#include <assert.h>
#include <cache.h>
#include <defs.h>
#include <locks.h>
#include <taskman/taskman.h>

#include <implement_me.h>
#include <stdio.h>

/// @brief Maximum number of wait handlers.
#define TASKMAN_NUM_HANDLERS 32

/// @brief Maximum number of scheduled tasks.
#define TASKMAN_NUM_TASKS 128

/// @brief Maximum total stack size.
#define TASKMAN_STACK_SIZE (256 << 10)

#define TASKMAN_LOCK_ID 2

#define TASKMAN_LOCK()             \
    do {                           \
        get_lock(TASKMAN_LOCK_ID); \
    } while (0)

#define TASKMAN_RELEASE()              \
    do {                               \
        release_lock(TASKMAN_LOCK_ID); \
    } while (0)

__global static struct {
    /// @brief Wait handlers.
    struct taskman_handler* handlers[TASKMAN_NUM_HANDLERS];

    /// @brief Number of wait handlers;
    size_t handlers_count;

    /// @brief Stack area. Contains multiple independent stacks.
    uint8_t stack[TASKMAN_STACK_SIZE];

    /// @brief Stack offset (for the next allocation).
    size_t stack_offset;

    /// @brief Scheduled tasks.
    void* tasks[TASKMAN_NUM_TASKS];

    /// @brief Number of tasks scheduled.
    size_t tasks_count;

    /// @brief True if the task manager should stop.
    uint32_t should_stop;
} taskman;

/**
 * @brief Extra information attached to the coroutine used by the task manager.
 *
 */
struct task_data {
    struct {
        /// @brief Handler
        /// @note NULL if waiting on `coro_yield`.
        struct taskman_handler* handler;

        /// @brief Argument to the wait handler
        void* arg;
    } wait;

    /// @brief 1 if running, 0 otherwise.
    int running;
};

void taskman_glinit() {
    taskman.handlers_count = 0;
    taskman.stack_offset = 0;
    taskman.tasks_count = 0;
    taskman.should_stop = 0;
}

void* taskman_spawn(coro_fn_t coro_fn, void* arg, size_t stack_sz) {
    TASKMAN_LOCK();
    // Preliminary Checks
    die_if_not(TASKMAN_STACK_SIZE > taskman.stack_offset + stack_sz);
    die_if_not(TASKMAN_NUM_TASKS  > taskman.tasks_count);
    
    // (1) allocate stack space for the new task
    void* stack_loc = (void*)&taskman.stack[taskman.stack_offset];
    taskman.stack_offset += stack_sz;

    // (2) initialize the coroutine and struct task_data
    coro_init(stack_loc, stack_sz, coro_fn, arg);
    
    struct task_data new_task_data = {
        .wait = {
            .handler = NULL,
            .arg = NULL,
        },
        .running = 0,
    };
    
    *(struct task_data*)coro_data(stack_loc) = new_task_data;

    // (3) register the coroutine in the tasks array
    taskman.tasks[taskman.tasks_count] = stack_loc;
    ++taskman.tasks_count;

    TASKMAN_RELEASE();
    return stack_loc;
}

void taskman_loop() {
    // (a) Call the `loop` functions of all the wait handlers.
    // (b) Iterate over all the tasks, and resume them if.
    //        * The task is not complete.
    //        * it yielded using `taskman_yield`.
    //        * the waiting handler says it can be resumed.
    int next_task = 0;
    TASKMAN_LOCK();

    while (!taskman.should_stop) {
        void* stack_loc = taskman.tasks[next_task];
        struct task_data* task_data = coro_data(stack_loc);


        if(task_data->running == 0) {
            struct taskman_handler* handler = task_data->wait.handler;
            void* args = task_data->wait.arg;
            if(handler == NULL) {
                task_data->running = 1;
                
                TASKMAN_RELEASE();
                coro_resume(stack_loc);
                TASKMAN_LOCK();

            } else {
                handler->loop(handler); 

                if(handler->can_resume(handler, stack_loc, args)){
                    task_data->running = 1;

                    TASKMAN_RELEASE();
                    coro_resume(stack_loc);
                    TASKMAN_LOCK();
                }
            }
        } 
        
        next_task = (next_task + 1) % taskman.tasks_count; // looping functionality
    }
}


void taskman_stop() {
    TASKMAN_LOCK();
    taskman.should_stop = 1;
    TASKMAN_RELEASE();
}

void taskman_register(struct taskman_handler* handler) {
    die_if_not(handler != NULL);
    die_if_not(taskman.handlers_count < TASKMAN_NUM_HANDLERS);

    taskman.handlers[taskman.handlers_count] = handler;
    taskman.handlers_count++;
}

void taskman_wait(struct taskman_handler* handler, void* arg) {
    void* stack = coro_stack();
    struct task_data* task_data = coro_data(stack);

    // I suggest that you read `struct taskman_handler` definition.
    // (1) Call handler->on_wait, see if there is a need to yield.
    if(handler != NULL && handler->on_wait(handler, stack, arg)) {
        return;
    }
    // (2) Update the wait field of the task_data.
    task_data->wait.handler = handler;
    task_data->wait.arg = arg;
    task_data->running = 0;
    
    // (3) Yield if necessary.
    coro_yield();
}

void taskman_yield() {
    taskman_wait(NULL, NULL);
}

void taskman_return(void* result) {
    coro_return(result);
}
