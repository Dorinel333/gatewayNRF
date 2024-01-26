/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

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
#include "jsonMsg.h"
//#include <cJSON.h>
//#include <zephyr/sys/timeutil.h>

#define NAME_LEN 30

#define MAX_ASCII_DATA_LEN 1000

#define FORMAT_STRING "%s"
#define ERROR_STRING "No received data"

#define SENSOR_JSON_STRING "test value sent %f", 12.5

#define SENSOR_JSON_STRING_SEND_1 "{" \
    "\"sensor\":{" \
        "\"id\": \"1\"," \
        "\"values\": [" \
            "{" \
                "\"type\": \"temperature\"," \
                "\"value\": %d.%d" \
            "}," \
            "{" \
                "\"type\": \"humidity\"," \
                "\"value\": %d.%d" \
            "}," \
            "{" \
                "\"type\": \"PM1_0\"," \
                "\"value\": %f" \
            "}," \
            "{" \
                "\"type\": \"PM2_5\"," \
                "\"value\": %f" \
            "}," \
            "{" \
                "\"type\": \"PM10\"," \
                "\"value\": %f" \
            "}" \
        "]" \
    "}" \
"}"

#define SENSOR_JSON_STRING_SEND_2 "{" \
    "\"sensor\":{" \
        "\"id\": \"2\"," \
        "\"values\": [" \
            "{" \
                "\"type\": \"temperature\"," \
                "\"value\": %d.%d" \
            "}," \
            "{" \
                "\"type\": \"humidity\"," \
                "\"value\": %d.%d" \
            "}," \
            "{" \
                "\"type\": \"PM1_0\"," \
                "\"value\": %f" \
            "}," \
            "{" \
                "\"type\": \"PM2_5\"," \
                "\"value\": %f" \
            "}," \
            "{" \
                "\"type\": \"PM10\"," \
                "\"value\": %f" \
            "}" \
        "]" \
    "}" \
"}"

#define SENSOR_JSON_STRING_SEND_3 "{" \
    "\"sensor\":{" \
        "\"id\": \"3\"," \
        "\"values\": [" \
            "{" \
                "\"type\": \"temperature\"," \
                "\"value\": %d.%d" \
            "}," \
            "{" \
                "\"type\": \"humidity\"," \
                "\"value\": %d.%d" \
            "}," \
            "{" \
                "\"type\": \"PM1_0\"," \
                "\"value\": %f" \
            "}," \
            "{" \
                "\"type\": \"PM2_5\"," \
                "\"value\": %f" \
            "}," \
            "{" \
                "\"type\": \"PM10\"," \
                "\"value\": %f" \
            "}" \
        "]" \
    "}" \
"}"

char received_data[MAX_ASCII_DATA_LEN] = {0};
char data_to_publish[MAX_ASCII_DATA_LEN] = {0};

bool onlyOnce = false;
bool publishReset = true;
bool dev_1_val_received_set_1 = false;
bool dev_1_val_received_set_2 = false;
bool dev_2_val_received_set_1 = false;
bool dev_2_val_received_set_2 = false;
bool dev_3_val_received_set_1 = false;
bool dev_3_val_received_set_2 = false;

uint32_t integer_part_temp_1 = 0;
uint32_t decimal_part_temp_1 = 0;
uint32_t integer_part_hum_1 = 0;
uint32_t decimal_part_hum_1 = 0;
uint32_t sps_value_1[10] = {0};
float sps_value_float_1[10] = {0};

uint32_t integer_part_temp_2 = 0;
uint32_t decimal_part_temp_2 = 0;
uint32_t integer_part_hum_2 = 0;
uint32_t decimal_part_hum_2 = 0;
uint32_t sps_value_2[10] = {0};
float sps_value_float_2[10] = {0};

uint32_t integer_part_temp_3 = 0;
uint32_t decimal_part_temp_3 = 0;
uint32_t integer_part_hum_3 = 0;
uint32_t decimal_part_hum_3 = 0;
uint32_t sps_value_3[10] = {0};
float sps_value_float_3[10] = {0};

void reset_values_1()
{
	integer_part_temp_1 = 0;
	decimal_part_temp_1 = 0;
	integer_part_hum_1 = 0;
	decimal_part_hum_1 = 0;
	for(size_t i = 0; i < 10; i++)
	{
		sps_value_1[i] = 0;
	}
	for(size_t i = 0; i < 10; i++)
	{
		sps_value_float_1[i] = 0;
	}
}

