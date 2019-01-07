#ifndef RING_H
#define RING_H

#include <asm/semaphore.h>
#include <asm/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/malloc.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include "ring_ioctl.h"

#define DEFAULT_BUFFER_SIZE 1024
#define MIN_BUFFER_SIZE 256
#define MAX_BUFFER_SIZE 16384
#define RING_MAJOR 114

#include "ring.h"

struct buffer {
	char *buffer;
	int bufferCount;
	int bufferSize;
	int start;
	int end;
	int useCount;
	struct semaphore semaphore;
	struct wait_queue *readQueue;
	struct wait_queue *writeQueue; 
};

int ring_ioctl(struct inode *inode, struct file *file, unsigned int command, unsigned long data);
int ring_open(struct inode *inode, struct file *file);
void ring_release(struct inode *inode, struct file *file);
int ring_read(struct inode *inode, struct file *file, char *pB, int count);
int ring_write(struct inode *inode, struct file *file, const char *pB, int count);
int ring_init(void);
int init_module(void);
void cleanup_module(void);

#endif /* RING_H */


