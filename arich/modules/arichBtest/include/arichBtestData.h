/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef DEFREC_H
#define DEFREC_H

enum { EVENT_RECORD_TYPE, BEGIN_RECORD_TYPE, PAUSE_RECORD_TYPE,
       RESUME_RECORD_TYPE, END_RECORD_TYPE
     };

//! Event record structure for the beamtest data
struct EventRec {
//! Record ID
  unsigned int type;
//! Record length in bytes
  unsigned int len;
//! Event Number
  unsigned int evtno;
//! Event timestamp
  unsigned int time;
//! Time form beginning of the run in ms
  unsigned int mstime;
//! Run Number
  unsigned int runno; // from run
};

//! Begin record structure for the beamtest data
struct BeginRec {
//! Record ID
  unsigned int type;
//! Record length in bytes
  unsigned int len;
//! Run number
  unsigned int runno;
//! Current event number
  unsigned int evtno;
//! Timestamp of the run
  unsigned int time;
//! Reserved - unused
  unsigned int reserve1;
};

//! End record structure for the beamtest data
struct EndRec {
//! Record ID
  unsigned int type;
//! Record length in bytes
  unsigned int len;
//! Run number
  unsigned int runno;
//! Current event number
  unsigned int evtno;
//! End Record Timestamp
  unsigned int time;
//! Reserved - unused
  unsigned int reserve1;
};

#endif
