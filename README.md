# Operating-Systems-hw3

The goal of this assignment was to gain experience with kernel programming and to achieve a better understanding of the design and implementation of inter-process communication (IPC), kernel modules, and drivers.

In this assignment, we implemented a kernel module that provided a new IPC mechanism called a message slot.
A message slot was a character device file through which processes communicated.
The message slot device had multiple message channels active concurrently, allowing multiple processes to use them simultaneously.
When a process opened a message slot device file, it could use ioctl() to specify the ID of the message channel it wanted to use. Subsequently, read() and write() functions were used to receive and send messages on the selected channel.

Unlike pipes, a message channel preserved a message until it was explicitly overwritten, allowing the same message to be read multiple times by different processes.
