/*
 * test_6502.c
 *
 * test the "cpu6502" core.
 */

/* $Id: test_6502.c,v 1.8 2001/03/16 20:08:29 nyef Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "ui.h"
#include "cpu6502.h"
#include "memtypes.h"
#include "tool.h"

#include "cpu6502int.h"

#define MAX_MEMLOCS 32

struct cpu6502_context *context;

u8 page_0[0x200];

u32 system_flags;

int did_test_fail;
int num_passed;
int num_failed;
int in_test;

char cur_test[256];

int cur_line;

typedef void (*testfunc)(void);

struct memloc {
    u32 address;
    u8 data;
};

struct memloc memlocs[MAX_MEMLOCS];
int memlocs_used;

#define SET_MEMORY(addr, data8) do { memlocs[memlocs_used].data = data8; memlocs[memlocs_used].address = addr; memlocs_used++; if (addr < 0x200) page_0[addr] = data8; } while (0)

u8 test_read8(struct cal_cpu *cpu, u32 address)
{
    int i;
    u32 addr;
    
    addr = address & 0xffffff;

    if (addr < 0x0200) {
	return page_0[addr];
    }
    
    for (i = 0; i < memlocs_used; i++) {
	if (memlocs[i].address == addr) {
	    return memlocs[i].data;
	}
    }

    printf("read8 access to unknown location 0x%06lx.\n", addr);

    did_test_fail = 1;
    
    return 0;
}

void test_write8(struct cal_cpu *cpu, u32 address, u8 data)
{
    int i;
    u32 addr;

    addr = address & 0xffffff;
    
    if (addr < 0x0200) {
	page_0[addr] = data;
    }
    
    for (i = 0; i < memlocs_used; i++) {
	if (memlocs[i].address == addr) {
	    memlocs[i].data = data;
	    return;
	}
    }

    printf("write8 access 0x%02x to unknown location 0x%06lx.\n", data, addr);

    did_test_fail = 1;
}

void clear_cpu(void)
{
    bzero(page_0, sizeof(page_0));
    
    context->reg_a = 0x00;
    context->reg_x = 0x00;
    context->reg_y = 0x00;
    context->reg_s = 0xff;

    context->pc = 0;
    context->flags = 0;

#ifndef NO_CYCLE_CHECKS
    context->cycles_left = 0;
#endif
    
    memlocs_used = 0;
}

void deb_printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    did_test_fail = 1;
}

char *strip_blanks(char *buf)
{
    char *bufptr;
    
    bufptr = buf;

    while (isblank(*bufptr)) {
	bufptr++;
    }

    return bufptr;
}

typedef int (*line_handler)(char *);

struct parse_table {
    char *match;
    line_handler handler;
};

int do_start(char *buf)
{
    if (in_test) {
	printf("start found while already in test.\n");
	return 1;
    }

    if (!*buf) {
	printf("test has no name.\n");
	return 1;
    }

    strcpy(cur_test, buf);
    
    did_test_fail = 0;
    clear_cpu();
    in_test = 1;
    return 0;
}

int do_set_reg(char *buf)
{
    u8 *reg;
    char *bufptr;
    
    if (buf[0] == 'a') {
	reg = &context->reg_a;
    } else if (buf[0] == 'x') {
	reg = &context->reg_x;
    } else if (buf[0] == 'y') {
	reg = &context->reg_y;
    } else if (buf[0] == 's') {
	reg = &context->reg_s;
    } else {
	printf("unknown reg.\n");
	return 1;
    }

    bufptr = buf + 1;

    bufptr = strip_blanks(bufptr);
    
    if (*bufptr != '=') {
	printf("expected '='.\n");
	return 1;
    }

    bufptr++;
    
    bufptr = strip_blanks(bufptr);

    *reg = strtoul(bufptr, &bufptr, 0);

    return 0;
}

int do_set_mem(char *buf)
{
    char *bufptr;
    unsigned long addr;
    unsigned short data;

    addr = strtoul(buf, &bufptr, 0);

    bufptr = strip_blanks(bufptr);

    if (*bufptr != '=') {
	printf("expected '='.\n");
	return 1;
    }

    bufptr++;
    
    bufptr = strip_blanks(bufptr);
    
    data = strtoul(bufptr, &bufptr, 0);

    SET_MEMORY(addr, data);
    
    return 0;
}

int do_set_flags(char *buf)
{
    u8 flags;
    
    flags = strtoul(buf, NULL, 0);

    SET_FLAGS(context, flags);
    
    return 0;
}

int do_set_cycles(char *buf)
{
    if (!isdigit(*buf)) {
	printf("expected number.\n");
	return 1;
    }
    
#ifndef NO_CYCLE_CHECKS
    context->cycles_left = strtol(buf, NULL, 0);
#endif
    
    return 0;
}

int do_set_pc(char *buf)
{
    context->pc = strtoul(buf, NULL, 0);
    
    return 0;
}

struct parse_table set_line_types[] = {
    {"reg",      do_set_reg},
    {"register", do_set_reg},
    {"mem",      do_set_mem},
    {"memory",   do_set_mem},
    {"pc",       do_set_pc},
    {"cycles",   do_set_cycles},
    {"flags",    do_set_flags},
    {NULL, NULL}, /* must be last entry */
};

