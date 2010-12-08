/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTPROCESS_H
#define HLTPROCESS_H

#include <daq/hlt/HLTDefs.h>
#include <daq/hlt/Node.h>
#include <daq/hlt/NodeManager.h>

namespace Belle2 {
  class HLTProcess {
  public:
    HLTProcess();
    ~HLTProcess();
  private:
    NodeManager* m_nodeManager;
  };
}

#endif

