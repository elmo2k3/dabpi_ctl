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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __arm__
#include <wiringPi.h>
#include <wiringPiSPI.h>
#endif
#include "si46xx.h"
//#include "dab_radio_3_2_7.h"

#define msleep(x) usleep(x*1000)

#ifdef __arm__
#define CS_LOW() digitalWrite(10, LOW)
#define CS_HIGH() digitalWrite(10, HIGH)

#define RESET_LOW() digitalWrite(4, LOW)
#define RESET_HIGH() digitalWrite(4, HIGH)

#define SPI_Write(data,len) wiringPiSPIDataRW(0,data,len)
#else
#define CS_LOW()
#define CS_HIGH()
#define RESET_LOW()
#define RESET_HIGH()
#define SPI_Write(data,len)
#endif
//
//#define msleep(x) HAL_Delay(x)
//#define SPI_Write(data,len) HAL_SPI_Transmit(&hspi1,data,len,1000)
//#define CS_LOW() GPIOA->BSRRH = SI46XX_PIN_NSS
//#define CS_HIGH() GPIOA->BSRRL = SI46XX_PIN_NSS
//#define RESET_LOW() GPIOB->BSRRH = GPIO_PIN_8
//#define RESET_HIGH() GPIOB->BSRRL = GPIO_PIN_8
//
uint8_t dab_num_channels;

void print_hex_str(uint8_t *str, uint16_t len)
{
	uint16_t i;
	printf("0x");
	for(i=0;i<len;i++){
		printf("%02x",(int)str[i]);
	}
	printf("\r\n");
}

static void si46xx_write_host_load_data(uint8_t cmd,
		const uint8_t *data,
		uint16_t len)
{

	uint8_t zero_data[3];

	zero_data[0] = 0;
	zero_data[1] = 0;
	zero_data[2] = 0;
	CS_LOW();
	SPI_Write(&cmd,1);
	SPI_Write(zero_data,3);
	SPI_Write((uint8_t*)data,len);
	CS_HIGH();
}

static void si46xx_read(uint8_t *data, uint8_t cnt)
{
	uint8_t zero = 0;
	uint8_t timeout;

	timeout = 100; // wait for CTS
	while(--timeout){
		CS_HIGH();
		msleep(1); // make sure cs is high for 20us
		CS_LOW();
		zero = 0;
		SPI_Write(&zero,1); // read status register (we are working without interrupts)
		SPI_Write(data,cnt); // read data
		CS_HIGH();
		msleep(1); // make sure cs is high for 20us
		if(data[0] & 0x80)
			break;
	}
}

static void si46xx_write_data(uint8_t cmd,
		uint8_t *data,
		uint16_t len)
{
	uint8_t timeout;
	uint8_t buf[4];

	timeout = 100; // wait for CTS
	while(--timeout){
		si46xx_read(buf,4);
		if(buf[0] & 0x80)
			break;
	}

	CS_LOW();
	SPI_Write(&cmd,1);
	SPI_Write(data,len);
	CS_HIGH();
}

static uint16_t si46xx_read_dynamic(uint8_t *data)
{
	uint8_t zero = 0;
	uint16_t cnt;

	CS_HIGH();
	msleep(1); // make sure cs is high for 20us
	CS_LOW();
	SPI_Write(&zero,1);
	SPI_Write(data,6);
	cnt = ((uint16_t)data[5]<<8) | (uint16_t)data[4];
	if(cnt > 3000) cnt = 0;
	SPI_Write(&data[6],cnt);
	CS_HIGH();
	msleep(1); // make sure cs is high for 20us

	return cnt + 6;
}

static void si46xx_get_sys_state()
{
	uint8_t zero = 0;
	char buf[6];

	si46xx_write_data(SI46XX_GET_SYS_STATE,&zero,1);
	si46xx_read(buf,6);
	printf("si46xx_get_sys_state answer: ");
	print_hex_str(buf,6);
}