int do_set(char *buf)
{
    struct parse_table *cur_match;
    char *bufptr;
    int match_len;

    if (!in_test) {
	printf("set found while not in test.\n");
	return 1;
    }
    
    bufptr = buf;

    for (cur_match = set_line_types; cur_match->match; cur_match++) {
	match_len = strlen(cur_match->match);
	if (!strncmp(bufptr, cur_match->match, match_len)) {
	    if (bufptr[match_len] && (!isblank(bufptr[match_len]))) {
		continue;
	    }
	    
	    bufptr += match_len;
	    bufptr = strip_blanks(bufptr);
	    
	    return cur_match->handler(bufptr);
	}
    }

    return 1;
}

int do_run(char *buf)
{
    if (!in_test) {
	printf("run found while not in test.\n");
	return 1;
    }
    
    cpu6502_step(context);
    return 0;
}

void check_reg(u8 reg, u8 data)
{
    if (reg != data) {
	printf("check reg: expected 0x%02x, found 0x%02x.\n", data, reg);
	did_test_fail = 1;
    }
}

int do_check_reg(char *buf)
{
    u8 *reg;
    u8 data;
    char *bufptr;
    
    if (buf[0] == 'a') {
	reg = &context->reg_a;
    } else if (buf[0] == 'x') {
	reg = &context->reg_x;
    } else if (buf[0] == 'y') {
	reg = &context->reg_y;
    } else if (buf[0] == 's') {
	reg = &context->reg_s;
    } else {
	printf("unknown reg.\n");
	return 1;
    }

    bufptr = buf + 1;

    bufptr = strip_blanks(bufptr);
    
    if (*bufptr != '=') {
	printf("expected '='.\n");
	return 1;
    }

    bufptr++;
    
    bufptr = strip_blanks(bufptr);

    data = strtoul(bufptr, &bufptr, 0);

    check_reg(*reg, data);
    
    return 0;
}

void check_mem(u16 addr, u8 data)
{
    u8 data2;
    
    data2 = test_read8(NULL, addr);
    
    if (data != data2) {
	printf("check mem: expected 0x%02x, found 0x%02x.\n", data, data2);
	did_test_fail = 1;
    }
}

int do_check_mem(char *buf)
{
    char *bufptr;
    unsigned long addr;
    u8 data;

    addr = strtoul(buf, &bufptr, 0);

    bufptr = strip_blanks(bufptr);

    if (*bufptr != '=') {
	printf("expected '='.\n");
	return 1;
    }

    bufptr++;
    
    bufptr = strip_blanks(bufptr);
    
    data = strtoul(bufptr, &bufptr, 0);

    check_mem(addr, data);
    
    return 0;
}

void check_flags(u8 data)
{
    u8 flags;
    
    flags = GET_FLAGS(context);
    
    if (data != flags) {
	printf("check flags: expected 0x%02x, found 0x%02x.\n", data, flags);
	did_test_fail = 1;
    }
}

int do_check_flags(char *buf)
{
    u8 data;

    if (!isdigit(*buf)) {
	printf("expected number.\n");
	return 1;
    }
    
    data = strtoul(buf, NULL, 0);

    check_flags(data);
    
    return 0;
}

int do_check_cycles(char *buf)
{
    int data;
    
    data = strtol(buf, NULL, 0);

#ifndef NO_CYCLE_CHECKS
    if (data != context->cycles_left) {
	printf("check cycles: expected 0x%04x, found 0x%04x.\n", data, context->cycles_left);
	did_test_fail = 1;
    }
#endif
    
    return 0;
}

int do_check_pc(char *buf)
{
    u16 data;
    
    data = strtoul(buf, NULL, 0);
    
    if (data != context->pc) {
	printf("check pc: expected 0x%04hx, found 0x%04hx.\n", data, context->pc);
	did_test_fail = 1;
    }
    
    return 0;
}

