#!/usr/bin/env bash
sudo ip addr add 10.0.0.1/24 dev enp5s0u3u4
sudo ip link set enp5s0u3u4 up
sudo dnsmasq --dhcp-range=10.0.0.2,10.0.0.254 --interface=enp5s0u3u4 --bind-interfaces --no-daemon 
