#include <stdio.h>

#include "common/report.h"
#include "testing/io.h"
#include "testing/benchmark.h"


int main(int argc, char *const *argv)
{
    BenchmarkContext context = {};
    Args args = {};
    int status = 0;

    status = parse_args(&args, argc, argv);
    if (status != 0) {
        goto cleanup;
    }

    REPORT(stderr, "creating bench context");
    status = create_benchmark_context(&context, &args);
    if (status != 0){ 
        goto cleanup;
    }

    REPORT(stderr, "running benchmark");
    status = run_benchmark(&context);
    if (status != 0) {
        goto cleanup;
    }

    printf("total time: %lu\n", context.lookup_time);

cleanup:
    destroy_benchmark_context(&context); 
    return status;
}