static void si46xx_get_part_info()
{
	uint8_t zero = 0;
	char buf[22];

	si46xx_write_data(SI46XX_GET_PART_INFO,&zero,1);
	si46xx_read(buf,22);
	printf("si46xx_get_part_info answer: ");
	print_hex_str(buf,22);
}

void si46xx_periodic()
{
	char buf[4];
	si46xx_read(buf,4);
}


void si46xx_dab_start_digital_service(uint32_t service_id,
		uint32_t comp_id)
{
	uint8_t data[11];
	char buf[5];

	data[0] = 0;
	data[1] = 0;
	data[2] = 0;
	data[3] = service_id & 0xFF;
	data[4] = (service_id >>8) & 0xFF;
	data[5] = (service_id >>16) & 0xFF;
	data[6] = (service_id >>24) & 0xFF;
	data[7] = comp_id & 0xFF;
	data[8] = (comp_id >> 8) & 0xFF;
	data[9] = (comp_id >> 16) & 0xFF;
	data[10] = (comp_id >> 24) & 0xFF;

	si46xx_write_data(SI46XX_DAB_START_DIGITAL_SERVICE,data,11);
	si46xx_read(buf,5);
}

static si46xx_swap_services(uint8_t first, uint8_t second)
{
	struct dab_service_t tmp;

	memcpy(&tmp,&dab_service_list.services[first],sizeof(tmp));
	memcpy(&dab_service_list.services[first],
			&dab_service_list.services[second] ,sizeof(tmp));
	memcpy(&dab_service_list.services[second],&tmp ,sizeof(tmp));
}

static si46xx_sort_service_list(void)
{
	uint8_t i,p,swapped;

	swapped = 0;
	for(i=dab_service_list.num_services;i>1;i--){
		for(p=0;p<i-1;p++){
			if(dab_service_list.services[p].service_id >
					dab_service_list.services[p+1].service_id){
				si46xx_swap_services(p,p+1);
				swapped = 1;
			}
		}
		if(!swapped)
			break;
	}
}

static void si46xx_dab_parse_service_list(uint8_t *data, uint16_t len)
{
	uint16_t remaining_bytes;
	uint16_t pos;
	uint8_t service_num;
	uint8_t component_num;
	uint8_t i;

	if(len<6)
		return; // no list available? exit
	if(len >= 9){
		dab_service_list.list_size = data[5]<<8 | data[4];
		dab_service_list.version = data[7]<<8 | data[6];
		dab_service_list.num_services = data[8];
	}
	// 9,10,11 are align pad
	pos = 12;
	if(len <= pos)
		return; // no services? exit

	remaining_bytes = len - pos;
	service_num = 0;
	// size of one service with zero component: 24 byte
	// every component + 4 byte
	while(service_num < dab_service_list.num_services){
		dab_service_list.services[service_num].service_id =
			data[pos+3]<<24 |
			data[pos+2]<<16 |
			data[pos+1]<<8 |
			data[pos];
		component_num = data[pos+5] & 0x0F;
		dab_service_list.services[service_num].num_components = component_num;
		memcpy(dab_service_list.services[service_num].service_label,
				&data[pos+8],16);
		dab_service_list.services[service_num].service_label[16] = '\0';
		for(i=0;i<component_num;i++){
			dab_service_list.services[service_num].component_id[i] =
				data[pos+25] << 8 |
				data[pos+24];
			pos += 4;
		}
		pos +=24;
		service_num++;
	}
	si46xx_sort_service_list();
}

void si46xx_dab_get_ensemble_info()
{
	char buf[22];
	char data;
	uint8_t timeout;
	char label[17];

	//data[0] = (1<<4) | (1<<0); // force_wb, low side injection
	data = 0;

	si46xx_write_data(SI46XX_DAB_GET_ENSEMBLE_INFO,&data,1);
	timeout = 10;
	while(--timeout){ // completed with CTS
		si46xx_read(buf,22);
		if(buf[0] & 0x80)
			break;
	}
	memcpy(label,&buf[6],16);
	label[16] = '\0';
	printf("Name: %s",label);
}

