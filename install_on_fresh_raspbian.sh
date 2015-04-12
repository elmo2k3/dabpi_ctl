#!/bin/bash

# 
# install_on_fresh_raspbian.sh
# 
# Automatic install script for "dabpi_ctl - raspberry pi fm/fmhd/dab receiver board"
# Copyright (C) 2014  Patrick De Zordo <patrick@spamreducer.eu>
#                     Bjoern Biesenbach <bjoern@bjoern-b.de>
#
# Version: 0.1.1-beta
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

cd
echo "System upgrade.."
sudo apt-get -y update
sudo apt-get -y upgrade

echo "Base system Setup.."
sudo apt-get -y install make git-core
sudo apt-get -y install git-core

echo "Clone WiringPI GIT repo.."
git clone git://git.drogon.net/wiringPi

echo "Build and install WiringPI.."
cd wiringPi
git pull origin
./build

echo "Clone DABPi-Ctl GIT repo.."
cd
git clone https://github.com/elmo2k3/dabpi_ctl.git

echo "Build and install DABPi-Ctl.."
cd dabpi_ctl
make

echo "Copy precompiled DABPi-Ctl Kernel to system.."
sudo cp kernel/Image /boot/kernel.img
sudo cp kernel/Image /boot/kernel7.img

echo "Remove old modules.."
sudo rm -rf /lib/modules
sudo rm -rf /lib/firmware

echo "Extract new modules.."
sudo tar -C / -xvzf kernel/lib.tar.gz

echo "Edit Raspberry drivers blacklist to allow SPI and I2C.."
cp /etc/modprobe.d/raspi-blacklist.conf ~
cat << EOF | sudo tee /etc/modprobe.d/raspi-blacklist.conf
# blacklist spi and i2c by default (many users don't need them)

#blacklist spi-bcm2708
#blacklist i2c-bcm2708

EOF

echo "Setup modules to load on boot.."
cp /etc/modules ~
cat << EOF | sudo tee /etc/modules
# /etc/modules: kernel modules to load at boot time.
#
# This file contains the names of kernel modules that should be loaded
# at boot time, one per line. Lines beginning with "#" are ignored.
# Parameters can be specified after the module name.

bcm2708-dmaengine
snd-bcm2835
snd-soc-rpi-dabpi

EOF

echo "============================================"
echo "ALL DONE - Happy Radio listening with DABPi!"
echo "============================================"
