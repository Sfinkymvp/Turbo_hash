TASKSET_CALL="taskset -c 0"
FLAGS="-fdata/data3.txt -i1000000 -1"
FILE_PATH="./bin"

$TASKSET_CALL ${FILE_PATH}/bench_libc $FLAGS
$TASKSET_CALL ${FILE_PATH}/bench_strcmp $FLAGS
$TASKSET_CALL ${FILE_PATH}/bench_strcmp1 $FLAGS
$TASKSET_CALL ${FILE_PATH}/bench_strcmp2 $FLAGS