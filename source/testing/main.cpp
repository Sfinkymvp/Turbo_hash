#include <stdio.h>
#include <stdlib.h>

#include "common/report.h"
#include "testing/io.h"
#include "testing/benchmark.h"

int main(int argc, char *const *argv)
{
    srand(42);

    BenchmarkContext context = {};
    Args args = {};
    int status = 0;

    status = parse_args(&args, argc, argv);
    if (status != 0) {
        goto cleanup;
    }
    INFO("arguments parsing finished");

    status = create_benchmark_context(&context, &args);
    if (status != 0){ 
        goto cleanup;
    }
    INFO("creating behchmark context finished");

    status = run_benchmark(&context);
    if (status != 0) {
        goto cleanup;
    }
    INFO("benchmark finished");

    print_results(&context);
    INFO("Results printed");

cleanup:
    destroy_benchmark_context(&context); 
    return status;
}