#!/bin/bash

cd
sudo apt-get update
sudo apt-get --force-yes install git-core
git clone git://git.drogon.net/wiringPi
cd wiringPi
git pull origin
./build
cd
git clone https://github.com/elmo2k3/dabpi_ctl.git
cd dabpi_ctl
make
sudo cp kernel/Image /boot/kernel.img
sudo rm -rf /lib/modules
sudo rm -rf /lib/firmware
sudo tar -C / -xvzf kernel/lib.tar.gz
echo "snd-bcm2835" > /etc/modules
echo "bcm2708-dmaengine" >> /etc/modules
echo "snd-soc-rpi-dabpi" >> /etc/modules
