/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_RCNode_h
#define _Belle2_RCNode_h

#include "daq/slc/nsm/NSMNode.h"

namespace Belle2 {

  class RCNode : public NSMNode {

  public:
    RCNode(const std::string& name = "")
      : NSMNode(name), m_sequential(false), m_isglobal(-1) {}
    virtual ~RCNode() {}

  public:
    bool isSequential() const { return m_sequential; }
    const std::string& getConfig() const { return m_config; }
    void setSequential(bool seq) { m_sequential = seq; }
    void setConfig(const std::string& config) { m_config = config; }
    void setGlobal(int isglobal) { m_isglobal = isglobal; }
    int isGlobal() const { return m_isglobal; }

  private:
    bool m_sequential;
    int m_isglobal;
    std::string m_config;

  };

};

#endif
