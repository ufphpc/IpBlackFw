#ifndef _INTERFACE_IO_H
#define _INTERFACE_IO_H

#pragma pack(8)
struct fw_set_info {
	__u32 ip;
};
#pragma pack()

#define FW_IOCTL_MAGIC	's'
#define FW_IOCTL_ADD_INFO _IO(FW_IOCTL_MAGIC, 1)
#define FW_IOCTL_DEL_INFO _IO(FW_IOCTL_MAGIC, 2)

#define DRIVER_DEVICE_NAME "fwDevice"
#define DRIVER_DEVICE_PATH "/dev/" DRIVER_DEVICE_NAME

#endif