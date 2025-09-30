// On-disk file system format.
// Both the kernel and user programs use this header file.


#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// Disk layout:
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
  uint logstart;     // Block number of first log block
  //uint inodestart;   // Block number of first inode block
  //uint bmapstart;    // Block number of first free map block
  uint nblockgroups;
  uint bg_start;
  uint bg_size;
  uint ninodes_bg;
  uint ninodeblocks_bg;
  uint nbitmapblocks_bg;
  uint ndatablocks_bg;
  uint bg_meta;
};

#define NDIRECT 11
#define NINDIRECT (BSIZE / sizeof(uint))
#define N2INDIRECT NINDIRECT * NINDIRECT //define the number of doubly-indirect blocks
//#define MAXFILE (NDIRECT + NINDIRECT)
#define MAXFILE (NDIRECT + NINDIRECT + N2INDIRECT) //change MAXFILE accordingly

// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1+1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

#define IBLOCKGROUP(i) ((i) / IPB) //calculates where inode is contained

#define IBLOCKGROUPSTART(i) ((2+LOGAREA) + (IBLOCKGROUP(i) * BGSIZE))

// Block containing inode i
//#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)
#define IBLOCK(i) (IBLOCKGROUPSTART(i) + ((i) % NINODE_GROUP))

// Bitmap bits per block
#define BPB           (BSIZE*8)

#define BG_IDX(b) (((b)-(2+LOGAREA))/BGSIZE)


// Block of free map containing bit for block b
//#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)
#define BBLOCK(b,sb) (BG_IDX(b) + sb.ninodeblocks_bg +  (((b) - sb.bg_meta) % sb.bg_size) / (BSIZE*8))


// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

