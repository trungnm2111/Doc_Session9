Section: Linux Kernel Programming | Topic: Linux User-Kernel Communication: Syscall, IOCTL, procfs & Netlink Socket | Duration: 3 working days (8 hours/day)

Purpose: Understand and use the four standard interfaces for userspace-kernel communication, with an emphasis relevant to network applications. | Expectation: Four small C demo programs (syscall, ioctl, procfs, netlink).

Topics

• Overview of user-kernel communication interfaces: System Call, IOCTL, proc file system, Netlink Socket - what each is for and when to choose it

• System Call interface: deep dive + writing a simple C program that invokes a syscall directly via syscall(2)

• IOCTL interface: deep dive + writing a simple C program that issues an ioctl() to a device

• proc file system interface: deep dive + writing a simple C program that reads/writes a /proc entry

• Netlink Socket interface: deep dive + writing a simple C program that opens an AF_NETLINK socket and exchanges a message with the kernel (e.g. NETLINK_ROUTE)

• Why Netlink matters for network applications: the standard way Linux userspace networking tools (ip, ss, NetworkManager) talk to the kernel's networking subsystem

Readings / Resources • man 2 syscall • man 2 ioctl • man 5 proc • man 7 netlink

Studio / Lab

• Write a C program that calls an existing syscall directly through syscall(2) (bypassing the glibc wrapper) and explain why one might need to do this

• Write a C program that opens a device node and issues a simple ioctl() call

• Write a C program that reads a value from an existing /proc entry, and a second program that writes to a /proc entry exposed by the Session 8 driver

• Write a C program that opens an AF_NETLINK/NETLINK_ROUTE socket and requests the list of network interfaces from the kernel

