/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_FifoWriter_hh
#define _Belle2_FifoWriter_hh

#include "daq/slc/system/FileWriter.h"
#include "daq/slc/system/Fifo.h"

namespace Belle2 {

  typedef FileWriter<Fifo> FifoWriter;

}

#endif
