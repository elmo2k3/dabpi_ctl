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
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include "si46xx.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

void tune_fm(void)
{
	si46xx_init_fm();
	si46xx_set_property(SI46XX_PIN_CONFIG_ENABLE,0x0003);
	si46xx_set_property(SI46XX_FM_VALID_RSSI_THRESHOLD,0x0000);
	si46xx_set_property(SI46XX_FM_VALID_SNR_THRESHOLD,0x0000);
	si46xx_set_property(SI46XX_FM_SOFTMUTE_SNR_LIMITS,0x0000);
	si46xx_set_property(SI46XX_FM_TUNE_FE_CFG,0x0000); // switch open
	si46xx_set_property(SI46XX_DIGITAL_IO_OUTPUT_FORMAT,0x1000); // SAMPL_SIZE = 16
	si46xx_fm_tune_freq(98500,0);
}

uint32_t frequency_list_nrw[] = {	CHAN_5C,
					CHAN_11D};
uint32_t frequency_list_bayern[] = {	CHAN_5C,
					CHAN_12D,
					CHAN_11D,
					CHAN_9C,
					CHAN_10C,
					CHAN_11A,
					CHAN_11C,
					CHAN_12A,
					CHAN_6A};

void tune_dab(void)
{
	si46xx_init_dab();
	si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_nrw),frequency_list_nrw);
	si46xx_set_property(SI46XX_DAB_CTRL_DAB_MUTE_SIGNAL_LEVEL_THRESHOLD,0);
	si46xx_set_property(SI46XX_DAB_CTRL_DAB_MUTE_SIGLOW_THRESHOLD,0);
	si46xx_set_property(SI46XX_DAB_CTRL_DAB_MUTE_ENABLE,0);
	si46xx_set_property(SI46XX_DIGITAL_SERVICE_INT_SOURCE,1);
	si46xx_set_property(SI46XX_DAB_TUNE_FE_CFG,0x0001); // switch closed
	si46xx_set_property(SI46XX_DAB_TUNE_FE_VARM,10);
	si46xx_set_property(SI46XX_DAB_TUNE_FE_VARB,10);
	si46xx_set_property(SI46XX_PIN_CONFIG_ENABLE,0x0003);
	si46xx_dab_tune_freq(0,0);
}

//static struct global_args_t{
//	int dab;
//	int fm;
//	int dab_start_service;
//	int frequency;
//	int dab_service_list;
//} global_args;
//
//static struct option long_options[]=
//{
//	{"dab_start_service", required_argument, &global_args.dab_start_service,0},
//	{"frequency", required_argument,&global_args.frequency,0},
//	{"status", no_argument,0,'s'},
//	{"dab", no_argument,&global_args.dab, 'd'},
//	{"fm", no_argument,&global_args.fm, 'f'},
//	{"dab_service_list", no_argument, &global_args.dab_service_list,0},
//	{0,0,0,0}
//};
//

void show_help(char *prog_name)
{
	printf("usage: %s [-a|-b]\n",prog_name);
	printf("  -a             init DAB mode\n");
	printf("  -b             init fm mode\n");
	printf("  -c frequency   tune frequency in FM mode\n");
	printf("  -d             fm status\n");
	printf("  -e             dab status\n");
	printf("  -f num         start service num of dab service list\n");
	printf("  -g             get dab service list\n");
	printf("  -i             tune to num in dab frequency list\n");
	printf("  -j num         set frequency list\n");
	printf("                    0   NRW\r\n");
	printf("                    1   Bayern\r\n");
	printf("  -h             this help\n");
}

int main(int argc, char **argv)
{

	int c;
	int frequency;
	int tmp;

	si46xx_init();
	while((c=getopt(argc, argv, "abc:def:ghi:j:")) != -1){
		switch(c){
		case 'a':
			tune_dab();
			break;
		case 'b':
			tune_fm();
			break;
		case 'c':
			frequency = atoi(optarg);
			si46xx_fm_tune_freq(frequency,0);
			break;
		case 'd':
			si46xx_fm_rsq_status();
			break;
		case 'e':
			si46xx_dab_digrad_status();
			break;
		case 'f':
			si46xx_dab_get_digital_service_list();
			si46xx_dab_print_service_list();
			si46xx_dab_start_digital_service_num(atoi(optarg));
			break;
		case 'g':
			si46xx_dab_get_digital_service_list();
			si46xx_dab_print_service_list();
			break;
		case 'h':
			show_help(argv[0]);
			break;
		case 'i':
			si46xx_dab_tune_freq(atoi(optarg),0);
			break;
		case 'j':
			tmp = atoi(optarg);
			if(tmp == 0){
				si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_nrw),
							frequency_list_nrw);
			}else if(tmp == 1){
				si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_bayern),
							frequency_list_bayern);
			}
			break;
		default:
			show_help(argv[0]);
			break;
		}
	}
	if(argc == 1){
		show_help(argv[0]);
	}
	return 0;
}

