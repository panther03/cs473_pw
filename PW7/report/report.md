---
title: "CS476 - PW7 Report"
author: Julien de Castelnau (368509), Magnus Meyer (396302)
date: December 17, 2024
geometry: "a4paper, left=2cm,right=2cm,top=2cm,bottom=3cm"
output: pdf_document
---

# Part 1: Coroutines

## Question 1

1. Concurrency vs parallelism: concurrency is the overlapping of multiple execution streams. Parallelism indicates that mutliple streams are happening at the same time.

2. Cooperative multitasking: Each task has to manually yield to give up execution to other tasks. Preemptive multitasking means that there is some automatic switching between tasks, usually through the use of a timer interrupt.

3. Threads vs coroutines: Coroutines are managed inside the application whereas threads are managed by the OS. The scheduling of coroutines is a context switch within the same program whereas switching to different threads is managed by a prvileged OS kernel, usually by preemption instead of a manual yield as is the case with coroutines.

4. Stackless vs stackful coroutines: Stackful coroutines are allocated their own stack and program state separate from the main coroutine. This allows them to yield in the middle of a deeper call stack. Stackless coroutines do not permit this as they share the stack of the main coroutine and thus the context upon switching would be invalid.

## Question 2

### `part1()`

* Line 30: `coro_glinit();` This simply initializes the pointer to the current coroutine, which is held in the Thread Local Storage (TLS) register `r10`, to zero.

* Line 32: `coro_init()`: This function initializes the coroutine, which sets the pointer to the global stack buffer in the struct, and also sets the arguments that will be passed to the coroutine. 

* Line 33: `coro_data()`: This function returns the pointer to the start of the data section for that coroutine. The layout of the coroutine's stack is as follows: at the start is the `coro_data` struct itself, containing info such as the caller SP, the coroutine's SP, the return value, etc. Following that is the area which `core_data()` returns a pointer to. The rest of this area is considered the stack area, and the stack grows downward from the end of the buffer as given by `stack_sz` in `coro_init()`. So, the other end of the stack is used to store global data for coroutines.

* Line 37: `coro_resume()`: This switches to a coroutine from the main routine. The argument is the stack buffer of the coroutine.

* Line 41: `coro_completed()`: This checks the completed flag inside the `coro_data` struct and sets the result if the coroutine is complete.

### `test_fn()`

* Line 14: `coro_data()`: used to get pointer to data area once again
* Line 18: `coro_arg()`: this uses the pointer to current coroutine, stored in `r10`, to access the `arg` field of the `coro_data` struct set by `coro_init()`
* Line 19: `coro_yield()`: Switch execution back to caller routine.
* Line 24: `coro_return()`: Set the completed flag and return value using `r10` as the current coroutine, then yield.


The program's execution essentially ping-pongs between the main function and the coroutine. The coroutine's loop switches back to the `for (int i = 0; i < 9; ++i)` loop on line 35 after every time it prints `i` and `arg` on line 18. That loop in `part1()` switches back to `test_fn()` using `coro_resume` in turn. Then, `f()` yields after printing the argument `x`. Since `test_fn()` yields twice per loop iteration while `part` only once, this is why the `part1` loop has twice as many iterations

## Question 3

These all make use of `coro_switch`. This function takes the stack pointer so that it can switch to the corresponding context, as well as a pointer to where we can save the stack pointer in the current context. 

* `coro_resume` changes context from a non-coroutine to a coroutine. So, it accesses the `coro_sp` field of the passed coroutine stack buffer, and uses that as the new SP. The current stack pointer is saved in the `caller_sp` field of the `coro_data`. There are extra sanity checks reading `CORE_SELF()` to make sure this is not called from within a coroutine, as well as to ensure a coroutine which is already finished is not switched to. Note we need to set the current coroutine field in the global r10 register, and then set it back to NULL whenever we yield back. This is done using `CORO_SET_SELF` in the implementation.
 
* `coro_yield` is the reverse, so it gets the current `coro_data` using `CORE_SELF()` (reading r10), then switches to the caller_sp saved earlier, storing the current sp in `coro_sp`. This also has a sanity check to make sure it *is* being caled from a coroutine (`r10` should not be null.)

* `coro_return` is `coro_yield` with an extra step to set the completed flag and result fields of the `coro_data` beforehand.

## Question 4

Register r10 stores the pointer to the beginning of the current coroutine's stack buffer. This corresponds to the `coro_data` struct.

## Question 5

`coro__switch` is implemented by first saving the current context. Only the callee-saved registers need to be saved: `coro_switch` is a function call from the compiler's perspective, so any caller-saved registers used in the calling context would be saved according to the OpenRISC calling convention. Thus, this function only saves the callee saved registers. `r10` is not saved because it is set before and after switching by `coro_resume` appropriately. Likewise, `r1` is set when switching itself, so it doesn't need to be saved. It also saves the current stack pointer in the location passed to it, so it can be returned back from in the future. Once the context saving is done, it restores from the stack pointed to by the `sp` provided, and then jumps to it using the link register on that stack.

## Part 2.1: Single-core task manager