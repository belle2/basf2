/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef RFNODEINFO_H
#define RFNODEINFO_H

// Node info placed in NSM shared memory
namespace Belle2 {
  struct RfNodeInfo {
    int sysstate;
    int nevent_in;
    int nqueue_in;
    int nevent_out;
    int nqueue_out;
    int error;
    int pid_input;
    int pid_output;
    int pid_basf2;
    int pid_hserver;
    int pid_hrelay;
    int i_reserved[4];
    float flowrate_in;
    float flowrate_out;
    float avesize_in;
    float avesize_out;
    float evtrate_in;
    float evtrate_out;
    float loadave;
    float r_reserved[9];
  };
}
#endif




