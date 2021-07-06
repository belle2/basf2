/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef mybuf_h
#define mybuf_h

const int mybuf_revision = 3;

struct mybuf {
  uint32 reserved[300000];
  uint32 nodeid;
  uint32 state;
  uint32 ctime;
  uint32 eflag;
  uint32 expno;
  uint32 runno;
  uint32 subno;
  uint32 reserved2[2];
  struct io_status {
    uint32 nqueue;
    uint32 state;
    uint32 count;
    uint32 reserved_i;
    float freq;
    float evtsize;
    float rate;
    float reserved_f;
  } io[2];
};

#endif
