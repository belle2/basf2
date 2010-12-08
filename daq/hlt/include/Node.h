/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>

#include <daq/hlt/HLTDefs.h>
#include <daq/hlt/SignalMan.h>

namespace Belle2 {
  class Node {
  public:
    Node(ENodeType nodeType, std::vector<std::string> dest);
    virtual ~Node(void);

    EStatus init(void);

  private:
    ENodeType m_nodeType;

    SignalMan* m_signalMan;
    std::vector<std::string> m_dest;
  };
}

#endif
