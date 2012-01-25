#ifndef HLTPROCESS_H
#define HLTPROCESS_H

#include <sstream>
#include <wait.h>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>

#include <daq/hlt/HLTSender.h>
#include <daq/hlt/HLTReceiver.h>
#include <daq/hlt/NodeInfo.h>
#include <framework/pcore/RingBuffer.h>

namespace Belle2 {
  class HLTProcess {
  public:
    HLTProcess();
    ~HLTProcess();

    EHLTStatus initControl();
    EHLTStatus initSenders();
    EHLTStatus initReceivers();

    EHLTStatus process();
    EHLTStatus checkChildren();

    bool isChild();
    void displayMode(const std::string mode);

  private:
    std::vector<pid_t> m_HLTSenders;
    std::vector<pid_t> m_HLTReceivers;
    pid_t m_Process, m_Control, m_Monitor;

    bool m_isChild;

    RingBuffer* m_controlInBuffer;
    RingBuffer* m_controlOutBuffer;
    RingBuffer* m_dataInBuffer;
    RingBuffer* m_dataOutBuffer;

    NodeInfo m_nodeInfo;
  };
}

#endif
