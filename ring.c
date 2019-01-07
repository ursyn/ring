#include "ring.h"

struct buffer *buffers = NULL;

struct file_operations ring_ops = {
	read: ring_read,
	write: ring_write,
	open: ring_open,
	release:ring_release,
	ioctl: ring_ioctl
};

int ring_ioctl(struct inode *inode, struct file *file, unsigned int command, unsigned long data) {
	int m = MINOR(inode->i_rdev);
	int oldSize = 0;
	int newSize = 0;
	char *temp = NULL;
    int error = 0;
    printk("_ring::ioctl\n");
	if(command == RING_IOR_GETSIZE) {
        if((error = verify_area(VERIFY_READ, (void *)data, _IOC_SIZE(data))) < 0) {
            printk("Proba odczytu z niepoprawnego miejsca w pamieci. Blad: %d\n", error);
            return error;
        }
		return buffers[m].bufferSize;
	} else if (command == RING_IOW_SETSIZE) {
        if((error = verify_area(VERIFY_WRITE, (void *)data, _IOC_SIZE(data))) < 0) {
            printk("Proba odczytu z niepoprawnego miejsca w pamieci. Blad: %d\n", error);
            return error;
        }
		if(data < MIN_BUFFER_SIZE || data > MAX_BUFFER_SIZE) {
			return -EINVAL;
		}
		if(data < buffers[m].bufferSize && buffers[m].bufferCount != 0) {
			return -EINVAL;
		}
		oldSize = buffers[m].bufferSize;
		newSize = data;
	
		temp = kmalloc(oldSize, GFP_KERNEL);
		if(temp == NULL) {
			return -ENOMEM;
		}
		
		down(&buffers[m].semaphore);
		memcpy(temp, buffers[m].buffer, oldSize);
		
		kfree(buffers[m].buffer);

		buffers[m].buffer = kmalloc(newSize, GFP_KERNEL);
		if(buffers[m].buffer == NULL) {
			up(&buffers[m].semaphore);
			return -ENOMEM;
		}
		buffers[m].bufferSize = newSize;

		memcpy(buffers[m].buffer, temp, oldSize);
		
		if(data < newSize) {
			buffers[m].bufferCount = 0;
			buffers[m].start = 0;
			buffers[m].end = 0;
		}
		up(&buffers[m].semaphore);
		kfree(temp);
	}
	
	return -EINVAL;
}

int ring_open(struct inode *inode, struct file *file) {
	int m = MINOR(inode->i_rdev) % 4;
	printk("_ring::open\n");
	MOD_INC_USE_COUNT;	
	down(&buffers[m].semaphore);
	buffers[m].useCount++;
	if (buffers[m].useCount == 1) {
		buffers[m].buffer = kmalloc(buffers[m].bufferSize,GFP_KERNEL);
		buffers[m].bufferCount = 0;
		buffers[m].start = 0;
		buffers[m].end = 0;
	}
	up(&buffers[m].semaphore);
	return 0;
	
}

void ring_release(struct inode *inode, struct file *file) {
	int m = MINOR(inode->i_rdev) % 4;
	printk("_ring::release\n");
	MOD_DEC_USE_COUNT;
	buffers[m].useCount--;
	if (buffers[m].useCount == 0) {
		kfree(buffers[m].buffer);
	}
}

int ring_read(struct inode *inode, struct file *file, char *pB, int count) {
	int m = MINOR(inode->i_rdev) % 4;
	int i;
	char tmp;
    printk("_ring::read\n");
	for(i = 0; i < count; i++) {
		while (buffers[m].bufferCount == 0) {
			if (buffers[m].useCount == 1) {
				return i;
			}
	
			interruptible_sleep_on(&buffers[m].readQueue);
			if (current->signal & ~current->blocked) {
				if (i == 0) {
					return -ERESTARTSYS;
				} else {
				    return i;
				}
			}
		}
		
		tmp = buffers[m].buffer[buffers[m].start];
		buffers[m].start++;
		if (buffers[m].start == buffers[m].bufferSize) {
			buffers[m].start = 0;
		}
		buffers[m].bufferCount--;
		wake_up(&buffers[m].writeQueue);
		put_user(tmp, pB+i);
	}		
	return count;
}

int ring_write(struct inode *inode, struct file *file, const char *pB, int count) {
	int m = MINOR(inode->i_rdev) % 4;
	int i;
	char tmp;
    printk("_ring::write\n");
	for(i = 0; i < count; i++) {
		tmp = get_user(pB+i);
		while (buffers[m].bufferCount == buffers[m].bufferSize) {
			interruptible_sleep_on(&buffers[m].writeQueue);
			if (current->signal & ~current->blocked) {
				if (i == 0) {
					return -ERESTARTSYS;
				} else {
				    return i;
				}
			}
		}
		buffers[m].buffer[buffers[m].end] = tmp;
		buffers[m].bufferCount++;
		buffers[m].end++;
		if (buffers[m].end == buffers[m].bufferSize) {
			buffers[m].end = 0;
		}
		wake_up(&buffers[m].readQueue);
	}
	return count;
}

int ring_init(void) {
	int i = 0;
	buffers = kmalloc(4 * sizeof(struct buffer), GFP_KERNEL);
    printk("_ring::init\n");
	if(buffers == NULL) {
		return -ENOMEM;
	}
	
	for(i = 0; i < 4; i++) {
		init_waitqueue(&buffers[i].writeQueue);
		init_waitqueue(&buffers[i].readQueue);
		buffers[i].useCount = 0;
		buffers[i].semaphore = MUTEX;
		buffers[i].bufferSize = DEFAULT_BUFFER_SIZE;
	}

	register_chrdev(RING_MAJOR, "ring", &ring_ops);
	return 0;
}

int init_module(void) {
    printk("_ring::init_module\n");
	return ring_init();
}

void cleanup_module(void) {
    printk("_ring::cleanup_module\n");
	kfree(buffers);
	unregister_chrdev(RING_MAJOR, "ring");
}
