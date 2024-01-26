#ifndef _JSONMSG_H_
#define _JSONMSG_H_

#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include "message_channel.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

extern char string_to_publish[950];

void set_string(const char *string);
const char *get_string();

#endif /* _JSONMSG_H_ */