void reset_values_2()
{
	integer_part_temp_2 = 0;
	decimal_part_temp_2 = 0;
	integer_part_hum_2 = 0;
	decimal_part_hum_2 = 0;
	for(size_t i = 0; i < 10; i++)
	{
		sps_value_2[i] = 0;
	}
	for(size_t i = 0; i < 10; i++)
	{
		sps_value_float_2[i] = 0;
	}
}

void reset_values_3()
{
	integer_part_temp_3 = 0;
	decimal_part_temp_3 = 0;
	integer_part_hum_3 = 0;
	decimal_part_hum_3 = 0;
	for(size_t i = 0; i < 10; i++)
	{
		sps_value_3[i] = 0;
	}
	for(size_t i = 0; i < 10; i++)
	{
		sps_value_float_3[i] = 0;
	}
}

// void getCurrentTimestamp(char *timestampBuffer, size_t bufferSize) {
//     // Get the current time in seconds
//     time_t current_time = time(NULL);

//     // Convert the time to the desired format: "YYYY-MM-DDThh:mm:ss"
//     struct tm *time_info = localtime(&current_time);
//     strftime(timestampBuffer, bufferSize, "%Y-%m-%dT%H:%M:%S", time_info);
// }

//void convertToJSON() {
    // Create a buffer for JSON data
    //char json_buffer[512]; // Adjust the size as needed
    //char timestamp_buffer[20]; // Buffer for the timestamp

    // Get the current timestamp
    //getCurrentTimestamp(timestamp_buffer, sizeof(timestamp_buffer));

    // Format the JSON string
    // snprintf(json_buffer, sizeof(json_buffer),
    //          "{ \"bridge\": { \"id\": \"123abc\" }, \"sensors\": ["
    //          " { \"id\": \"temp_sensor\", \"type\": \"temperature\", \"value\": %d.%d, \"timestamp\": \"2023-10-10T12:00:00\" },"
    //          "] }",
    //          integer_part_temp, decimal_part_temp
    // );

    // Copy the JSON data to the target buffer
    //strncpy(data_to_publish, json_buffer, sizeof(data_to_publish));
	//snprintf(data_to_publish, sizeof(data_to_publish), json_buffer);
//}

/* Register log module */
LOG_MODULE_REGISTER(sampler, CONFIG_MQTT_SAMPLE_SAMPLER_LOG_LEVEL);

/* Register subscriber */
ZBUS_SUBSCRIBER_DEFINE(sampler, CONFIG_MQTT_SAMPLE_SAMPLER_MESSAGE_QUEUE_SIZE);

typedef union {
    float f;
    struct {
        unsigned int mantissa : 23;
        unsigned int exponent : 8;
        unsigned int sign : 1;
    } raw;
} myfloat;

void intToFloat(uint32_t intValue, myfloat* result)
{
    // Determine the sign bit
    result->raw.sign = (intValue >> 31) & 1;

    // Extract the exponent and mantissa
    int exponent = ((intValue >> 23) & 0xFF) - 127; // Adjust the bias (127) to match the IEEE 754 standard
    unsigned int mantissa = (intValue & 0x007FFFFF) | 0x00800000; // Add the implicit leading 1 to mantissa

    // Assign values to the result structure
    result->raw.exponent = exponent + 127; // Adjust the bias back
    result->raw.mantissa = mantissa;
}

