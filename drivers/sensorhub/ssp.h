/*
 *  Copyright (C) 2015, Samsung Electronics Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifndef __SSP_H__
#define __SSP_H__

#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/iio/iio.h>
#include <linux/wakelock.h>

#include "ssp_type_define.h"
#include "ssp_platform.h"
#include "factory/ssp_factory.h"
#define ssp_dbg(fmt, ...) do { \
        pr_debug("[SSP] " fmt "\n", ##__VA_ARGS__); \
        } while (0)

#define ssp_info(fmt, ...) do { \
        pr_info("[SSP] " fmt "\n", ##__VA_ARGS__); \
        } while (0)

#define ssp_err(fmt, ...) do { \
        pr_err("[SSP] " fmt "\n", ##__VA_ARGS__); \
        } while (0)

#define ssp_dbgf(fmt, ...) do { \
        pr_debug("[SSP] %20s(%4d): " fmt "\n", __func__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define ssp_infof(fmt, ...) do { \
        pr_info("[SSP] %20s(%4d): " fmt "\n", __func__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define ssp_errf(fmt, ...) do { \
        pr_err("[SSP] %20s(%4d): " fmt "\n", __func__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define MAKE_WORD(H,L) ((((u16)H) << 8 ) & 0xff00 ) | ((((u16)L)) & 0x00ff )
#define WORD_TO_LOW(w) ((u8)((w) & 0xff ))
#define WORD_TO_HIGH(w) ((u8)(((w) >>8 ) & 0xff))

#define SUCCESS 0
#define FAIL    -2
#define ERROR   -1

#define DEFUALT_POLLING_DELAY   (200)

#ifdef CONFIG_SENSORS_SSP_PROXIMITY_STK3X3X 
#define CONFIG_SENSROS_SSP_PROXIMITY_THRESH_CAL
#endif

#define SENSOR_NAME_MAX_LEN             35

struct sensor_info {
    char name[SENSOR_NAME_MAX_LEN];
    bool enable;
    int report_mode;
    int get_data_len;
    int report_data_len;
};

enum {
	INITIALIZATION_STATE = 0,
	NO_SENSOR_STATE,
	ADD_SENSOR_STATE,
	RUNNING_SENSOR_STATE,
};

/* SSP_INSTRUCTION_CMD */
enum {
	REMOVE_SENSOR = 0,
	ADD_SENSOR,
	CHANGE_DELAY,
	GO_SLEEP,
	REMOVE_LIBRARY,
	ADD_LIBRARY,
};

enum {
	RESET_INIT_VALUE = 0,
	RESET_KERNEL_NO_EVENT = 1,
	RESET_KERNEL_TIME_OUT,
	RESET_KERNEL_COM_FAIL,
	RESET_KERNEL_SYSFS,
	RESET_MCU_CRASHED,
};

enum {
	SSP_ST_SHUTDOWN = 0,
	SSP_ST_FW_DL_DONE,
	SSP_ST_RUN,
	SSP_ST_RQ_RESET,
	SSP_ST_ERROR,
	SSP_ST_MAX,
};

struct sensor_value {
	union {
		struct { /* accel, gyro, mag */
			s16 x;
			s16 y;
			s16 z;
			u32 gyro_dps;
		} __attribute__((__packed__));
		struct { /*calibrated mag, gyro*/
			s16 cal_x;
			s16 cal_y;
			s16 cal_z;
			u8 accuracy;
		} __attribute__((__packed__));
		struct { /*uncalibrated mag, gyro*/
			s16 uncal_x;
			s16 uncal_y;
			s16 uncal_z;
			s16 offset_x;
			s16 offset_y;
			s16 offset_z;
		} __attribute__((__packed__));
		struct { /* rotation vector */
			s32 quat_a;
			s32 quat_b;
			s32 quat_c;
			s32 quat_d;
			u8 acc_rot;
		} __attribute__((__packed__));
		struct { /* light */
			u32 lux;
			s32 cct;
			u32 brightness;
			u16 r;
			u16 g;
			u16 b;
			u16 w;
			u16 a_time;
			u16 a_gain;
		} __attribute__((__packed__));
		struct { /* pressure */
			s32 pressure;
			s16 temperature;
			s32 pressure_cal;
			s32 pressure_sealevel;
		} __attribute__((__packed__));
		struct { /* step detector */
			u8 step_det;
		};
		struct { /* step counter */
			u32 step_diff;
			u64 step_total;
		} __attribute__((__packed__));
		struct { /* proximity */
			u8 prox;
			u16 prox_ex;
		} __attribute__((__packed__));
		struct { /* proximity raw */
			u16 prox_raw[4];
		};
		struct { /* significant motion */
			u8 sig_motion;
		};
		struct { /* tilt detector */
			u8 tilt_detector;
		};
		struct { /* pickup gesture */
			u8 pickup_gesture;
		};
		struct { /* light auto brightness */
			s32 ab_lux;
			u32 ab_brightness;
		};
		struct meta_data_event { /* meta data */
			s32 what;
			s32 sensor;
		} __attribute__((__packed__)) meta_data;
		u8 data[20];
	};
	u64 timestamp;
} __attribute__((__packed__));