void si46xx_dab_print_service_list()
{
	uint8_t i,p;

	printf("List size:     %d\r\n",dab_service_list.list_size);
	printf("List version:  %d\r\n",dab_service_list.version);
	printf("Services:      %d\r\n",dab_service_list.num_services);

	for(i=0;i<dab_service_list.num_services;i++){
		printf("Num: %2u  Service ID: %8x  Service Name: %s  Component ID: %d\r\n",
				i,
				dab_service_list.services[i].service_id,
				dab_service_list.services[i].service_label,
				dab_service_list.services[i].component_id[0]
		      );
		for(p=0;p<dab_service_list.services[i].num_components;p++){
			printf("                                                               Component ID: %d\r\n",
					dab_service_list.services[i].component_id[i]
			      );
		}
	}
}

void si46xx_dab_start_digital_service_num(uint32_t num)
{
	printf("Starting service %s %x %x\r\n", dab_service_list.services[num].service_label,
			dab_service_list.services[num].service_id,
			dab_service_list.services[num].component_id[0]);
	si46xx_dab_start_digital_service(dab_service_list.services[num].service_id,
			dab_service_list.services[num].component_id[0]);
}

int si46xx_dab_get_digital_service_list()
{
	uint8_t zero = 0;
	uint16_t len;
	uint16_t timeout;
	char buf[2047+6];

	printf("si46xx_dab_get_digital_service_list()\r\n");
	timeout = 100;
	while(timeout--){
		si46xx_write_data(SI46XX_DAB_GET_DIGITAL_SERVICE_LIST,&zero,1);
		if((len = si46xx_read_dynamic(buf)) > 6)
			break;
	}
	si46xx_dab_parse_service_list(buf,len);
	return len;
}

void si46xx_dab_get_audio_info()
{
	uint8_t zero = 0;
	char buf[9];

	printf("si46xx_dab_get_audio_info()\r\n");
	si46xx_write_data(SI46XX_DAB_GET_AUDIO_INFO,&zero,1);
	si46xx_read(buf,9);
	print_hex_str(buf,9);
	printf("Bit rate: %dkbps\r\n",buf[4] + (buf[5]<<8));
	printf("Sample rate: %dHz\r\n",buf[6] + (buf[7]<<8));
	if((buf[8]& 0x03) == 0) {
		printf("Audio Mode = Dual Mono\r\n");
	}
	if((buf[8]& 0x03) == 1) {
		printf("Audio Mode = Mono\r\n");
	}
	if((buf[8]& 0x03) == 2) {
		printf("Audio Mode = Stereo\r\n");
	}
	if((buf[8]& 0x03) == 3) {
		printf("Audio Mode = Joint Stereo\r\n");
	}
	printf("SBR: %d\r\n", (buf[8] & 0x04) ? 1:0);
	printf("PS: %d\r\n", (buf[8] & 0x08) ? 1:0);
}

