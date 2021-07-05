/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/version/Version.h"

//define if DEF_DAQ_SLC_VERSION is not set at compilation using `g++ -DDEF_DAQ_SLC_VERSION=\"version_whatever\"
#ifndef DEF_DAQ_SLC_VERSION
//use default "NOT_SET"
#define DEF_DAQ_SLC_VERSION "NOT_SET"
#endif

const std::string DAQ_SLC_VERSION::DAQ_SLC_VERSION(DEF_DAQ_SLC_VERSION);

