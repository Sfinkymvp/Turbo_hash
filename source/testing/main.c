#include <stdio.h>
// #include <ittnotify.h>

#include "common/report.h"
#include "testing/io.h"
#include "testing/benchmark.h"

int main(int argc, char *const *argv)
{
    // __itt_pause();

    BenchmarkContext context = {};
    Args args = {};
    int status = 0;

    status = parse_args(&args, argc, argv);
    if (status != 0) {
        goto cleanup;
    }
    INFO("arguments parsing finished");

    DEBUG("naive: %016llx", hash_string_crc32_naive("Hello, world!"));
    DEBUG("intrs: %016llx", hash_string_crc32_intr("Hello, world!"));

    DEBUG("using hash func %p", args.hash_func);
    DEBUG("naive func: %p, intr func: %p", hash_string_crc32_naive,
        hash_string_crc32_intr);

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
    printf("%lu\n", context.lookup_time);

cleanup:
    destroy_benchmark_context(&context); 
    return status;
}