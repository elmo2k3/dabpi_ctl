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
#include "version.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

uint32_t frequency_list_nrw[] = {	CHAN_5C,
					CHAN_11D};
uint32_t frequency_list_by[] = {	CHAN_5C,
					CHAN_12D,
					CHAN_11D,
					CHAN_9C,
					CHAN_10C,
					CHAN_11A,
					CHAN_11C,
					CHAN_12A,
					CHAN_6A};
uint32_t frequency_list_bw[] = {	CHAN_5C,
					CHAN_8D,
					CHAN_9D,
					CHAN_11B};
uint32_t frequency_list_bb[] = {	CHAN_5C,
					CHAN_7B,
					CHAN_7D};
uint32_t frequency_list_hb[] = {	CHAN_5C,
					CHAN_7B};
uint32_t frequency_list_hh[] = {	CHAN_5C,
					CHAN_7A};
uint32_t frequency_list_he[] = {	CHAN_5C,
					CHAN_7B,
					CHAN_11C};
uint32_t frequency_list_mv[] = {	CHAN_5C,
					CHAN_12B};
uint32_t frequency_list_ni[] = {	CHAN_5C,
					CHAN_6A,
					CHAN_6D,
					CHAN_11B,
					CHAN_12A};
uint32_t frequency_list_rp[] = {	CHAN_5C,
					CHAN_11A};
uint32_t frequency_list_sl[] = {	CHAN_5C,
					CHAN_9A};
uint32_t frequency_list_sn[] = {	CHAN_5C,
					CHAN_6C,
					CHAN_8D,
					CHAN_9C,
					CHAN_12A};
uint32_t frequency_list_st[] = {	CHAN_5C,
					CHAN_11C,
					CHAN_12C};
uint32_t frequency_list_sh[] = {	CHAN_5C,
					CHAN_9C};
uint32_t frequency_list_th[] = {	CHAN_5C,
					CHAN_7B,
					CHAN_9C,
					CHAN_12B};
uint32_t frequency_list_it_sue[] = {	CHAN_10B,
					CHAN_10C,
					CHAN_10D,
					CHAN_12A,
					CHAN_12B,
					CHAN_12C};

void init_fm(void)
{
	si46xx_init_fm();
	si46xx_set_property(SI46XX_PIN_CONFIG_ENABLE,0x0003); // enable I2S output
	//si46xx_set_property(SI46XX_FM_VALID_RSSI_THRESHOLD,0x0000);
	//si46xx_set_property(SI46XX_FM_VALID_SNR_THRESHOLD,0x0000);
	si46xx_set_property(SI46XX_FM_SOFTMUTE_SNR_LIMITS,0x0000); // set the SNR limits for soft mute attenuation
	si46xx_set_property(SI46XX_FM_TUNE_FE_CFG,0x0000); // front end switch open
	si46xx_set_property(SI46XX_DIGITAL_IO_OUTPUT_FORMAT,0x1000); // SAMPL_SIZE = 16, I2S mode
	si46xx_set_property(SI46XX_FM_RDS_CONFIG, 0x0001); // enable RDS
	si46xx_set_property(SI46XX_FM_AUDIO_DE_EMPHASIS, SI46XX_AUDIO_DE_EMPHASIS_EU); // set de-emphasis for Europe
	si46xx_fm_tune_freq(105500,0);
}
void init_dab(void)
{
	si46xx_init_dab();
	si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_nrw),frequency_list_nrw);
	si46xx_set_property(SI46XX_DAB_CTRL_DAB_MUTE_SIGNAL_LEVEL_THRESHOLD,0);
	si46xx_set_property(SI46XX_DAB_CTRL_DAB_MUTE_SIGLOW_THRESHOLD,0);
	si46xx_set_property(SI46XX_DAB_CTRL_DAB_MUTE_ENABLE,0);
	si46xx_set_property(SI46XX_DIGITAL_SERVICE_INT_SOURCE,1); // enable DSRVPAKTINT interrupt ??
	si46xx_set_property(SI46XX_DAB_TUNE_FE_CFG,0x0001); // front end switch closed
	si46xx_set_property(SI46XX_DAB_TUNE_FE_VARM,0x1710); // Front End Varactor configuration (Changed from '10' to 0x1710 to improve receiver sensitivity - Bjoern 27.11.14)
	si46xx_set_property(SI46XX_DAB_TUNE_FE_VARB,0x1711); // Front End Varactor configuration (Changed from '10' to 0x1711 to improve receiver sensitivity - Bjoern 27.11.14)
	si46xx_set_property(SI46XX_PIN_CONFIG_ENABLE,0x0003); // enable I2S output
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
	printf("  -f service     start service of dab service list\n");
	printf("  -g             get dab service list\n");
	printf("  -i channel     tune to channel in dab frequency list\n");
	printf("  -j region      set frequency list\n");
	printf("                    0   Baden-Wuertemberg\r\n");
	printf("                    1   Bayern\r\n");
	printf("                    2   Berlin-Brandenburg\r\n");
	printf("                    3   Bremen\r\n");
	printf("                    4   Hamburg\r\n");
	printf("                    5   Hessen\r\n");
	printf("                    6   Mecklenburg-Vorpommern\r\n");
	printf("                    7   Niedersachsen\r\n");
	printf("                    8   Nordrhein-Westfalen\r\n");
	printf("                    9   Rheinland-Pfalz\r\n");
	printf("                    10  Saarland\r\n");
	printf("                    11  Sachsen\r\n");
	printf("                    12  Sachsen-Anhalt\r\n");
	printf("                    13  Schleswig-Holstein\r\n");
	printf("                    14  Thueringen\r\n");
	printf("                    15  Suedtirol (Italien)\r\n");
	printf("  -k region      scan frequency list\r\n");
	printf("  -l up|down     fm seek next station\r\n");
	printf("  -m             fm rds status\r\n");
	printf("  -n             dab get audio info\r\n");
	printf("  -o             dab get subchannel info\r\n");
	printf("  -s             get sys state (fm,dab,am...)\r\n");
	printf("  -h             this help\n");
}

