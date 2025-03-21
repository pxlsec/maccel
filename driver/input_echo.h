#ifndef _INPUT_ECHO_
#define _INPUT_ECHO_

#include "fixedptc.h"
#include "linux/cdev.h"
#include "linux/fs.h"
#include "speed.h"
#include <linux/version.h>

int create_char_device(void);
void destroy_char_device(void);

static struct cdev device;
static struct class *device_class;
static dev_t device_number;

/*
 * Convert an int into an array of four/eight bytes, in big endian (MSB first)
 */
static void fpt_to_int_be_bytes(fpt num, char bytes[sizeof(fpt)]) {
#define byte(i) (FIXEDPT_BITS - (i * 8))
  bytes[0] = (num >> byte(1)) & 0xFF; // Most significant byte
  bytes[1] = (num >> byte(2)) & 0xFF;
  bytes[2] = (num >> byte(3)) & 0xFF;
#if FIXEDPT_BITS == 64
  if (sizeof(fpt) == 8) {
    bytes[3] = (num >> byte(4)) & 0xFF;
    bytes[4] = (num >> byte(5)) & 0xFF;
    bytes[5] = (num >> byte(6)) & 0xFF;
    bytes[6] = (num >> byte(7)) & 0xFF;
    bytes[7] = num & 0xFF; // Least significant byte
    return;
  }
#else
  bytes[3] = num & 0xFF; // Least significant byte
#endif
}

static ssize_t read(struct file *f, char __user *user_buffer, size_t size,
                    loff_t *offset) {
  dbg("echoing speed to userspace: %s", fptoa(LAST_INPUT_MOUSE_SPEED));

  char be_bytes_for_int[sizeof(fpt)] = {0};
  fpt_to_int_be_bytes(LAST_INPUT_MOUSE_SPEED, be_bytes_for_int);

  int err =
      copy_to_user(user_buffer, be_bytes_for_int, sizeof(be_bytes_for_int));
  if (err)
    return -EFAULT;

  return sizeof(be_bytes_for_int);
}

struct file_operations fops = {.owner = THIS_MODULE, .read = read};

int create_char_device(void) {
  int err;
  err = alloc_chrdev_region(&device_number, 0, 1, "maccel");
  if (err)
    return -EIO;

  cdev_init(&device, &fops);
  cdev_add(&device, device_number, 1);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 4, 0))
  device_class = class_create(THIS_MODULE, "maccel");
#else
  device.owner = THIS_MODULE;
  device_class = class_create("maccel");
#endif

  if (IS_ERR(device_class)) {
    goto err_free_cdev;
  }

  device_create(device_class, NULL, device_number, NULL, "maccel");

  return 0;

err_free_cdev:
  cdev_del(&device);
  unregister_chrdev_region(device_number, 1);
  return -EIO;
}

void destroy_char_device(void) {
  device_destroy(device_class, device_number);
  class_destroy(device_class);
  cdev_del(&device);
  unregister_chrdev_region(device_number, 1);
}

#endif // !_INPUT_ECHO_
