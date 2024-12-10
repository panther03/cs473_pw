
# Notes / Answers from Magnus
## Part 1
### Question 1:
1: Concurrency => Relates to programs. Two functions that run concurrently can assumed to run independently and at different speeds. Parallelism => Relates to CPUs. You Parallelism allows you to actually run two procedures at once on the same computer.

2: TBA

3: TBA

4: TBA

### Question 2:
We see that the program launches a coroutine, starting in the function _test\_fun_. Since this was spawned by the function _part1_, it will not be run instantly. Instead, _part1_ will keep executing, printing the first line of format "func = \[...\]". After this, it will resume the execution of the function _test\_fn_. This will print the first "func = \[...\]" line with the _coro data_. Following this, it will enter the for-loop and print another of these functions with the _coro/_arg_. After this it will yield and let the main function continue. 
This will print another line of the format "func = \[...\]". 
Again, it will allow _test\_fn_ to continue and enter the "_f_" function, where it will print "func = f, x = 15".
After this, it yields again, and the main program prints once more, before allowing _test\_fn_ to run again. Hereafter, function "_f_" returns with 75, causing it to print "func = test_fn, f(15 + i) = 75". It will then keep looping and write "func = test_fn, i = 1, arg = 0xdeadbeef". 
This looping continues until the main function has looped 9 times. Hereafter, it will print the result returned from _test\_fn_, which is 10.

### Question 3:
#### coro\_resume
This function is used to resume a coroutine that is currently paused. 
First of, it makes some preliminary checks, such as making sure a coroutine to resume is deined, and that we are not already in a coroutine.
It then sets the value of register 10 to the function pointer of the coroutine to resume and uses the _coro\_\_switch_ function to switch executing from the current function to the coroutine. Here, it uses data from the coro\_data struct that describes the coroutine.
When the coroutine yields, it resets register 10 and returns.

#### coro\_yield
This works by first checking it is called from a coroutine and then returning CPU priority to the function that resumed the coroutine.
Again, this switch is achieved using the _coro\_\_switch_ function.

#### coro\_return
This works almost in the same way as _coro\_yield_, except before reutrning CPU priority, it will set its _completed_ flag to _true_ and the _result_ value to anything passed as argument to the _coro\_return_ function.


### Question 4:
This is explained on page 356 of the OpenRisc manual found on Moodle. It holds the memory address of the data associated with the CURRENTLY executing coroutine. This is also reflected by its use in _coro.c_.

### Question 5:
The function takes two arguments, namely "sp" and "old\_sp". The first one is the stack pointer of the coroutine to switch __to__ and the other is the stack pointer of the coroutine to switch __from__.

First, it allocated 0x2C bytes on the stack, and then saves r1, r9 and all the other registers noted as "Callee-saved register" in table 17-4 of the OpenRisc Manual found on page 355. It also stored the special purpose register 17. This is the supervisor register as described on page 25 of the OpenRisc Manual.

After this, it loads all the values that might be stored on the stack of the context you're switching to. It does the same as above, but instead of storing from registers to the stack, it loads from the stack to the registers.

Finally, it jumps to the function pointer of the coroutine to jump to, which stored at the bottom of the stack allocated for that coroutine. 

## Part 2.1:
### Question 6:
Done

### Question 7:
The purpose of the flag is to specify, whether or not a coroutine is currently being executed. We see that this is initially set to "false" (0), but updated to "1" whenever the coroutine is resumed. This is useful in a multi-core implementation, as we do not want to execute the same coroutine in two cores at the same time. However, in a single core implementation, we know that when we are in the taskman\_loop function no coroutine is running. If they where running, then we would not be able to execute the taskman\_loop. Using this fact, we can simply remove the flag when simply treat it is always being set to "false" inside the taskman\_loop.

### Question 8:
Done

### Question 9:
Done

## Part 2
### Question 10:
The lock works by having 256 distinct locks that each can be acquired by a CPU, based on the ID of each CPU. Each lock has a unique memory location. When a CPU wants to require a lock, it provides the ID for the lock (any value between 0-255) and busy-waits, until the value at the memory location of the lock is zero. It uses an _atomic_ compare and swap (CAS) operation to do this. When the value at the address equals zero, it will atomically swap this with the ID of the CPU that wants to require the lock. 
When a CPU wants to release the lock, it simply sets the value of the lock to zero, allowing another CPU to acquire the lock if needed.

### Question 11:
For the Taskman implementation, we can identify 2 major race-conditions.
Firstly, reading and updating task_data->running flag in a mutli core setting needs to be done inside a critical reading. Otherwise, two cores might run the taskman\_loop at the same time, both read that a coroutine is not running and both try and run the coroutine.

Secondly, as mentioned in the project description, all handler functions need to be called from within a critical region. This is because the handlers are managed through a global struct that may be changed by calling these functions. 

In both of these cases, they are relevant from within the taskman\_loop. One simple way to account for this, is to make everything within the loop a critical region, except for the instruction of _coro\_resume_, in which the loop switches to for coroutine for a CPU. However, it may be that two CPU's, both running the taskman loop, try and evaluate two different coroutines that each use two different handlers. In this case, the two loops should be able to run in parallel, as we don't risk overlapping critical regions. Even though this approach is more efficient, it also comes with greater complexity, as the CPUs need to somehow share which coroutine they are working on. When using TASKMAN\_LOCK(), we use a single lock across all CPUs. One way to make this implementation would be to give each task a unique lock and each handler a unique lock and acquire those instead of the taskman lock. 

When we implement the TASKMAN\_LOCK() call before entering the taskman loop, while releasing before each _coro\_resume_, re-acquiering just after, we see that the print tasks are called continously and from alternating CPUs. This is the expected behavior, and indicate that the multi core implementation works.
