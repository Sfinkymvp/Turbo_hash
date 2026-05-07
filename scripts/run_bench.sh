#! /usr/bin/env bash
set -e 

LOOKUP_ITERATIONS=1000000
SAMPLE_COUNT=30
CORE_ID=3
MONITOR_INTERVAL=0.25

DATA_FILE="data/word_forms.txt"
RESULT_DIR="reports"
SCRIPT_DIR="scripts"
IMAGE_DIR="images"
BINARY_DIR="bin"

declare -A binaries
binaries["DEFAULT"]="bench_default"
binaries["LEVEL0"]="bench_level0"
binaries["LEVEL1"]="bench_level1"
binaries["LEVEL2"]="bench_level2"

levels_order=("DEFAULT" "LEVEL0" "LEVEL1" "LEVEL2")

set_vars() {
    echo "--- Setting up system variables ---"

    echo -n "boost: " && echo 0 | sudo tee /sys/devices/system/cpu/cpufreq/boost
}

clear_vars() {
    trap - EXIT INT TERM 

    echo "--- Restoring system variables ---"

    echo -n "boost: " && echo 1 | sudo tee /sys/devices/system/cpu/cpufreq/boost
    
    if [[ -n "${monitor_pid:-}" ]]; then
        echo "--- Stopping monitor ---"
        sudo kill "$monitor_pid" || true
    fi
}

# Ловушка, которая позволяет вернуть программу в исходное состояние при завершении
trap clear_vars EXIT INT TERM

set_vars

# Пересборка проекта
echo "--- Rebuilding the project ---"

make clean
for level in "${levels_order[@]}"; do
    make DEBUG=OFF OPTI="$level" || {
        echo "ERROR: compilation failed!"
        exit 1
    }
done

# Остывание процессора
echo "--- Cooling down after rebuilding ---"

sleep 10

# Стадия тестирования
echo "--- Running the benchmark ---"

mkdir -p "$RESULT_DIR"

for level in "${levels_order[@]}"; do
    binary_name="${binaries[$level]}"

    echo -e "\t- Running the file $binary_name with opti=$level"

    taskset -c 0 ./scripts/monitor.sh "$CORE_ID" "$RESULT_DIR/monitor_$level.csv" "$MONITOR_INTERVAL" &
    monitor_pid=$!

    taskset -c "$CORE_ID" "$BINARY_DIR/$binary_name" \
        -f "$DATA_FILE" \
        -i "$LOOKUP_ITERATIONS" \
        -s "$SAMPLE_COUNT" > "$RESULT_DIR/raw_$level.txt"

    sudo kill "$monitor_pid"
    unset monitor_pid

    echo -e "\t- Cooling down after startup"

    # sleep 10
done

clear_vars

echo "--- Image creation ---"

mkdir -p "$IMAGE_DIR"

for level in "${levels_order[@]}"; do
    csv_file="$RESULT_DIR/monitor_$level.csv"
    output_file="$IMAGE_DIR/monitor_$level.png"
    python3 "$SCRIPT_DIR/plot_monitor.py" "$csv_file" "$output_file"
done

output_file="$IMAGE_DIR/compare_all.png"
python3 "$SCRIPT_DIR/compare_levels.py" "$LOOKUP_ITERATIONS" "$output_file"

echo "--- Finish! ---"
