#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
    int r = hello();
    printf("hello returned %d\n", r);
    return 0;
}