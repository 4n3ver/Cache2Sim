//
// Created by lennart on 1/27/16.
//
#include <iostream>
#include <stdio.h>
#include <string>
#include "cache.h"

int main()
{
    uint64_t addr = 0x5214abc84a8f8af2;
    cache* test = new cache(16, 0, 0);
    printf("Test for %lx tag: %lx index %lx offset: %lx\n", addr,
           test->tag_of(addr), test->index_of(addr), test->offset_of(addr));
    delete test;
    return 0;
}
