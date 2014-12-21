# dabpi_ctl - RPi fm/fmhd/dab receiver control interface

![Image of DABPi v1 PCB]
(https://github.com/elmo2k3/dabpi_ctl/blob/master/doc/pics/dabpi_v1.jpg)

## Warning

!!! Firmware files removed due to license issues !!!

This project is pre-alpha. Most features are still missing.

## Requirements

You need a RPi that has the option to mount the I2S extension header.
![Image of RPi with soldered I2S extension header]
(https://github.com/elmo2k3/dabpi_ctl/blob/master/doc/pics/IMG_0292.JPG)

Get the firmware package for Si4688 from Silabs.

Be sure to enable SPI in raspi-config.

Install wiringPi library. See http://wiringpi.com/download-and-install/

To get the audio you need to install a new kernel. It can be compiled from here
https://github.com/raspberrypi/linux or use the precompiled image. For the precompiled binarys, see the Install section.

## RPi A+/B+ instructions
![Image of RPi B+ with additional cables]
(https://github.com/elmo2k3/dabpi_ctl/blob/master/doc/pics/a_b_plus.jpg)

Connect P5 header pins 3,4,5 to GPIO header pins 12,35,38.

## Install

There is a very simple install script for a fresh raspian now (where there is nothing to be damaged).

```bash
wget https://github.com/elmo2k3/dabpi_ctl/raw/master/install_on_fresh_raspbian.sh
chmod a+x install_on_fresh_raspbian.sh
./install_on_fresh_raspbian.sh
```

### Manual Install

Get and compile the source code:

```bash
git clone https://github.com/elmo2k3/dabpi_ctl
cd dabpi_ctl
make
```
If you want to use the precompiled kernel, it can be found in the kernel/ directory.

```bash
sudo cp kernel/Image /boot/kernel.img
sudo rm -rf /lib/modules
sudo rm -rf /lib/firmware
sudo tar -C / -xvzf kernel/lib.tar.gz
```
Edit /etc/modules
```bash
bcm2708_dmaengine
snd-bcm2835
```
After a reboot load the module.

```bash
sudo modprobe snd-soc-rpi-dabpi
```
You should see this in the kernel log:
```bash
pi@raspberrypi ~/dabpi_ctl $ dmesg | tail -n1
[ 2435.067966] snd-dabpi snd-dabpi.0:  si468x-hifi <-> bcm2708-i2s.0 mapping ok
pi@raspberrypi ~/dabpi_ctl $
```

## Usage

Run dabpi_ctl:

```bash
[bjoern@x200 user_program]$ ./dabpi_ctl
usage: ./dabpi_ctl [-a|-b]
  -a             init DAB mode
  -b             init fm mode
  -c frequency   tune frequency in FM mode
  -d             fm status
  -e             dab status
  -f service     start service of dab service list
  -g             get dab service list
  -i channel     tune to channel in dab frequency list
  -j region      set frequency list
                    0   Baden-Wuertemberg
                    1   Bayern
                    2   Berlin-Brandenburg
                    3   Bremen
                    4   Hamburg
                    5   Hessen
                    6   Mecklenburg-Vorpommern
                    7   Niedersachsen
                    8   Nordrhein-Westfalen
                    9   Rheinland-Pfalz
                    10  Saarland
                    11  Sachsen
                    12  Sachsen-Anhalt
                    13  Schleswig-Holstein
                    14  Thueringen
  -k region      scan frequency list
  -l up|down     fm seek next station
  -m             fm rds status
  -n             dab get audio info
  -o             dab get subchannel info
  -h             this help
```

### Audio output

To test audio output, redirect it from capture to playback device:

```bash
alsaloop -C hw:1,0
```

### Examples

Enter fm mode:

```bash
pi@raspberrypi ~/dabpi_ctl $ sudo ./dabpi_ctl -b
si46xx_init_mode_fm()
si46xx_get_sys_state answer: 800000c001ff
si46xx_get_part_info answer: 800000c0000000005012000000005012000000000100
[...]
si46xx_fm_tune_freq(98500)
808000c0
pi@raspberrypi ~/dabpi_ctl $
```

Tune to radio station 106.700MHz:

```bash
pi@raspberrypi ~/dabpi_ctl $ sudo ./dabpi_ctl -c 106700
si46xx_fm_tune_freq(106700)
808000c0
pi@raspberrypi ~/dabpi_ctl $
```

Enter DAB mode and scan all channels in the list for NRW:

```bash
pi@raspberrypi ~/dabpi_ctl $ sudo ./dabpi_ctl -a -k 8
si46xx_init_mode_dab()
[...]
si46xx_dab_tune_freq(0): 818000c0
si46xx_dab_digrad_status(): Channel 0: ACQ: 1 RSSI: 45 SNR: -10 Name: DR Deutschland
si46xx_dab_tune_freq(1): 818000c0
si46xx_dab_digrad_status(): Channel 1: ACQ: 1 RSSI: 41 SNR: -10 Name: Radio fuer NRW
pi@raspberrypi ~/dabpi_ctl $
```

Select DAB channel:

```bash
pi@raspberrypi ~/dabpi_ctl $ sudo ./dabpi_ctl -i 1
si46xx_dab_tune_freq(0): 818000c0
pi@raspberrypi ~/dabpi_ctl $
```

Get ensemble information:

```bash
pi@raspberrypi ~/dabpi_ctl $ sudo ./dabpi_ctl -g
si46xx_dab_get_digital_service_list()
List size:     370
List version:  227
Services:      13
Num:  0  Service ID: e0d01005  Service Name: WDR-INFO          Component ID: 49162
                                                               Component ID: 49162
Num:  1  Service ID:     109d  Service Name: Domradio          Component ID: 12
                                                               Component ID: 0
Num:  2  Service ID: e0d01006  Service Name: WDR-TPEG          Component ID: 49163
                                                               Component ID: 0
Num:  3  Service ID:     d394  Service Name: WDR 4             Component ID: 14
                                                               Component ID: 0
Num:  4  Service ID:     df92  Service Name: VERA              Component ID: 8
                                                               Component ID: 0
Num:  5  Service ID:     d391  Service Name: 1LIVE             Component ID: 1
                                                               Component ID: 0
Num:  6  Service ID:     d395  Service Name: WDR 5             Component ID: 4
                                                               Component ID: 0
Num:  7  Service ID:     d496  Service Name: FUNKHAUS EUROPA   Component ID: 5
                                                               Component ID: 0
Num:  8  Service ID: e0d01004  Service Name: WDR-EPG           Component ID: 49161
                                                               Component ID: 0
Num:  9  Service ID:     df95  Service Name: KinderRadioKanal  Component ID: 2
                                                               Component ID: 0
Num: 10  Service ID:     df91  Service Name: 1LIVE diggi       Component ID: 3
                                                               Component ID: 0
Num: 11  Service ID:     1462  Service Name: RADIO IMPALA      Component ID: 13
                                                               Component ID: 0
Num: 12  Service ID:     d392  Service Name: WDR 2             Component ID: 7
                                                               Component ID: 0
pi@raspberrypi ~/dabpi_ctl $
```

Start one of the services in ensemble:

```bash
pi@raspberrypi ~/dabpi_ctl $ sudo ./dabpi_ctl -f 5
si46xx_dab_get_digital_service_list()
List size:     370
List version:  227
Services:      13
Num:  0  Service ID: e0d01005  Service Name: WDR-INFO          Component ID: 49162
                                                               Component ID: 49162
[...]
Starting service 1LIVE            d391 1
pi@raspberrypi ~/dabpi_ctl $
```