void si46xx_dab_get_subchannel_info()
{
	uint8_t zero = 0;
	char buf[12];
	printf("si46xx_dab_get_subchannel_info()\r\n");
	si46xx_write_data(SI46XX_DAB_GET_SUBCHAN_INFO,&zero,1);
	si46xx_read(buf,12);
	if(buf[4] == 0) {
		printf("Service Mode = Audio Stream Service\r\n");
	}
	if(buf[4] == 1) {
		printf("Service Mode = Data Stream Service\r\n");
	}
	if(buf[4] == 2) {
		printf("Service Mode = FIDC Service\r\n");
	}
	if(buf[4] == 3) {
		printf("Service Mode = MSC Data Packet Service\r\n");
	}
	if(buf[4] == 4) {
		printf("Service Mode = DAB+\r\n");
	}
	if(buf[4] == 5) {
		printf("Service Mode = DAB\r\n");
	}
	if(buf[4] == 6) {
		printf("Service Mode = FIC Service\r\n");
	}
	if(buf[4] == 7) {
		printf("Service Mode = XPAD Data\r\n");
	}
	if(buf[4] == 8) {
		printf("Service Mode = No Media\r\n");
	}
	if(buf[5] == 1) {
		printf("Protection Mode UEP-1\r\n");
	}
	if(buf[5] == 2) {
		printf("Protection Mode UEP-2\r\n");
	}
	if(buf[5] == 3) {
		printf("Protection Mode UEP-3\r\n");
	}
	if(buf[5] == 4) {
		printf("Protection Mode UEP-4\r\n");
	}
	if(buf[5] == 5) {
		printf("Protection Mode UEP-5\r\n");
	}
	if(buf[5] == 6) {
		printf("Protection Mode EEP-1A\r\n");
	}
	if(buf[5] == 7) {
		printf("Protection Mode EEP-2A\r\n");
	}
	if(buf[5] == 8) {
		printf("Protection Mode EEP-3A\r\n");
	}
	if(buf[5] == 9) {
		printf("Protection Mode EEP-4A\r\n");
	}
	if(buf[5] == 10) {
		printf("Protection Mode EEP-1B\r\n");
	}
	if(buf[5] == 11) {
		printf("Protection Mode EEP-2B\r\n");
	}
	if(buf[5] == 12) {
		printf("Protection Mode EEP-3B\r\n");
	}
	if(buf[5] == 13) {
		printf("Protection Mode EEP-4B\r\n");
	}
	printf("Subchannel Bitrate: %dkbps\r\n",buf[6] + (buf[7]<<8));
	printf("Capacity Units: %d CU\r\n",buf[8] + (buf[9]<<8));
	printf("CU Starting Adress: %d\r\n",buf[10] + (buf[11]<<8));
}


void si46xx_dab_set_freq_list(uint8_t num, uint32_t *freq_list)
{
	uint8_t data[3+4*48]; // max 48 frequencies
	uint8_t i;
	char buf[4];

	dab_num_channels = num;

	printf("si46xx_dab_set_freq_list(): ");
	if(num == 0 || num > 48){
		printf("num must be between 1 and 48\r\n");
		return;
	}

	data[0] = num; // NUM_FREQS 1-48
	data[1] = 0;
	data[2] = 0;

	for(i=0;i<num;i++){
		data[3+4*i] = freq_list[i] & 0xFF;
		data[4+4*i] = freq_list[i] >> 8;
		data[5+4*i] = freq_list[i] >> 16;
		data[6+4*i] = freq_list[i] >> 24;
	}
	si46xx_write_data(SI46XX_DAB_SET_FREQ_LIST,data,3+4*num);

	si46xx_read(buf,4);
	print_hex_str(buf,4);
}

void si46xx_dab_tune_freq(uint8_t index, uint8_t antcap)
{
	uint8_t data[5];
	char buf[4];
	uint8_t timeout;

	printf("si46xx_dab_tune_freq(%d): ",index);

	//data[0] = (1<<4) | (1<<0); // force_wb, low side injection
	data[0] = 0;
	data[1] = index;
	data[2] = 0;
	data[3] = antcap;
	data[4] = 0;

	si46xx_write_data(SI46XX_DAB_TUNE_FREQ,data,5);
	timeout = 20;
	while(--timeout){ // wait for tune to complete
		si46xx_read(buf,4);
		if(buf[0] & 0x01)
			break;
		msleep(100);
	}
	print_hex_str(buf,4);
}

void si46xx_fm_tune_freq(uint32_t khz, uint16_t antcap)
{
	uint8_t data[5];
	char buf[4];

	printf("si46xx_fm_tune_freq(%d)\r\n",khz);

	//data[0] = (1<<4) | (1<<0); // force_wb, low side injection
	//data[0] = (1<<4)| (1<<3); // force_wb, tune_mode=2
	data[0] = 0;
	data[1] = ((khz/10) & 0xFF);
	data[2] = ((khz/10) >> 8) & 0xFF;
	data[3] = antcap & 0xFF;
	data[4] = 0;
	si46xx_write_data(SI46XX_FM_TUNE_FREQ,data,5);

	si46xx_read(buf,4);
	print_hex_str(buf,4);
}