static void deviceCustom_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

    memset(received_data, 0, sizeof(received_data));

    if (strncmp(addr_str, "CB:F4:29:F6:3D:40", strlen("CB:F4:29:F6:3D:40")) == 0) 
	{
        if (ad->len >= 8) {
			//printf("The set number is: %d\n", ad->data[30]);
			if(ad->data[30] == 1 && dev_1_val_received_set_1 == false)
			{
				// Combine the first 4 bytes as the integer part
				for (size_t i = 0; i < 4; i++) {
					integer_part_temp_1 |= (ad->data[i + 2] << (i * 8));
				}

				// Combine the next 4 bytes as the decimal part
				for (size_t i = 0; i < 4; i++) {
					decimal_part_temp_1 |= (ad->data[i + 6] << (i * 8));
				}

				for(size_t j = 0; j < 5;  j++)
				{
					// Combine the next 4 bytes as the decimal part
					for (size_t i = 0; i < 4; i++) {
						sps_value_1[j] |= (ad->data[(j * 4) + i + 10] << (i * 8));
					}
				}
				dev_1_val_received_set_1 = true;
			}
			else if(ad->data[30] == 2 && dev_1_val_received_set_2 == false)
			{
				// Combine the first 4 bytes as the integer part
				for (size_t i = 0; i < 4; i++) {
					integer_part_hum_1 |= (ad->data[i + 2] << (i * 8));
				}

				// Combine the next 4 bytes as the decimal part
				for (size_t i = 0; i < 4; i++) {
					decimal_part_hum_1 |= (ad->data[i + 6] << (i * 8));
				}

				for(size_t j = 0; j < 5;  j++)
				{
					// Combine the next 4 bytes as the decimal part
					for (size_t i = 0; i < 4; i++) {
						sps_value_1[j+5] |= (ad->data[(j * 4) + i + 10] << (i * 8));
					}
				}
				dev_1_val_received_set_2 = true;
			}

			// uint32_t intValue = 0b11000000000100000000000000000000; // Example value to the the converter
			// myfloat resultTest;
			// intToFloat(intValue, &resultTest);
		    // printf("The float value of the given uint32_t representation is: %f\n", resultTest.f);
			// printf("\n");

			if(dev_1_val_received_set_1 == true && dev_1_val_received_set_2 == true)
			{
				printf("AIR SENSING DEVICE #1:\n");
				printf("-----------------------------------\n");
				for(size_t k = 0; k < 10; k++)
				{
					myfloat result;
					intToFloat(sps_value_1[k], &result);
					printf("SPS Sensor %d Value: %f\n", k + 1, result.f);
					sps_value_float_1[k] = result.f;
				}
				printf("-----------------------------------\n");
				printf("Thingy Temperature Value: %d.%d\n", integer_part_temp_1, decimal_part_temp_1);
				printf("Thingy Humidity    Value: %d.%d\n", integer_part_hum_1, decimal_part_hum_1);
				printf("\n");
				char sensor_json_string[256];
				snprintf(sensor_json_string, sizeof(sensor_json_string),
				SENSOR_JSON_STRING_SEND_1, (int32_t)integer_part_temp_1, (int32_t)decimal_part_temp_1, (int32_t)integer_part_hum_1, (int32_t)decimal_part_hum_1, sps_value_float_1[0], sps_value_float_1[1], sps_value_float_1[3]
				);
				//snprintf(data_to_publish, sizeof(data_to_publish), SENSOR_JSON_STRING_SEND);//, integer_part_temp_3, decimal_part_temp_3, integer_part_hum_3, decimal_part_hum_3, sps_value_float_3[0], sps_value_float_3[1], sps_value_float_3[3]);
				set_string(sensor_json_string);
				//snprintf(data_to_publish, sizeof(data_to_publish), "MODULE #1: Thingy Temp: %d.%d; SPS P1.0: %f; SPS P2.5: %f", integer_part_temp_1, decimal_part_temp_1, sps_value_float_1[0], sps_value_float_1[1]);
				if (publishReset) {
					publishReset = !publishReset;
				}
				reset_values_1();
				dev_1_val_received_set_1 = false;
				dev_1_val_received_set_2 = false;
			}
        }
		net_buf_simple_init(ad, 0);
    }
	else if(strncmp(addr_str, "F4:6B:20:47:DA:2C", strlen("F4:6B:20:47:DA:2C")) == 0)
	{
		    if (ad->len >= 8) {
			//printf("The set number is: %d\n", ad->data[30]);
			if(ad->data[30] == 1 && dev_2_val_received_set_1 == false)
			{
				// Combine the first 4 bytes as the integer part
				for (size_t i = 0; i < 4; i++) {
					integer_part_temp_2 |= (ad->data[i + 2] << (i * 8));
				}

				// Combine the next 4 bytes as the decimal part
				for (size_t i = 0; i < 4; i++) {
					decimal_part_temp_2 |= (ad->data[i + 6] << (i * 8));
				}

				for(size_t j = 0; j < 5;  j++)
				{
					// Combine the next 4 bytes as the decimal part
					for (size_t i = 0; i < 4; i++) {
						sps_value_2[j] |= (ad->data[(j * 4) + i + 10] << (i * 8));
					}
				}
				dev_2_val_received_set_1 = true;
			}
			else if(ad->data[30] == 2 && dev_2_val_received_set_2 == false)
			{
				// Combine the first 4 bytes as the integer part
				for (size_t i = 0; i < 4; i++) {
					integer_part_hum_2 |= (ad->data[i + 2] << (i * 8));
				}

				// Combine the next 4 bytes as the decimal part
				for (size_t i = 0; i < 4; i++) {
					decimal_part_hum_2 |= (ad->data[i + 6] << (i * 8));
				}

				for(size_t j = 0; j < 5;  j++)
				{
					// Combine the next 4 bytes as the decimal part
					for (size_t i = 0; i < 4; i++) {
						sps_value_2[j+5] |= (ad->data[(j * 4) + i + 10] << (i * 8));
					}
				}
				dev_2_val_received_set_2 = true;
			}

			// uint32_t intValue = 0b11000000000100000000000000000000; // Example value to the the converter
			// myfloat resultTest;
			// intToFloat(intValue, &resultTest);
		    // printf("The float value of the given uint32_t representation is: %f\n", resultTest.f);
			//printf("\n");

			if(dev_2_val_received_set_1 == true && dev_2_val_received_set_2 == true)
			{
				printf("AIR SENSING DEVICE #2:\n");
				printf("-----------------------------------\n");
				for(size_t k = 0; k < 10; k++)
				{
					myfloat result;
					intToFloat(sps_value_2[k], &result);
					printf("SPS Sensor %d Value: %f\n", k + 1, result.f);
					sps_value_float_2[k] = result.f;
				}
				printf("-----------------------------------\n");
				printf("Thingy Temperature Value: %d.%d\n", integer_part_temp_2, decimal_part_temp_2);
				printf("Thingy Humidity    Value: %d.%d\n", integer_part_hum_2, decimal_part_hum_2);
				printf("\n");
				//snprintf(data_to_publish, sizeof(data_to_publish), "MODULE #2: Thingy Hum: %d.%d; SPS P4.0: %f ; SPS P10: %f", integer_part_hum_2, decimal_part_hum_2, sps_value_float_2[2], sps_value_float_2[3]);
				char sensor_json_string[256];
				snprintf(sensor_json_string, sizeof(sensor_json_string),
				SENSOR_JSON_STRING_SEND_2, (int32_t)integer_part_temp_2, (int32_t)decimal_part_temp_2, (int32_t)integer_part_hum_2, (int32_t)decimal_part_hum_2, sps_value_float_2[0], sps_value_float_2[1], sps_value_float_2[3]
				);
				//snprintf(data_to_publish, sizeof(data_to_publish), SENSOR_JSON_STRING_SEND);//, integer_part_temp_3, decimal_part_temp_3, integer_part_hum_3, decimal_part_hum_3, sps_value_float_3[0], sps_value_float_3[1], sps_value_float_3[3]);
				set_string(sensor_json_string);
				if (publishReset) {
					publishReset = !publishReset;
				}
				reset_values_2();
				dev_2_val_received_set_1 = false;
				dev_2_val_received_set_2 = false;
			}
        }
		net_buf_simple_init(ad, 0);
    }
	else if((strncmp(addr_str, "D9:40:7F:F4:02:22", strlen("D9:40:7F:F4:02:22")) == 0))
	{
        if (ad->len >= 8) {
			//printf("The set number is: %d\n", ad->data[30]);
			if(ad->data[30] == 1 && dev_3_val_received_set_1 == false)
			{
				// Combine the first 4 bytes as the integer part
				for (size_t i = 0; i < 4; i++) {
					integer_part_temp_3 |= (ad->data[i + 2] << (i * 8));
				}

				// Combine the next 4 bytes as the decimal part
				for (size_t i = 0; i < 4; i++) {
					decimal_part_temp_3 |= (ad->data[i + 6] << (i * 8));
				}

				for(size_t j = 0; j < 5;  j++)
				{
					// Combine the next 4 bytes as the decimal part
					for (size_t i = 0; i < 4; i++) {
						sps_value_3[j] |= (ad->data[(j * 4) + i + 10] << (i * 8));
					}
				}
				dev_3_val_received_set_1 = true;
			}
			else if(ad->data[30] == 2 && dev_3_val_received_set_2 == false)
			{
				// Combine the first 4 bytes as the integer part
				for (size_t i = 0; i < 4; i++) {
					integer_part_hum_3 |= (ad->data[i + 2] << (i * 8));
				}

				// Combine the next 4 bytes as the decimal part
				for (size_t i = 0; i < 4; i++) {
					decimal_part_hum_3 |= (ad->data[i + 6] << (i * 8));
				}

				for(size_t j = 0; j < 5;  j++)
				{
					// Combine the next 4 bytes as the decimal part
					for (size_t i = 0; i < 4; i++) {
						sps_value_3[j+5] |= (ad->data[(j * 4) + i + 10] << (i * 8));
					}
				}
				dev_3_val_received_set_2 = true;
			}

			if(dev_3_val_received_set_1 == true && dev_3_val_received_set_2 == true)
			{
				printf("AIR SENSING DEVICE #3:\n");
				printf("-----------------------------------\n");
				for(size_t k = 0; k < 10; k++)
				{
					myfloat result;
					intToFloat(sps_value_3[k], &result);
					printf("SPS Sensor %d Value: %f\n", k + 1, result.f);
					sps_value_float_3[k] = result.f;
				}
				printf("-----------------------------------\n");
				printf("Thingy Temperature Value: %d.%d\n", integer_part_temp_3, decimal_part_temp_3);
				printf("Thingy Humidity    Value: %d.%d\n", integer_part_hum_3, decimal_part_hum_3);
				printf("\n");
				char sensor_json_string[256];
				snprintf(sensor_json_string, sizeof(sensor_json_string),
				SENSOR_JSON_STRING_SEND_3, (int32_t)integer_part_temp_3, (int32_t)decimal_part_temp_3, (int32_t)integer_part_hum_3, (int32_t)decimal_part_hum_3, sps_value_float_3[0], sps_value_float_3[1], sps_value_float_3[3]
				);
				//snprintf(data_to_publish, sizeof(data_to_publish), SENSOR_JSON_STRING_SEND);//, integer_part_temp_3, decimal_part_temp_3, integer_part_hum_3, decimal_part_hum_3, sps_value_float_3[0], sps_value_float_3[1], sps_value_float_3[3]);
				set_string(sensor_json_string);
				printk("Lenght sent: %d\n", sizeof(data_to_publish));

				if (publishReset) {
					publishReset = !publishReset;
				}
				reset_values_3();
				dev_3_val_received_set_1 = false;
				dev_3_val_received_set_2 = false;
			}
        }
		net_buf_simple_init(ad, 0);
	}
}

