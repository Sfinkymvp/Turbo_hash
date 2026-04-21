#include <stdio.h>

#include "common/report.h"
#include "table/table.h"


int main()
{
    REPORT(stderr, "Hello, %s!", "User");

    return 0;
}