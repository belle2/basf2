/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_NSMState_hh
#define _Belle2_NSMState_hh

#include "daq/slc/base/Enum.h"

namespace Belle2 {

  class NSMState : public Enum {

  public:
    static const NSMState ONLINE_S;

  public:
    NSMState() : Enum(Enum::UNKNOWN) {}
    NSMState(const Enum& num) : Enum(num) {}
    NSMState(const NSMState& st) : Enum(st) {}
    ~NSMState()  {}

  protected:
    NSMState(int id, const char* label)
      : Enum(id, label) {}

  };

}

#endif