void si46xx_fm_seek_start(uint8_t up, uint8_t wrap)
{
	uint8_t data[5];
	char buf[4];

	printf("si46xx_fm_seek_start()\r\n");

	data[0] = 0;
	data[1] = (up&0x01)<<1 | (wrap&0x01);
	data[2] = 0;
	data[3] = 0;
	data[4] = 0;
	si46xx_write_data(SI46XX_FM_SEEK_START,data,5);

	si46xx_read(buf,4);
	print_hex_str(buf,4);
}

void si46xx_am_tune_freq(uint32_t khz, uint16_t antcap)
{
	uint8_t data[5];
	char buf[4];

	printf("si46xx_am_tune_freq(%d)\r\n",khz);

	//data[0] = (1<<4) | (1<<0); // force_wb, low side injection
	//data[0] = (1<<4)| (1<<3); // force_wb, tune_mode=2
	data[0] = 0;
	data[1] = ((khz) & 0xFF);
	data[2] = ((khz) >> 8) & 0xFF;
	data[3] = antcap & 0xFF;
	data[4] = 0;
	si46xx_write_data(SI46XX_AM_TUNE_FREQ,data,5);

	si46xx_read(buf,4);
	print_hex_str(buf,4);
}


static void si46xx_load_init()
{
	uint8_t data = 0;
	si46xx_write_data(SI46XX_LOAD_INIT,&data,1);
	msleep(4); // wait 4ms (datasheet)
}

static void store_image(const uint8_t *data, uint32_t len, uint8_t wait_for_int)
{
	uint32_t remaining_bytes = len;
	uint32_t count_to;
	char buf[4];

	si46xx_load_init();
	while(remaining_bytes){
		if(remaining_bytes >= 2048){
			count_to = 2048;
		}else{
			count_to = remaining_bytes;
		}

		si46xx_write_host_load_data(SI46XX_HOST_LOAD, data+(len-remaining_bytes), count_to);
		remaining_bytes -= count_to;
		msleep(1);
	}
	msleep(4); // wait 4ms (datasheet)
	si46xx_read(buf,4);
	msleep(4); // wait 4ms (datasheet)
}

static void store_image_from_file(char *filename, uint8_t wait_for_int)
{
	long remaining_bytes;
	long len;
	uint32_t count_to;
	FILE *fp;
	uint8_t buffer[2048];
	size_t result;
	char buf[4];

	fp = fopen(filename, "rb");
	if(fp == NULL){
		printf("file error %s\r\n",filename);
		return;
	}

	fseek(fp,0, SEEK_END);
	len = ftell(fp);
	remaining_bytes = len;
	rewind(fp);

	si46xx_load_init();
	while(remaining_bytes){
		if(remaining_bytes >= 2048){
			count_to = 2048;
		}else{
			count_to = remaining_bytes;
		}
		result = fread(buffer,1,count_to,fp);
		if(result != count_to){
			printf("file error %s\r\n",filename);
			return;
		}

		si46xx_write_host_load_data(SI46XX_HOST_LOAD, buffer, count_to);
		remaining_bytes -= count_to;
		msleep(1);
	}
	fclose(fp);
	msleep(4); // wait 4ms (datasheet)
	si46xx_read(buf,4);
	msleep(4); // wait 4ms (datasheet)
}

static void si46xx_powerup()
{
	uint8_t data[15];
	char buf[4];

	data[0] = 0x80; // ARG1
	data[1] = (1<<4) | (7<<0); // ARG2 CLK_MODE=0x1 TR_SIZE=0x7
	//data[2] = 0x28; // ARG3 IBIAS=0x28
	data[2] = 0x48; // ARG3 IBIAS=0x28
	data[3] = 0x00; // ARG4 XTAL
	data[4] = 0xF9; // ARG5 XTAL // F8
	data[5] = 0x24; // ARG6 XTAL
	data[6] = 0x01; // ARG7 XTAL 19.2MHz
	data[7] = 0x1F; // ARG8 CTUN
	data[8] = 0x00 | (1<<4); // ARG9
	data[9] = 0x00; // ARG10
	data[10] = 0x00; // ARG11
	data[11] = 0x00; // ARG12
	data[12] = 0x00; // ARG13 IBIAS_RUN
	data[13] = 0x00; // ARG14
	data[14] = 0x00; // ARG15

	si46xx_write_data(SI46XX_POWER_UP,data,15);
	msleep(1); // wait 20us after powerup (datasheet)
	si46xx_read(buf,4);
}

