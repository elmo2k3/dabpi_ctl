# dabpi_ctl - RPi fm/fmhd/dab receiver control interface

## Warning

This project is pre-alpha. Most features are still missing.

## Requirements

Be sure to enable SPI in raspi-config.

Install wiringPi library. See http://wiringpi.com/download-and-install/

To get the audio you need to install a new kernel. It can be compiled from here
https://github.com/raspberrypi/linux or use the precompiled image. For the precompiled binarys, see the Install section.


## Install

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
  -f num         start service num of dab service list
  -g             get dab service list
  -i             tune to num in dab frequency list
  -j num         set frequency list
                    0   NRW
                    1   Bayern
  -h             this help
```
### Examples

Enter fm mode:

```bash
pi@raspberrypi ~/dabpi_ctl $ sudo ./dabpi_ctl -b
si46xx_init_mode_fm()
si46xx_get_sys_state answer: 800000c001ff
si46xx_get_part_info answer: 800000c0000000005012000000005012000000000100
si46xx_set_property(2048,3)
800000c0
si46xx_set_property(12802,0)
800000c0
si46xx_set_property(12804,0)
800000c0
si46xx_set_property(13568,0)
800000c0
si46xx_set_property(5906,0)
800000c0
si46xx_set_property(514,4096)
800000c0
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
