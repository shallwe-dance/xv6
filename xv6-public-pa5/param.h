
#define NPROC        64  // maximum number of processes
#define KSTACKSIZE 4096  // size of per-process kernel stack
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache

#define FSSIZE       21113  // size of file system in blocks
#define LOGSIZE (MAXOPBLOCKS*3)

#define BGSIZE      ((((FSSIZE)/32)<4096) ? 4096 : ((FSSIZE)/32))
#define NINODE_GROUP BGSIZE/4

#define LOGAREA      (((FSSIZE)%(BGSIZE))-2)  // max data blocks in on-disk log
#define BSIZE 512
//#define NBITMAP_GROUP ((BGSIZE)-(NINODE_GROUP)+(BSIZE*8))/(((BSIZE*8)+1))
#define NBITS_INODE ((NINODE_GROUP) * ((BSIZE)/sizeof(struct dinode)))
#define NBITMAP_GROUP (((NBITS_INODE) + (BSIZE*8) - 1) / (BSIZE*8))

#define NDATA_GROUP  ((BGSIZE)-(NINODE_GROUP)-(NBITMAP_GROUP))

#define NBG ((FSSIZE)/(BGSIZE))