static void si46xx_boot()
{
	uint8_t data = 0;
	char buf[4];

	si46xx_write_data(SI46XX_BOOT,&data,1);
	msleep(300); // 63ms at analog fm, 198ms at DAB
	si46xx_read(buf,4);
}

void si46xx_fm_rsq_status()
{
	uint8_t data = 0;
	char buf[20];

	printf("si46xx_fm_rsq_status()\r\n");
	si46xx_write_data(SI46XX_FM_RSQ_STATUS,&data,1);
	si46xx_read(buf,20);
	print_hex_str(buf,20);
	printf("SNR: %d dB\r\n",(int8_t)buf[10]);
	printf("RSSI: %d dBuV\r\n",(int8_t)buf[9]);
	printf("Frequency: %dkHz\r\n",(buf[7]<<8 | buf[6])*10);
	printf("FREQOFF: %d\r\n",(int8_t)buf[8]*2);
	printf("READANTCAP: %d\r\n",(int8_t)(buf[12]+(buf[13]<<8)));
}

void si46xx_fm_rds_blockcount()
{
	//uint8_t data = 1; // clears block counts if set
	uint8_t data = 0; // clears block counts if set
	char buf[10];

	printf("si46xx_rds_blockcount()\r\n");
	si46xx_write_data(SI46XX_FM_RDS_BLOCKCOUNT,&data,1);
	si46xx_read(buf,10);
	printf("Expected: %d\r\n",buf[4] | (buf[5]<<8));
	printf("Received: %d\r\n",buf[6] | (buf[7]<<8));
	printf("Uncorrectable: %d\r\n",buf[8] | (buf[9]<<8));
}

static uint8_t si46xx_rds_parse(uint16_t *block)
{
	uint8_t addr;
	fm_rds_data.pi = block[0];
	if((block[1] & 0xF800) == 0x00){ // group 0A
		addr = block[1] & 0x03;
		fm_rds_data.ps_name[addr*2] = (block[3] & 0xFF00)>>8;
		fm_rds_data.ps_name[addr*2+1] = block[3] & 0xFF;
		fm_rds_data.group_0a_flags |= (1<<addr);
	}else if((block[1] & 0xF800)>>11 == 0x04){ // group 2A
		addr = block[1] & 0x0F;
		if((block[1] & 0x10) == 0x00){ // parse only string A
			fm_rds_data.radiotext[addr*4] = (block[2] & 0xFF00)>>8;
			fm_rds_data.radiotext[addr*4+1] = (block[2] & 0xFF);
			fm_rds_data.radiotext[addr*4+2] = (block[3] & 0xFF00)>>8;
			fm_rds_data.radiotext[addr*4+3] = (block[3] & 0xFF);

			if(fm_rds_data.radiotext[addr*4] == '\r'){
				fm_rds_data.radiotext[addr*4] = 0;
				fm_rds_data.group_2a_flags = 0xFFFF;
			}
			if(fm_rds_data.radiotext[addr*4+1] == '\r'){
				fm_rds_data.radiotext[addr*4+1] = 0;
				fm_rds_data.group_2a_flags = 0xFFFF;
			}
			if(fm_rds_data.radiotext[addr*4+2] == '\r'){
				fm_rds_data.radiotext[addr*4+2] = 0;
				fm_rds_data.group_2a_flags = 0xFFFF;
			}
			if(fm_rds_data.radiotext[addr*4+3] == '\r'){
				fm_rds_data.radiotext[addr*4+3] = 0;
				fm_rds_data.group_2a_flags = 0xFFFF;
			}
			fm_rds_data.group_2a_flags |= (1<<addr);
		}
	}
	if(fm_rds_data.group_0a_flags == 0x0F &&
			fm_rds_data.group_2a_flags == 0xFFFF){
		fm_rds_data.ps_name[8] = 0;
		fm_rds_data.radiotext[128] = 0;
		return 1;
	}
	return 0;
}


