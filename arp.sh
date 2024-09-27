#!/bin/bash

sudo ifconfig enp1s0f0 10.10.1.2
sudo arp -s 10.10.1.1 80:61:5f:06:0f:a3
