#!/bin/bash

# Network interface
INTERFACE="enp1s0f0"

# Set the CPU mask to use all 16 cores (0-15)
CPU_MASK="ff"  # 16 cores

# Find the number of receive queues for the interface
NUM_QUEUES=$(ls -d /sys/class/net/$INTERFACE/queues/rx-* | wc -l)

# Set RPS for each receive queue
for QUEUE in $(seq 0 $(($NUM_QUEUES - 1))); do
    echo "Setting RPS for $INTERFACE queue rx-$QUEUE to $CPU_MASK"
    echo $CPU_MASK > /sys/class/net/$INTERFACE/queues/rx-$QUEUE/rps_cpus || echo "Failed to set RPS for $INTERFACE queue rx-$QUEUE"
done

# Set the number of flow entries
echo 32768 > /proc/sys/net/core/rps_sock_flow_entries

echo "RPS settings applied for $INTERFACE."
