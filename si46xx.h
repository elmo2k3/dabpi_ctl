/* 
 * dabpi_ctl - raspberry pi fm/fmhd/dab receiver board control interface
 * Copyright (C) 2014  Bjoern Biesenbach <bjoern@bjoern-b.de>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __SI46XX_H__
#define __SI46XX_H__

#include <stdint.h>

#define SI46XX_RD_REPLY 0x00
#define SI46XX_POWER_UP 0x01
#define SI46XX_HOST_LOAD 0x04
#define SI46XX_FLASH_LOAD 0x05
#define SI46XX_LOAD_INIT 0x06
#define SI46XX_BOOT 0x07
#define SI46XX_GET_PART_INFO 0x08
#define SI46XX_GET_SYS_STATE 0x09
#define SI46XX_SET_PROPERTY 0x13
#define SI46XX_GET_PROPERTY 0x14
#define SI46XX_FM_TUNE_FREQ 0x30
#define SI46XX_FM_SEEK_START 0x31
#define SI46XX_FM_RSQ_STATUS 0x32
#define SI46XX_FM_ACF_STATUS 0x33
#define SI46XX_FM_RDS_STATUS 0x34
#define SI46XX_FM_RDS_BLOCKCOUNT 0x35

#define SI46XX_DAB_TUNE_FREQ 0xB0
#define SI46XX_DAB_DIGRAD_STATUS 0xB2
#define SI46XX_DAB_GET_SERVICE_LINKING_INFO 0xB7
#define SI46XX_DAB_SET_FREQ_LIST 0xB8
#define SI46XX_DAB_GET_DIGITAL_SERVICE_LIST 0x80
#define SI46XX_DAB_START_DIGITAL_SERVICE 0x81
#define SI46XX_DAB_GET_ENSEMBLE_INFO 0xB4
#define SI46XX_DAB_GET_AUDIO_INFO 0xBD
#define SI46XX_DAB_GET_SUBCHAN_INFO 0xBE

#define SI46XX_FM_INT_CTL_ENABLE 0x0000
#define SI46XX_FM_INT_CTL_REPEAT 0x0001
#define SI46XX_FM_DIGITAL_IO_OUTPUT_SELECT 0x0200
#define SI46XX_DIGITAL_IO_OUTPUT_FORMAT 0x0202
#define SI46XX_PIN_CONFIG_ENABLE 0x0800
#define SI46XX_FM_VALID_MAX_TUNE_ERROR 0x3200
#define SI46XX_FM_VALID_RSSI_TIME 0x3201
#define SI46XX_FM_VALID_RSSI_THRESHOLD 0x3202
#define SI46XX_FM_VALID_SNR_TIME 0x3203
#define SI46XX_FM_VALID_SNR_THRESHOLD 0x3204
#define SI46XX_FM_SOFTMUTE_SNR_LIMITS 0x3500
#define SI46XX_FM_SOFTMUTE_SNR_ATTENUATION 0x3501
#define SI46XX_FM_TUNE_FE_CFG 0x1712
#define SI46XX_FM_RDS_CONFIG 0x3C02
#define SI46XX_FM_AUDIO_DE_EMPHASIS 0x3900

#define SI46XX_DAB_TUNE_FE_CFG 0x1712
#define SI46XX_DAB_TUNE_FE_VARM 0x1710
#define SI46XX_DAB_TUNE_FE_VARB 0x1711
#define SI46XX_DAB_CTRL_DAB_MUTE_ENABLE 0xB400
#define SI46XX_DAB_CTRL_DAB_MUTE_SIGNAL_LEVEL_THRESHOLD 0xB501
#define SI46XX_DAB_CTRL_DAB_MUTE_SIGLOW_THRESHOLD 0xB505

#define SI46XX_DIGITAL_SERVICE_INT_SOURCE 0x8100

#define SI46XX_AUDIO_DE_EMPHASIS_EU 0x01 //0x00=75us -> defaults to USA (default), 0x01=50us -> defaults to Europe, 0x02=disabled
#define SI46XX_AUDIO_DE_EMPHASIS_US 0x00 //0x00=75us -> defaults to USA (default), 0x01=50us -> defaults to Europe, 0x02=disabled
#define SI46XX_AUDIO_DE_EMPHASIS_OFF 0x02 //0x00=75us -> defaults to USA (default), 0x01=50us -> defaults to Europe, 0x02=disabled

// DAB channels
#define CHAN_5A 174928
#define CHAN_5B 176640
#define CHAN_5C 178352
#define CHAN_5D 180064
#define CHAN_6A 181936
#define CHAN_6B 183648
#define CHAN_6C 185360
#define CHAN_6D 187072
#define CHAN_7A 188928
#define CHAN_7B 190640
#define CHAN_7C 192352
#define CHAN_7D 194064
#define CHAN_8A 195936
#define CHAN_8B 197648
#define CHAN_8C 199360
#define CHAN_8D 201072
#define CHAN_9A 202928
#define CHAN_9B 204640
#define CHAN_9C 206352
#define CHAN_9D 208064
#define CHAN_10A 209936
#define CHAN_10B 211648
#define CHAN_10C 213360
#define CHAN_10D 215072
#define CHAN_11A 216928
#define CHAN_11B 218640
#define CHAN_11C 220352
#define CHAN_11D 222064
#define CHAN_12A 223936
#define CHAN_12B 225648
#define CHAN_12C 227360
#define CHAN_12D 229072
#define CHAN_13A 230784
#define CHAN_13B 232496
#define CHAN_13C 234208
#define CHAN_13D 235776
#define CHAN_13E 237488
#define CHAN_13F 239200

#define MAX_SERVICES 32
#define MAX_COMPONENTS 15

struct dab_service_t{
	uint32_t service_id;
	uint8_t service_info1;
	uint8_t service_info2;
	uint8_t service_info3;
	char service_label[17];
	uint8_t num_components;
	// only one component by now
	uint16_t component_id[MAX_COMPONENTS];
	uint8_t component_info[MAX_COMPONENTS];
	uint8_t component_valid_flags[MAX_COMPONENTS];
};

struct dab_digrad_status_t{
	uint8_t hard_mute_int;
	uint8_t fic_error_int;
	uint8_t acq_int;
	uint8_t rssi_h_int;
	uint8_t rssi_l_int;
	uint8_t hardmute;
	uint8_t fic_error;
	uint8_t acq;
	uint8_t valid;
	int8_t rssi; // -128-63
	int8_t snr; // 0-20
	uint8_t fic_quality; // 0-100
	uint8_t cnr; // 0-54
	uint16_t fib_error_count;
	uint32_t frequency;
	uint8_t tuned_index;
	uint8_t fft_offset;
	uint16_t read_ant_cap;
	uint16_t cu_level; // 0-470
};

struct fm_rds_data_t{
	uint8_t sync;
	uint16_t pi;
	uint8_t pty;
	char ps_name[9];
	char radiotext[129];
	uint16_t group_0a_flags;
	uint32_t group_2a_flags;
}fm_rds_data;

struct dab_service_list_t{
	uint16_t list_size;
	uint16_t version;
	uint8_t num_services;
	struct dab_service_t services[MAX_SERVICES];
}dab_service_list;

void si46xx_init(void);
void si46xx_init_dab(void);
void si46xx_init_fm(void);
void si46xx_fm_tune_freq(uint32_t khz, uint16_t antcap);
void si46xx_set_property(uint16_t property_id, uint16_t data);
void si46xx_fm_rsq_status(void);
void si46xx_fm_rds_status(void);
void si46xx_fm_rds_blockcount(void);

void si46xx_dab_set_freq_list(uint8_t num, uint32_t *freq_list);
void si46xx_dab_tune_freq(uint8_t index, uint8_t antcap);
void si46xx_dab_digrad_status(struct dab_digrad_status_t *status);
void si46xx_dab_digrad_status_print(struct dab_digrad_status_t *status);
int si46xx_dab_get_digital_service_list(void);
void si46xx_dab_get_service_linking_info(uint32_t service_id);
void si46xx_dab_start_digital_service(uint32_t service_id, uint32_t comp_id);
void si46xx_dab_print_service_list(void);
void si46xx_dab_start_digital_service_num(uint32_t num);
void si46xx_dab_get_ensemble_info(void);
void si46xx_dab_get_audio_info(void);
void si46xx_dab_get_subchannel_info(void);


void si46xx_dab_scan();

#endif