void si46xx_fm_rds_status()
{
	uint8_t data = 0;
	char buf[20];
	uint16_t timeout;
	uint16_t blocks[4];

	printf("si46xx_rds_status()\r\n");
	timeout = 5000; // work on 1000 rds blocks max
	while(--timeout){
		data = 1;
		si46xx_write_data(SI46XX_FM_RDS_STATUS,&data,1);
		si46xx_read(buf,20);
		blocks[0] = buf[12] + (buf[13]<<8);
		blocks[1] = buf[14] + (buf[15]<<8);
		blocks[2] = buf[16] + (buf[17]<<8);
		blocks[3] = buf[18] + (buf[19]<<8);
		fm_rds_data.sync = (buf[5] & 0x02)?1:0;
		if(!fm_rds_data.sync)
			break;
		if(si46xx_rds_parse(blocks))
			break;
		if(fm_rds_data.group_0a_flags == 0x0F) // stop at ps_name complete
			break;
	}
	if(!timeout)
		printf("Timeout\r\n");
	printf("RDSSYNC: %u\r\n",(buf[5]&0x02)?1:0);
	printf("PI: %d  Name:%s\r\nRadiotext: %s\r\n",
			fm_rds_data.pi,
			fm_rds_data.ps_name,
			fm_rds_data.radiotext);
}

void si46xx_dab_get_service_linking_info(uint32_t service_id)
{
	uint8_t data[7];
	char buf[24];

	printf("si46xx_dab_get_service_linking_info()\r\n");
	data[0] = 0;
	data[1] = 0;
	data[2] = 0;
	data[3] = (service_id) & 0xFF;
	data[4] = (service_id>>8) & 0xFF;
	data[5] = (service_id>>16) & 0xFF;
	data[6] = (service_id>>24) & 0xFF;
	si46xx_write_data(SI46XX_DAB_GET_SERVICE_LINKING_INFO,data,7);
	si46xx_read(buf,24);
}

void si46xx_dab_digrad_status_print(struct dab_digrad_status_t *status)
{
	printf("ACQ: %d\r\n",status->acq);
	printf("VALID: %d\r\n",status->valid);
	printf("RSSI: %d\r\n",status->rssi);
	printf("SNR: %d\r\n",status->snr);
	printf("FIC_QUALITY: %d\r\n",status->fic_quality);
	printf("CNR %d\r\n",status->cnr);
	printf("FFT_OFFSET %d\r\n",status->fft_offset);
	printf("Tuned frequency %dkHz\r\n",status->frequency);
	printf("Tuned index %d\r\n",status->tuned_index);

	printf("ANTCAP: %d\r\n",status->read_ant_cap);
}

void si46xx_dab_digrad_status(struct dab_digrad_status_t *status)
{
	uint8_t data = (1<<3) | 1; // set digrad_ack and stc_ack
	char buf[22];
	uint8_t timeout = 100;

	printf("si46xx_dab_digrad_status():\r\n");
	timeout = 10;
	while(--timeout){
		data = (1<<3) | 1; // set digrad_ack and stc_ack
		si46xx_write_data(SI46XX_DAB_DIGRAD_STATUS,&data,1);
		si46xx_read(buf,22);
		if(buf[0] & 0x81)
			break;
	}
	if(!timeout){
		printf("si46xx_dab_digrad_status() timeout reached\r\n");
		return;
	}
	if(!status)
		return;

	status->acq = (buf[5] & 0x04) ? 1:0;
	status->valid = buf[5] & 0x01;
	status->rssi = (int8_t)buf[6];
	status->snr = (int8_t)buf[7];
	status->fic_quality = buf[8];
	status->cnr = buf[9];
	status->fft_offset = (int8_t)buf[17];
	status->frequency = buf[12] |
		buf[13]<<8 |
		buf[14]<<16 |
		buf[15]<<24;
	status->tuned_index = buf[16];
	status->read_ant_cap = buf[18] | buf[19]<<8;

}

