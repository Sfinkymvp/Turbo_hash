#! /usr/bin/env bash
set -e 

# --- Изменяемые параметры запуска ---

LOOKUP_ITERATIONS=1000000
SAMPLE_COUNT=30
CORE_ID=3
MONITOR_INTERVAL=0.25

# ------------------------------------

DATA_FILE="data/word_forms.txt"
RESULT_DIR="results"
SCRIPT_DIR="scripts"
IMAGE_DIR="images"
BINARY_DIR="bin"

declare -A opti_bin
opti_bin["DEFAULT"]="bench_default"
opti_bin["LEVEL0"]="bench_level0"
opti_bin["LEVEL1"]="bench_level1"
opti_bin["LEVEL2"]="bench_level2"

levels_order=("DEFAULT" "LEVEL0" "LEVEL1" "LEVEL2")

declare -A list_bin
list_bin["ARRAY"]="bench_intr_icmp_default"
list_bin["CF"]="bench_intr_icmp_cf"
list_bin["STD"]="bench_intr_icmp_std"
list_bin["CLASSIC"]="bench_intr_icmp_classic"

structures=("ARRAY" "CF" "STD" "CLASSIC")

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
        echo "[ERROR] compilation failed!"
        exit 1
    }
done

for structure in "${structures[@]}"; do
    make DEBUG=OFF HASH=hash CMP=inline LIST_TYPE=$structure all || {
        echo "[ERROR] compilation failed!"
        exit 1
    }
done

# Остывание процессора
echo "--- Cooling down after rebuilding ---"

sleep 5

mkdir -p "$RESULT_DIR/opti"
mkdir -p "$IMAGE_DIR/opti"
mkdir -p "$RESULT_DIR/list"
mkdir -p "$IMAGE_DIR/list"

# Стадия тестирования

echo "--- Running the opti benchmark ---"

for level in "${levels_order[@]}"; do
    binary_name="${opti_bin[$level]}"

    echo -e "\t- Running the file $binary_name with opti=$level"

    taskset -c 0 ./scripts/monitor.sh "$CORE_ID" "$RESULT_DIR/opti/monitor_$level.csv" "$MONITOR_INTERVAL" &
    monitor_pid=$!

    taskset -c "$CORE_ID" "$BINARY_DIR/$binary_name" \
        -f "$DATA_FILE" \
        -i "$LOOKUP_ITERATIONS" \
        -s "$SAMPLE_COUNT" > "$RESULT_DIR/opti/raw_$level.txt"

    sudo kill "$monitor_pid"
    unset monitor_pid

    echo -e "\t- Cooling down after startup"

    sleep 5
done

echo "--- Running the list benchmark ---"

for structure in "${structures[@]}"; do
    binary_name="${list_bin[$structure]}"

    echo -e "\t- Running the file $binary_name with structure=$structure"

    taskset -c 0 ./scripts/monitor.sh "$CORE_ID" "$RESULT_DIR/list/monitor_$structure.csv" "$MONITOR_INTERVAL" &
    monitor_pid=$!

    taskset -c "$CORE_ID" "$BINARY_DIR/$binary_name" \
        -f "$DATA_FILE" \
        -i "$LOOKUP_ITERATIONS" \
        -s "$SAMPLE_COUNT" > "$RESULT_DIR/list/raw_$structure.txt"

    sudo kill "$monitor_pid"
    unset monitor_pid

    echo -e "\t- Cooling down after startup"

    sleep 5
done

clear_vars

echo "--- Image creation ---"

mkdir -p "$IMAGE_DIR"

for level in "${levels_order[@]}"; do
    csv_file="$RESULT_DIR/opti/monitor_$level.csv"
    output_file="$IMAGE_DIR/opti/monitor_$level.png"
    python3 "$SCRIPT_DIR/plot_monitor.py" "$csv_file" "$output_file"
done

output_file="$IMAGE_DIR/opti/compare_opti.png"
python3 "$SCRIPT_DIR/compare_levels.py" "$LOOKUP_ITERATIONS" "$output_file" "$RESULT_DIR/opti/"raw_*.txt

for structure in "${structures[@]}"; do
    csv_file="$RESULT_DIR/list/monitor_$structure.csv"
    output_file="$IMAGE_DIR/list/monitor_$structure.png"
    python3 "$SCRIPT_DIR/plot_monitor.py" "$csv_file" "$output_file"
done

output_file="$IMAGE_DIR/list/compare_list.png"
python3 "$SCRIPT_DIR/compare_levels.py" "$LOOKUP_ITERATIONS" "$output_file" "$RESULT_DIR/list/"raw_*.txt

echo "--- Finish! ---"