struct sensor_delay {
	int sampling_period;	/* delay (ms)*/
	int max_report_latency;	 /* batch_max_latency*/
};

struct calibraion_data {
	s16 x;
	s16 y;
	s16 z;
};

struct sensor_info;

struct ssp_data {
	/* yum ToDo: it will be removed */
	bool is_refresh_done;

	bool is_probe_done;
	struct wake_lock ssp_wake_lock;
	unsigned int curr_fw_rev;
	struct delayed_work work_refresh;
	struct delayed_work work_power_on;
	int cnt_reset;
	unsigned int cnt_no_event_reset;

	struct timer_list ts_sync_timer;
	struct workqueue_struct *ts_sync_wq;
	struct work_struct work_ts_sync;
/* platform */
	void *platform_data;
	
/* comm */
	struct mutex comm_mutex;
	struct mutex pending_mutex;
	struct list_head pending_list;
	unsigned int cnt_timeout;
	unsigned int cnt_com_fail;

/* debug */
	char sensor_state[BIG_DATA_SENSOR_TYPE_MAX + 1];

	struct timer_list debug_timer;
	struct workqueue_struct *debug_wq;
	struct work_struct work_debug;
	bool debug_enable;

	char last_ap_status;
	char last_resume_status;

	bool is_reset_from_kernel;
	bool is_reset_from_sysfs;
	bool is_reset_started;
	int reset_type;

	char *sensor_dump[SENSOR_TYPE_MAX];
#ifdef CONFIG_SSP_ENG_DEBUG
	char register_value[5];
#endif

/* sensor hub dump */
	char *callstack_data;
	int dump_index;
	
/* sensor */
	struct mutex enable_mutex;
	uint64_t sensor_probe_state;	/* uSensorState */
	atomic64_t sensor_en_state;		/* aSensorEnable */
	u64 latest_timestamp[SENSOR_TYPE_MAX];
	bool is_data_reported[SENSOR_TYPE_MAX];

	int sensor_status[SENSOR_TYPE_MAX];
	struct sensor_value buf[SENSOR_TYPE_MAX];
	struct sensor_delay delay[SENSOR_TYPE_MAX];
	struct sensor_info info[SENSOR_TYPE_MAX];

	/* device */
	struct device *mcu_device;
	struct miscdevice batch_io_device;
	struct iio_dev *indio_devs[SENSOR_TYPE_MAX];
	struct iio_chan_spec indio_channels[SENSOR_TYPE_MAX];
	struct device *devices[SENSOR_TYPE_MAX];
	struct miscdevice scontext_device;
	struct miscdevice injection_device;

	/* ops */
	int fw_dl_state;
	bool is_accel_alert;
	bool is_barcode_enabled;


#ifdef CONFIG_SENSORS_SSP_ACCELOMETER
	struct  accelometer_sensor_operations *accel_ops;
	int accel_position;
	struct calibraion_data accelcal;
#endif
#ifdef CONFIG_SENSORS_SSP_GYROSCOPE
	struct  gyroscope_sensor_operations *gyro_ops;
	struct calibraion_data gyrocal;
	int gyro_position;
#endif
#ifdef CONFIG_SENSORS_SSP_MAGNETIC
	struct  magnetic_sensor_operations *magnetic_ops;
	int mag_position;
	unsigned char pdc_matrix[PDC_SIZE];
	unsigned char uFuseRomData[3];
	unsigned char geomag_cntl_regdata;
	bool is_geomag_raw_enabled;
	struct calibraion_data magcal;
#endif
#ifdef CONFIG_SENSORS_SSP_PROXIMITY
	struct  proximity_sensor_operations *proximity_ops;
	unsigned int prox_raw_avg[4];
	bool is_proxraw_enabled;
#if defined(CONFIG_SENSORS_SSP_PROXIMITY_AUTO_CAL_TMD3725)
	u8 prox_thresh[PROX_THRESH_SIZE]; /* high, low, detect_hight, detect_low*/
#else
	u16 prox_thresh[PROX_THRESH_SIZE]; /* high, low*/
#endif
#if defined(CONFIG_SENSROS_SSP_PROXIMITY_THRESH_CAL)
	struct delayed_work work_prox_cal_off;
	bool is_prox_cal;
#ifdef CONFIG_SENSORS_SSP_PROXIMITY_STK3X3X
	u16 prox_thresh_addval[PROX_THRESH_SIZE+1];
#else
	u16 prox_thresh_addval[PROX_THRESH_SIZE];
#endif
#endif
	int prox_trim;
#endif
#ifdef CONFIG_SENSORS_SSP_LIGHT
	struct  light_sensor_operations *light_ops;
	int light_coef[LIGHT_COEF_SIZE];
	int light_log_cnt;
	int light_cct_log_cnt;
	int light_ab_log_cnt;
	u32 light_position[6];
	int brightness;
	int last_brightness_level;
	bool camera_lux_en;
	int camera_lux;
	int camera_lux_hysteresis[2];
	int camera_br_hysteresis[2];
#endif
#ifdef CONFIG_SENSORS_SSP_BAROMETER
	struct  barometer_sensor_operations *barometer_ops;
#endif
};

#endif /* __SSP_H__ */