struct parse_table check_line_types[] = {
    {"reg",      do_check_reg},
    {"register", do_check_reg},
    {"mem",      do_check_mem},
    {"memory",   do_check_mem},
    {"pc",       do_check_pc},
    {"cycles",   do_check_cycles},
    {"flags",    do_check_flags},
    {NULL, NULL}, /* must be last entry */
};

int do_check(char *buf)
{
    struct parse_table *cur_match;
    char *bufptr;
    int match_len;

    if (!in_test) {
	printf("check found while not in test.\n");
	return 1;
    }
    
    bufptr = buf;

    for (cur_match = check_line_types; cur_match->match; cur_match++) {
	match_len = strlen(cur_match->match);
	if (!strncmp(bufptr, cur_match->match, match_len)) {
	    if (bufptr[match_len] && (!isblank(bufptr[match_len]))) {
		continue;
	    }
	    
	    bufptr += match_len;
	    bufptr = strip_blanks(bufptr);
	    
	    return cur_match->handler(bufptr);
	}
    }

    return 1;
}

int do_done(char *buf)
{
    if (!in_test) {
	printf("done found while not in test.\n");
	return 1;
    }
    
    if (did_test_fail) {
	printf("test \"%s\" failed.\n", cur_test);
	num_failed++;
    } else {
	num_passed++;
    }

    in_test = 0;
    
    return 0;
}

struct parse_table line_types[] = {
    {"start", do_start},
    {"set",   do_set},
    {"run",   do_run},
    {"check", do_check},
    {"done",  do_done}, /* Do Done is a song by Kudo Shizuka */
    {NULL, NULL}, /* must be last entry */
};

int parse_test_file_line(char *buf)
{
    struct parse_table *cur_match;
    char *bufptr;
    int match_len;

    bufptr = buf;

    bufptr = strip_blanks(bufptr);

    if (!*bufptr) {
	/* blank line, no action */
	return 0;
    }
    
    if (*bufptr == '#') {
	/* comment, no action */
	return 0;
    }

    for (cur_match = line_types; cur_match->match; cur_match++) {
	match_len = strlen(cur_match->match);
	if (!strncmp(bufptr, cur_match->match, match_len)) {
	    if (bufptr[match_len] && (!isblank(bufptr[match_len]))) {
		continue;
	    }
	    
	    bufptr += match_len;
	    bufptr = strip_blanks(bufptr);
	    
	    return cur_match->handler(bufptr);
	}
    }

    return 1;
}

void parse_test_file(FILE *testfile)
{
    char buf[256];
    
    in_test = 0;
    
    while (!feof(testfile)) {
	cur_line++;
	if (!fgets(buf, 256, testfile)) {
	    return;
	}
	if (buf[strlen(buf) - 1] == '\n') {
	    buf[strlen(buf) - 1] = '\0';
	}
	if (parse_test_file_line(buf)) {
	    printf("Syntax error, line %d.\n", cur_line);
	    did_test_fail = 1;
	}
    }
}

void create_cpu(void)
{
    context = cpu6502_create();

    cpu6502_setmemhandlers(context, test_read8, test_write8);
    cpu6502_setzpage(context, page_0);
}

#include "test_6502_1.h"

int main(int argc, char *argv[])
{
    num_passed = 0;
    num_failed = 0;

    create_cpu();
    
    printf("Running tests...\n");

    run_internal_tests();
    
    parse_test_file(stdin);

    if (in_test) {
	printf("Still in test at EOF.\n");
	printf("Syntax error, line %d.\n", cur_line + 1);
	num_failed++;
    }
    
    printf("Summary: %d passed, %d failed, %d total.\n", num_passed, num_failed, num_passed + num_failed);
    
    return 0;
}

/*
 * $Log: test_6502.c,v $
 * Revision 1.8  2001/03/16 20:08:29  nyef
 * changed to use the cpu6502 interface directly, instead of using the cal
 *
 * Revision 1.7  2001/03/16 19:58:40  nyef
 * removed the marat6502 testing code
 *
 * Revision 1.6  2000/09/23 02:29:29  nyef
 * dug check_{reg,mem,flags} out from do_check_{reg,mem,flags}
 *
 * Revision 1.5  2000/09/19 02:00:31  nyef
 * added an internal test system similar to test_68k.c
 *
 * Revision 1.4  2000/09/07 02:55:40  nyef
 * fixed bug added in last revision
 *
 * Revision 1.3  2000/08/25 02:48:55  nyef
 * started adding support for testing the Marat6502 core
 *
 * Revision 1.2  2000/07/15 22:19:01  nyef
 * converted to use lazy flag evaluation
 *
 * Revision 1.1  2000/05/01 00:35:51  nyef
 * Initial revision
 *
 */
