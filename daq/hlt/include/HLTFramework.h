/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTFRAMEWORK_H
#define HLTFRAMEWORK_H

#include <string>

#include <daq/hlt/HLTDefs.h>
#include <daq/hlt/HLTManager.h>
#include <daq/hlt/HLTProcess.h>

namespace Belle2 {
  class HLTFramework {
  public:
    HLTFramework(ENodeType nodeType = c_ProcessNode);
    ~HLTFramework();
  private:
    ENodeType m_nodeType;

    HLTManager* m_HLTManager;
    HLTProcess* m_HLTProcess;

    std::string m_inputNodeInfo;
  };
}

#endif

