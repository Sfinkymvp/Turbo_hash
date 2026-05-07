#! /usr/bin/env bash

CORE="$1"
LOG_FILE="$2"
INTERVAL="$3"

echo "timestamp,freq_mhz,temp_c" > "$LOG_FILE"

while true; do
    freq=$(cat "/sys/devices/system/cpu/cpu$CORE/cpufreq/scaling_cur_freq")
    temp=$(cat "/sys/class/thermal/thermal_zone0/temp")
    timestamp=$(date +%s.%N)

    echo "$timestamp,$((freq / 1000)),$((temp / 1000))" >> "$LOG_FILE"
    sleep "$INTERVAL"
done
