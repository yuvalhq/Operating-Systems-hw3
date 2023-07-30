#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

#define SUCCESS 0
#define FAIL -1
#define ERROR 1
#define MAJOR_NUM 235
#define MINOR_NUM_CAP 257
#define DEVICE_NAME "message_slot"
#define BUF_LEN 128
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned int)
#define MAX_CHANNEL_ID 2147483647  // 2^31 - 1

typedef struct channelNode {
    unsigned int channel_id;
    int message_len;
    char message[BUF_LEN];
    struct channelNode *next;
} channelNode;

typedef struct channelList {
    channelNode *head;
} channelList;

//static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset);
//static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset);
//static int device_open(struct inode *inode, struct file *file);
//static int device_release(struct inode *inode, struct file *file);
//static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long ioctl_param);

#endif 