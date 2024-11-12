# Practical Work 2

## 2.1 - Sweep
### What does the __packed modifier do?
As explained in the [referenced article](http://www.catb.org/esr/structure-packing/), alignment requirements can produce "unneccesary" padding, that is required to ensure that memory accesses are aligned. As an example, integers (4 bytes) must start on an address divisible by 4 (2 right-most bits 0), while long (8 bytes) needs to be on addresses divisible by 8, while chars can be on any address.

As described, this forces the compiler to introduce "padding", in order to make sure that the next address is aligned with its type. The "__packed" attribute specifies that fields within the struct should be as compact as possible. [Source](https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Packed-Structures.html). 

### Explain memory layout
In this case of item_t, the uint32_t type makes the _stride access_ of the struct 4 bytes. Meaning, at the end of the memory allocation for the struct, a padding of __(-PARAM_DATALEN mod 4)__ is added to the end of the struct. Adding the __packed attribute removes this.

So, if the __PARAM_DATALEN__ is 1, the padding is 3 bytes. If the __PARAM_DATALEN__ is 2, the padding is 2, and if it is 3, the padding is 1. If __PARAM_DATALEN__ is 4, there is no padding. In all cases, the total size of the struct is 8 bytes.
Using the _\_packed attribute, the padding is removed, and the size of the struct is 5, 6, 7, 8 bytes, respectively for each __PARAM_DATALEN__ value.

This is very well presented in figure 2.2(a) of the project description.

### Explain Memory Layout for array
For the array, we utilize the size of the different structs with the various parameters. 
Say, the _\_packed attribute is not used, the size if always 8 bytes. Using __PARAM_DATALEN__ of 1, there are 3 bytes of padding per item_t. For the first two values of the array there are, in total, 6 bytes of slob. If the first element is at address 0x0, the next is at 0x8. For __PARAM_DATALEN__ of 2, the total bytes of slob if 4, but the address layout is the same.

Using the _\_packed attribute, the total slob is always 0. The memory layout is different. For __PARAM_DATALEN__ of 1, the first address is 0x0, and the next is 0x5. For __PARAM_DATALEN__ of 2, the first is 0x0, while the next is 0x6. 

### Cache Miss
First, lets examine figure 2.2(B). As mentioned in the previous section, the non-packed struct will be 8 for __PARAM_DATALEN__  = [1,2,3,4]. We see that when __PARAM_DATALEN__ goes to 5, the cache misses increase, at the same time as the struct size increases to 12 bytes. This indicates that the cache line is 32 bytes long, as it would not need to ..... 

As described in the previous sections, packing the struct tight makes better use of the memory. Considering __PARAM_DATALEN__ of 1, the packed struct uses 5 bytes, while unpacked uses 8. Given that the cache fecthed data in cache lines that are filled with many bytes, the packed struct_t can fit more structs next to eachother in each cache-line, while unpacked can only fit one. This means there are significantly less cache-misses for the packed version. However, this difference is lowered whent the __PARAM_DATALEN__ is 2, and lowered even further when it is 3. At value 4, the structs have the same size, meaning cache misses are the same. 

This behaviour is the same until the data length is 28 bytes. At this point, the size of the struct is 32 bytes. 



### Cache line size
TEXTETXTEXT



## TASK 1 - Optimize _node\_t_
### What accesses cause cache-misses?
We assume that the cache lines are 32 bytes.

Both the "node−>id" on line 29 and "node = node−>next" on line 31 cause cache misses. First of, "node−>id" fetches the id from a pointer, which is likely not in the cache.
Afterwards, the "node = node−>next" instruction reads the _next_ pointer from the struct. Because the length of the data character array is 52, and the id takes 4 bytes, the 4 bytes neccesary for the _next_ pointer is outside the cache line fetched while getting the node id. This causes a few cache miss.

### Optimize
For optimizing we can simply move the _next_ pointer up in the struct definition. Putting it above the data array but below the id field will ensure that it is fetched when reading the id. Therefore, the instruction on line 31 will not cause another cache miss.

Doing this, we reduce cache misses from 42 to 26.

## TASK 2 - Optimize _item\_t_
### Source of data cache misses
When looping in the _items/_find_ function, we read _items[i].id_ which fetches data from the cache. This is likely not in the cache already, leading to a cache miss. Furthermore, the _data_ field is within the same struct and takes 32 bytes. As the cache line is 32 bytes, it gets filled up for every struct, meaning it needs to fetch a new cache line for each struct.

### Optimize
If we use a data pointer, instead of storing the entire array, we can reduce the size of the struct from 36 bytes to just $4+8=12$ bytes. Doing this, we can fit 3 structs into a single cache line. To achieve this, we just allocate bytes when initializing the struct in _item\_init_. This data is therefore somewhere else in memory. 

This reduced the cache misses from 16 to just 4. 


## TASK 3 - Optimize Matrix-Vector Multiplication
### Explain the terms row-major and column-major order. Which approach is used in C?
Row-major => items in rows are subsequent in memory.
Column-major => items in columns are subsequent in memory.
C uses row-major.

### What accesses cause cache misses in function multiply?
The fetching of __matrix\[i\][j]__, as it is accessing items in a column. As our board uses row-major order (?), the row items are subsequent in memory. Reading in a column order causes large jumps in memory. This is likely to fill the cache and cause many cache-invalidations and therefore cache-misses when iterating over the loop.

### Optimize
Fixing this is quite easy, as you just swap the __i__ and __j__ in the instructed mentioned above. This causes it to iterate over the row items, instead of column items. This reduces the cache misses from 65893 to 8257.


## TASK 4 - Bouncing Ball Example
### Why does not the bouncing ball work as expected? You should be observing no change in the LEDs. The expected behavior is a single pixel moving and reflects as it bounces at the edges.

Because we are writing to a non-cache-coherent cacheable region with write-back enabled. This causes the data to just recide in the cache and never be written to the LEDS that should light up.

### How to fix this.
There are two approaches: either disable the cache, or use the write-through strategy, in order to ensure that the data is written directly to memory.

The first is very easy, you just add _dcache\_enable(1);_ in the beginning of the _bouncing\_ball_ function. Alternatively, you can keep the cache enabled and just use the function _dcache\_flush_ (found in cache.h in the support/include folder) in order to flush the data and force a write to the CPU, after updating the LED color.

Otherwise, you can enable the write-through policy, we replacing __CACHE\_WRITE\_BACK__ with __CACHE\_WRITE\_THROUGH__ in the _init\_dcache_ function. 


Other Ideas: Change to non-cacheable area (Original address is in non-cacheable area)
