/*
 * tiledraw.c
 *
 * drawing routines for tilecached VDPs
 */

/* $Id: tiledraw.c,v 1.5 1999/11/27 23:47:03 nyef Exp $ */

#include "tiledraw.h"

void tiledraw_8(unsigned char *cur_vbp, unsigned char **tiles, unsigned char **palettes, int num_tiles, int finescroll)
{
    int i;
    unsigned char *palette;
    unsigned char *tiledata;
    //printf("ENTERING TILDRAW\n");
    palette = *palettes++;
    tiledata = *tiles++;
    if (!tiledata) {
        //printf("tiledata is NONE\n");
	cur_vbp += (8 - finescroll);
    } else {
	tiledata += finescroll;
	switch (finescroll) {
	case 0:
	    if (*tiledata) {
                //printf("tiledata is not NONE\n");
		*cur_vbp = palette[*tiledata];
            }
            cur_vbp++;
	    tiledata++;
	case 1:
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	case 2:
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	case 3:
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	case 4:
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	case 5:
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	case 6:
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	case 7:
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	}
    }
    
    for (i = num_tiles; i > 1; i--) {
	palette = *palettes++;
	tiledata = *tiles++;
	if (!tiledata) {
            //printf("tiledata is NONE\n");
	    cur_vbp += 8;
	} else {
	    if (*tiledata) {
                //printf("tiledata is not NONE (2) tile=%d\n",i);
		*cur_vbp = palette[*tiledata];
            }
	    cur_vbp++;
	    tiledata++;
	    if (*tiledata) {
                //printf("tiledata is not NONE (3)\n");
		*cur_vbp = palette[*tiledata];
            }
	    cur_vbp++;
	    tiledata++;
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	    if (*tiledata)
		*cur_vbp = palette[*tiledata];
	    cur_vbp++;
	    tiledata++;
	}
    }
    
    if (finescroll) {
	palette = *palettes++;
	tiledata = *tiles++;
	if (tiledata) {
	    switch (finescroll) {
	    case 7:
		if (*tiledata)
		    *cur_vbp = palette[*tiledata];
		cur_vbp++;
		tiledata++;
	    case 6:
		if (*tiledata)
		    *cur_vbp = palette[*tiledata];
		cur_vbp++;
		tiledata++;
	    case 5:
		if (*tiledata)
		    *cur_vbp = palette[*tiledata];
		cur_vbp++;
		tiledata++;
	    case 4:
		if (*tiledata)
		    *cur_vbp = palette[*tiledata];
		cur_vbp++;
		tiledata++;
	    case 3:
		if (*tiledata)
		    *cur_vbp = palette[*tiledata];
		cur_vbp++;
		tiledata++;
	    case 2:
		if (*tiledata)
		    *cur_vbp = palette[*tiledata];
		cur_vbp++;
		tiledata++;
	    case 1:
		if (*tiledata)
		    *cur_vbp = palette[*tiledata];
		cur_vbp++;
		tiledata++;
	    }
	}
    }
}

/*
 * $Log: tiledraw.c,v $
 * Revision 1.5  1999/11/27 23:47:03  nyef
 * changed to allow null pointers for tile data (treats as a blank tile)
 *
 * Revision 1.4  1999/11/26 02:25:07  nyef
 * converted to use *pointer++ instead of pointer[index]
 * reversed the index in the for loop
 *
 * Revision 1.3  1999/11/23 03:29:53  nyef
 * fixed a bug with finescroll on the right side of the screen
 *
 * Revision 1.2  1999/11/23 03:21:53  nyef
 * changed to use new tiledraw interface
 *
 * Revision 1.1  1999/11/23 01:41:06  nyef
 * Initial revision
 *
 */