int observerCustom_start(void)
{
	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
		.options    = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};
	int err;

	err = bt_le_scan_start(&scan_param, deviceCustom_found);
	if (err) {
		printf("Start scanning failed (err %d)\n", err);
		return err;
	}
	printf("Started scanning...\n");

	return 0;
}

static void sample(void)
{
	struct payload payload = { 0 };
	int err, len;

	if(!onlyOnce)
	{
		err = bt_enable(NULL);
		if (err) {
			printf("Bluetooth init failed (err %d)\n", err);
			return;
		}

		(void)observerCustom_start();

		onlyOnce = !onlyOnce;
	}

	/* The payload is user defined and can be sampled from any source.
	 * Default case is to populate a string and send it on the payload channel.
	 */
	if(strlen(data_to_publish) == 0)
	{
		len = snprintf(payload.string, sizeof(payload.string), ERROR_STRING);
		if ((len < 0) || (len >= sizeof(payload))) {
			LOG_ERR("Failed to construct message, error: %d", len);
			SEND_FATAL_ERROR();
			return;
		}
	}
	else
	{
		len = snprintf(payload.string, sizeof(payload.string), FORMAT_STRING, data_to_publish);
		if ((len < 0) || (len >= sizeof(payload))) {
			LOG_ERR("Failed to construct message, error: %d", len);
			SEND_FATAL_ERROR();
			return;
		}
	}

	err = zbus_chan_pub(&PAYLOAD_CHAN, &payload, K_SECONDS(1));
	if (err) {
		LOG_ERR("zbus_chan_pub, error:%d", err);
		SEND_FATAL_ERROR();
	}
}

static void sampler_task(void)
{
	const struct zbus_channel *chan;

	while (!zbus_sub_wait(&sampler, &chan, K_FOREVER)) {
		if (&TRIGGER_CHAN == chan) {
			sample();
			publishReset = !publishReset;
			memset(data_to_publish, 0, sizeof(data_to_publish));
		}
	}
}

K_THREAD_DEFINE(sampler_task_id,
		CONFIG_MQTT_SAMPLE_SAMPLER_THREAD_STACK_SIZE,
		sampler_task, NULL, NULL, NULL, 3, 0, 0);
