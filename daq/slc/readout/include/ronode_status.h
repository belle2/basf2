/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_ronode_status_h
#define _Belle2_ronode_status_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int ronode_status_revision = 1;

  struct ronode_status {
    uint32 nodeid;
    uint32 state;
    uint32 eflag;
    uint32 expno;
    uint32 runno;
    uint32 subno;
    uint32 reserved_i[2];
    uint32 stime;
    uint32 ctime;
    uint32 nevent_in;
    uint32 nqueue_in;
    uint32 connection_in;
    uint32 connection_out;
    uint32 nevent_out;
    uint32 nqueue_out;
    float evtrate_in;
    float evtsize_in;
    float flowrate_in;
    float evtrate_out;
    float evtsize_out;
    float flowrate_out;
    float loadavg;
    float reserved_f[5];
    struct event_header {
      uint32 nword;
      uint32 format;
      uint32 exp_run;
      uint32 evtno;
      uint32 ctime_trgtype;
      uint32 utime;
      uint32 nodeid;
      uint32 crc_err;
    } header;
  };

}

#endif
