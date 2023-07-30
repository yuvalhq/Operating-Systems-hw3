#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include "message_slot.h"

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset);
static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset);
static int device_open(struct inode *inode, struct file *file);
static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long ioctl_param);

struct file_operations fo = {
        .owner = THIS_MODULE,
        .read = device_read,
        .write = device_write,
        .open = device_open,
        .unlocked_ioctl = device_ioctl,
};
static channelList channel_lists[MINOR_NUM_CAP];

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset){
    channelNode* cur_channel;
    int put_user_status;
    unsigned int minor_num;
    size_t i;

    if (buffer == NULL) {
        printk("Buffer is NULL\n");
        return -EINVAL;
    }

    minor_num = iminor(file -> f_inode);
    cur_channel = (channelNode *) file -> private_data;
    if (cur_channel == NULL) {
        printk("No channel has been set for this fd, minor %d\n", minor_num);
        return -EINVAL;
    }

    if (cur_channel -> message_len > length) {
        printk("The provided buffer length is too small in channel %d, minor %d\n", cur_channel -> channel_id, minor_num);
        return -ENOSPC;
    }

    if (cur_channel -> message_len == 0) {
        printk("No or empty message in channel %d, minor %d\n", cur_channel -> channel_id, minor_num);
        return -EWOULDBLOCK;
    }

    for (i = 0; i < cur_channel -> message_len; i++) {
        put_user_status = put_user(cur_channel -> message[i], &buffer[i]);
        if (put_user_status != 0) {
            printk("put_user failed\n");
            return -EFAULT;
        }
    }

    printk("Read the message from channel %d, minor %d\n", cur_channel -> channel_id, minor_num);
    return i;
}

static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset){
    channelNode* cur_channel;
    int get_user_status;
    unsigned int minor_num;
    ssize_t i;
    char temp_message[BUF_LEN];

    if (buffer == NULL) {
        printk("Buffer is NULL\n");
        return -EINVAL;
    }

    if (length > BUF_LEN) {
        printk("The new message length is too long\n");
        return -EMSGSIZE;
    }

    if (length == 0) {
        printk("The new message is empty\n");
        return -EMSGSIZE;
    }

    minor_num = iminor(file -> f_inode);
    cur_channel = (channelNode *) file -> private_data;
    if (cur_channel == NULL) {
        printk("No channel has been set for this fd, minor %d\n", minor_num);
        return -EINVAL;
    }

    // transfer the meaassge to temp_message
    // this is done in order to avoid overwriting the message in case of an error
    for (i = 0; i < length; i++) {
        get_user_status = get_user(temp_message[i], &buffer[i]);
        if (get_user_status != 0){
            printk("get_user failed\n");
            return -EFAULT;
        }
    }

    // update the message
    cur_channel -> message_len = i;
    for (i = 0; i < cur_channel -> message_len; i++) {
        cur_channel -> message[i] = temp_message[i];
    }

    printk("Write message to channel %d, minor %d\n", cur_channel -> channel_id, minor_num);
    return i;
}

static int device_open(struct inode *inode, struct file *file) {
    // no need for further actions here
    return SUCCESS;
}

static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long  ioctl_param ) {
    channelNode *channel_node, *prev_node;
    unsigned int minor_num;

    if (ioctl_command_id != MSG_SLOT_CHANNEL) {
        printk("Wrong ioctl command\n");
        return -EINVAL;
    }

    if (ioctl_param == 0 || ioctl_param > MAX_CHANNEL_ID) {
        printk("Wrong channel id\n");
        return -EINVAL;
    }

    // find the channel with the required channel id
    prev_node = NULL;
    minor_num = iminor(file->f_inode);
    channel_node = channel_lists[minor_num].head;
    while (channel_node != NULL) {
        if(channel_node -> channel_id == ioctl_param) {
            break;
        }
        prev_node = channel_node;
        channel_node = channel_node -> next;
    }

    // if the channel with the required channel id don't exist yet, create it
    if (channel_node == NULL) {
        channel_node = (channelNode *) kmalloc(sizeof(channelNode), GFP_KERNEL);
        if (channel_node == NULL) {
            printk("Memory allocation fail\n");
            return -ENOMEM;
        }

        if (prev_node == NULL) {
            // first channel of this minor
            channel_lists[minor_num].head = channel_node;
        } else {
            prev_node -> next = channel_node;
        }

        channel_node -> channel_id = ioctl_param;
        channel_node -> next = NULL;
        channel_node -> message_len = 0;
    }

    file -> private_data = channel_node;

    printk("Ioctl of channel %d, minor %d\n", channel_node->channel_id, minor_num);
    return SUCCESS;
}

static int __init init_message_slot(void){
    int reg_chrdev = -1;
    size_t i;

    // Register driver capabilities.
    reg_chrdev = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fo);

    // Negative values signify an error
    if(reg_chrdev < 0 ) {
        printk(KERN_ERR "%s registration failed for %d\n", DEVICE_NAME, MAJOR_NUM );
        return reg_chrdev;
    }

    // init the channel lists
    for (i = 0; i < MINOR_NUM_CAP; i++){
        channel_lists[i].head = NULL;
    }

    printk("Registeration is successful\n");
    return SUCCESS;
}

static void __exit cleanup_message_slot(void){
    channelNode *channel_node, *temp;
    size_t i;

    // Free all the allocated memory by the channels
    for (i = 0; i < MINOR_NUM_CAP; i++){
        channel_node = channel_lists[i].head;
        while (channel_node != NULL){
            temp = channel_node;
            channel_node = channel_node -> next;
            kfree(temp);
        }
    }

    // Unregister the device
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
}

module_init(init_message_slot);
module_exit(cleanup_message_slot);
