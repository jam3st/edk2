#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef u64 resource_t;

struct resource {
    resource_t base;    /* Base address of the resource */
    resource_t size;    /* Size of the resource */
    resource_t limit;   /* Largest valid value base + size -1 */
    unsigned long flags;    /* Descriptions of the kind of resource */
    unsigned long index;    /* Bus specific per device resource id */
    unsigned char align;    /* Required alignment (log 2) of the resource */
    unsigned char gran; /* Granularity (log 2) of the resource */
    /* Alignment must be >= the granularity of the resource */
};

