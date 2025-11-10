# Assignment 1. System Call(100/100)
+ Goal : Making three system calls in xv6 kernel(getnice, setnice, ps)
```
int getnice(int pid); //obtains the nice value of a process. Return nice value. -1 if no corresponding pid
int setnice(int pid, int value); //sets the nice value of a process. Return 0 on success, -1 if there is no process
void ps(int pid); //prints out process(es)'s information
```
</br>

# Assignment 2. CFS Scheduler (100/100)
+ Goal : Implement CFS (Completely Fair Scheduler) in xv6
+ Key Features
    + Update virtual runtime, according to a weight of each process
    + Modify timer interrupt handling logic for context switches
    + Consider all edge cases (fork, sleep, busy work, etc.)
+ Review : 
</br>

# Assignment 3. mmap() Implementation (60/100)
+ Goal : Implement system calls related with virtual memory (mmap, munmap, freemem)
+ Key Features
    + mmap() supports MAP_POPULATE, MAP_ANONYMOUS arguments, R/W modes
    + mmap_area structure stores metadata of a file
    + Page fault handler deals with access on mapping region with physical page & page table is not allocated
+ Review : The code fails to manage mmap_area when fork() is called. I'm planning to patch this someday.
</br>

# Assignment 4. Page Replacement (100/100)
+ Goal : Implement page-level swapping (swap-in & swap-out)
+ Key Features
  + Manages swappable pages with LRU clock algorithm
  + Considered edge cases (fork, sleep, etc.)
+ Review : Easy to understand, hard to implement. So many macros and variables were needed to make everything work. While this project, I learned [why](https://stackoverflow.com/questions/10820340/the-need-for-parentheses-in-macros-in-c) using parentheses is important in macros
</br>

# Assignment 5. Large File Support & Block Group File System (100/100)
+ Goal : Increase the maximum size of an xv6 file + implement block group
+ Key Features
+ Review : My favorite xv6 project! Easy to debug, highly instructive, and fun.
