#ifndef RING_IOCTL_H
#define RING_IOCTL_H

/* Use 'r' (114) as magic number */
#define RING_IOC_MAGIC 'r'                            
#define RING_IOR_GETSIZE _IOR(RING_IOC_MAGIC, 1, int)       
#define RING_IOW_SETSIZE _IOW(RING_IOC_MAGIC, 2, int) 

#endif /* RING_IOCTL_H */
