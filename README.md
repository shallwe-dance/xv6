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
</br>

# Assignment 3. mmap() Implementation (60/100)
+ Goal : Implement system calls related with virtual memory (mmap, munmap, freemem)
+ Key Features
    + mmap() supports MAP_POPULATE, MAP_ANONYMOUS arguments, R/W modes
    + mmap_area structure stores metadata of a file
    + Page fault handler deals with access on mapping region with physical page & page table is not allocated
(The code fails to manage mmap_area when fork() is called. I'm planning to patch this someday.)
</br>

# Assignment 4. Page Swapping (100/100)
</br>

# Assignment 5. Large File Support & Block Group File System (100/100)