void si46xx_dab_scan()
{
	uint8_t i;
	struct dab_digrad_status_t status;

	for(i=0;i<dab_num_channels;i++){
		si46xx_dab_tune_freq(i,0);
		si46xx_dab_digrad_status(&status);
		printf("Channel %d: ACQ: %d RSSI: %d SNR: %d ", i,
				status.acq,
				status.rssi,
				status.snr);
		if(status.acq){
			msleep(1000);
			si46xx_dab_get_ensemble_info();
		};
		printf("\r\n");
	}
}

void si46xx_set_property(uint16_t property_id, uint16_t value)
{
	uint8_t data[5];
	char buf[4];

	printf("si46xx_set_property(0x%02X,0x%02X)\r\n",property_id,value);
	
	data[0] = 0;
	data[1] = property_id & 0xFF;
	data[2] = (property_id >> 8) & 0xFF;
	data[3] = value & 0xFF;
	data[4] = (value >> 8) & 0xFF;
	si46xx_write_data(SI46XX_SET_PROPERTY,data,5);
	si46xx_read(buf,4);
	print_hex_str(buf,4);
}

void si46xx_init()
{
	uint8_t read_data[30];
#ifdef __arm__
	wiringPiSetup();
	pinMode(4, OUTPUT);
	pinMode(10, OUTPUT);
	CS_HIGH();

	if(!wiringPiSPISetup(0,10000000) == -1){ // 10MHz SPI
		printf("setup SPI error\r\n");
	}
#endif

}

void si46xx_init_fm()
{
	uint8_t read_data[30];
	printf("si46xx_init_mode_fm()\r\n");
	/* reset si46xx  */
	RESET_LOW();
	msleep(10);
	RESET_HIGH();
	msleep(10);
	si46xx_powerup(read_data);
	store_image_from_file("firmware/rom00_patch.016.bin",0);

	//store_image(rom00_patch_016_bin,rom00_patch_016_bin_len,0);
	store_image_from_file("firmware/fmhd_radio_3_0_19.bif",0);
	//store_image(fmhd_radio_3_0_19_bif,fmhd_radio_3_0_19_bif_len,0);
	si46xx_boot(read_data);
	si46xx_get_sys_state(read_data);
	si46xx_get_part_info(read_data);
	//CDC_TxString("si46xx_init() done\r\n");
}

void si46xx_init_am()
{
	uint8_t read_data[30];
	printf("si46xx_init_mode_am()\r\n");
	/* reset si46xx  */
	RESET_LOW();
	msleep(10);
	RESET_HIGH();
	msleep(10);
	si46xx_powerup(read_data);
	store_image_from_file("firmware/rom00_patch.016.bin",0);
	store_image_from_file("firmware/amhd_radio_1_0_5.bif",0);
	si46xx_boot(read_data);
	si46xx_get_sys_state(read_data);
	si46xx_get_part_info(read_data);
	printf("si46xx_init() done\r\n");
}

void si46xx_init_dab()
{
	uint8_t read_data[30];
	printf("si46xx_init_mode_dab()\r\n");
	/* reset si46xx  */
	RESET_LOW();
	msleep(10);
	RESET_HIGH();
	msleep(10);
	si46xx_powerup(read_data);
	store_image_from_file("firmware/rom00_patch.016.bin",0);
	//store_image(rom00_patch_016_bin,rom00_patch_016_bin_len,0);
	//
	store_image_from_file("firmware/dab_radio_3_2_7.bif",0);
	//store_image(dab_radio_3_2_7_bif,dab_radio_3_2_7_bif_len,0);
	si46xx_boot(read_data);
	si46xx_get_sys_state(read_data);
	si46xx_get_part_info(read_data);
	printf("si46xx_init() done\r\n");
}
