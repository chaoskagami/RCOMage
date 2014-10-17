/*
 * rcodump.h - This file is part of RCOMage
 *
 * Copyright (C) ZiNgA BuRgA <zingaburga@hotmail.com>
 *
 * This software is distributed under the terms of the GNU Lesser General Public
 * License ("LGPL") version 2.1, as published by the Free Software Foundation.
 */

#ifndef __RCODUMP_H__
#define __RCODUMP_H__

typedef uint8_t (*OutputDumpFunc) (char *, void *, rRCOEntry *, void *);

typedef struct {
  char *cmd;
  char *ext;
  char *extFlags;
} RcoDumpGimconvOpts;

uint8_t dump_resource (char *dest, rRCOEntry * entry, OutputDumpFunc outputfunc,
    void *outputfuncArg);
void dump_resources (char *labels, rRCOEntry * parent, const RcoTableMap extMap,
    char *pathPrefix, void *outputFilterArg, int optional);
void dump_text_resources (char *labels, rRCOEntry * parent, uint8_t writeHeader,
    char *pathPrefix, uint8_t bWriteXML);
uint8_t dump_output_data (char *dest, void *buf, rRCOEntry * entry, void *arg);
uint8_t dump_output_vsmxdec (char *dest, void *buf, rRCOEntry * entry,
    void *arg);

void compile_gimconv_map (rRCOFile * rco, rRCOEntry * entry, void *arg);
void compile_vagconv_map (rRCOFile * rco, rRCOEntry * entry, void *arg);
void compile_vsmxconv_map (rRCOFile * rco, rRCOEntry * entry, void *arg);
void compile_wavcheck_map (rRCOFile * rco, rRCOEntry * entry, void *arg);

#endif
