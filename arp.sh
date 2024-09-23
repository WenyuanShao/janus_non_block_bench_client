#!/bin/bash

sudo ifconfig enp5s0 10.10.1.1
sudo arp -s 10.10.1.2 2c:94:64:07:99:02
