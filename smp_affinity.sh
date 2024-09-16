#!/bin/bash
# Set the CPU mask to use all 16 cores (0-15)
CPU_MASK="000000ff"  # 16 cores

# Loop through IRQ numbers 128 to 143
for IRQ in $(seq 132 139); do
    echo "Setting smp_affinity for IRQ $IRQ to $CPU_MASK"
    echo $CPU_MASK > /proc/irq/$IRQ/smp_affinity || echo "Failed to set smp_affinity for IRQ $IRQ"
done

echo "smp_affinity settings applied for IRQs 128 to 143."
