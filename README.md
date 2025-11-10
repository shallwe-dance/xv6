xv6 projects

#Assignment 1. System Call(100/100)
Goal : Making three system calls in xv6 kernel(getnice, setnice, ps)
```
int getnice(int pid); //obtains the nice value of a process. Return nice value. -1 if no corresponding pid
int setnice(int pid, int value); //sets the nice value of a process. Return 0 on success, -1 if there is no process
void ps(int pid); //prints out process(es)'s information
```

2. CFS scheduler (100/100)
3. memory map/unmap (60/100)
4. page swapping (100/100)
5. large file / block group file system (100/100)
