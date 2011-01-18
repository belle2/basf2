/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef NODEINFO_H
#define NODEINFO_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>

#include <framework/logging/Logger.h>

namespace Belle2 {

  class NodeInfo {
  public:
    NodeInfo(void);
    NodeInfo(const std::string type, const int unitNo, const int nodeNo);
    ~NodeInfo(void);

    void init(void);

    int unitNo(void);
    int nodeNo(void);
    std::string type(void);

    void setPortData(int inPort, int outPort);
    void setPortControl(int port);
    int portBaseDataIn(void);
    int portBaseDataOut(void);
    int portBaseControl(void);

    void setThisIP(const std::string ip);
    void setManagerIP(const std::string ip);
    void setSourceIP(std::string ip);
    void setSourceIP(std::vector<std::string> ip);
    void setTargetIP(std::string ip);
    void setTargetIP(std::vector<std::string> ip);

    std::string thisIP(void);
    std::string managerIP(void);
    std::vector<std::string> sourceIP(void);
    std::vector<std::string> targetIP(void);

    void setSteering(const char* steering);
    void getSteering(void);

    std::string serializedNodeInfo(void);
    void deserializedNodeInfo(const std::string nodeinfo);

    void Print(void);

  private:
    int m_unitNo, m_nodeNo;

    std::string m_type;
    int m_portBaseDataIn, m_portBaseDataOut;
    int m_portBaseControl;

    std::string m_thisIP;
    std::string m_managerIP;
    std::vector<std::string> m_sourceIP;
    std::vector<std::string> m_targetIP;

    char* m_steeringName;
    char* m_steeringContents;
  };
}

#endif
