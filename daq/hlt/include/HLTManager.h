#ifndef HLTMANAGER_H
#define HLTMANAGER_H

#include <map>
#include <wait.h>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>

#include <daq/hlt/XMLParser.h>
#include <daq/hlt/NodeInfo.h>
#include <daq/hlt/HLTSender.h>
#include <framework/pcore/RingBuffer.h>

namespace Belle2 {
  class HLTManager {
  public:
    HLTManager();
    ~HLTManager();

    EHLTStatus initSenders();

    EHLTStatus storeNodeInfo(XMLParser* xml);
    std::string encodeNodeInfo(unsigned int key);
    void decodeNodeInfo(std::string nodeinfo);

    bool isChild();
    EHLTStatus checkChildren();

    void printNodeInfo();

  private:
    std::map<int, NodeInfo> m_nodeInfoMap;
    std::vector<pid_t> m_senders;
    bool m_isChild;

    std::vector<int> m_controlBuffers;
  };
}

#endif