void load_regional_channel_list(uint8_t tmp)
{
	if(tmp == 0){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_bw),
					frequency_list_bw);
	}else if(tmp == 1){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_by),
					frequency_list_by);
	}else if(tmp == 2){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_bb),
					frequency_list_bb);
	}else if(tmp == 3){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_hb),
					frequency_list_hb);
	}else if(tmp == 4){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_hh),
					frequency_list_hh);
	}else if(tmp == 5){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_he),
					frequency_list_he);
	}else if(tmp == 6){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_mv),
					frequency_list_mv);
	}else if(tmp == 7){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_ni),
					frequency_list_ni);
	}else if(tmp == 8){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_nrw),
					frequency_list_nrw);
	}else if(tmp == 9){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_rp),
					frequency_list_rp);
	}else if(tmp == 10){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_sl),
					frequency_list_sl);
	}else if(tmp == 11){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_sn),
					frequency_list_sn);
	}else if(tmp == 12){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_st),
					frequency_list_st);
	}else if(tmp == 13){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_sh),
					frequency_list_sh);
	}else if(tmp == 14){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_th),
					frequency_list_th);
	}else if(tmp == 15){
		si46xx_dab_set_freq_list(ARRAY_SIZE(frequency_list_it_sue),
					frequency_list_it_sue);
	}else{
		printf("Region %d not implemented\r\n",tmp);
	}
}

int main(int argc, char **argv)
{

	int c;
	int frequency;
	int tmp;
	struct dab_digrad_status_t dab_digrad_status;

	printf("dabpi_ctl version %s\r\n",GIT_VERSION);

	si46xx_init();
	while((c=getopt(argc, argv, "abc:def:ghi:j:k:l:mnos")) != -1){
		switch(c){
		case 'a':
			init_dab();
			break;
		case 'b':
			init_fm();
			break;
		case 'c':
			frequency = atoi(optarg);
			si46xx_fm_tune_freq(frequency,0);
			break;
		case 'd':
			si46xx_fm_rsq_status();
			break;
		case 'e':
			si46xx_dab_digrad_status(&dab_digrad_status);
			si46xx_dab_digrad_status_print(&dab_digrad_status);
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
			load_regional_channel_list(tmp);
			break;
		case 'k':
			tmp = atoi(optarg);
			load_regional_channel_list(tmp);
			si46xx_dab_scan();
			break;
		case 'l':
			if(strcmp(optarg,"down") == 0){
				si46xx_fm_seek_start(0,1);
			}else{
				si46xx_fm_seek_start(1,1);
			}
			break;
		case 'm':
			si46xx_fm_rds_status();
			si46xx_fm_rds_blockcount();
			break;
		case 'n':
			si46xx_dab_get_audio_info();
			break;
		case 'o':
			si46xx_dab_get_subchannel_info();
			break;
		case 's':
			si46xx_get_sys_state();
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

