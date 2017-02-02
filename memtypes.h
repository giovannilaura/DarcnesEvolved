/*
 * memtypes.h
 *
 * CPU memory access definitions (function pointer and ranged memory types)
 */

/* $Id: memtypes.h,v 1.1 2001/03/16 18:18:29 nyef Exp $ */

#ifndef MEMTYPES_H
#define MEMTYPES_H

struct cal_cpu;

typedef unsigned char (*memread8_t)(struct cal_cpu *, unsigned long);
typedef void (*memwrite8_t)(struct cal_cpu *, unsigned long, unsigned char);
typedef unsigned short (*memread16_t)(struct cal_cpu *, unsigned long);
typedef void (*memwrite16_t)(struct cal_cpu *, unsigned long, unsigned short);
typedef unsigned long (*memread32_t)(struct cal_cpu *, unsigned long);
typedef void (*memwrite32_t)(struct cal_cpu *, unsigned long, unsigned long);

typedef struct {
    unsigned long start;
    unsigned long end;
    unsigned char *data;
    unsigned short flags;
    unsigned short tag;
} ranged_mmu;

#endif

/*
 * $Log: memtypes.h,v $
 * Revision 1.1  2001/03/16 18:18:29  nyef
 * Initial revision
 *
 */
