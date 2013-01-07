/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/HLTProcess.h>

using namespace Belle2;

/// @brief HLTProcess constructor
HLTProcess::HLTProcess()
{
  m_isChild = false;

  m_controlInBuffer = NULL;
  m_controlOutBuffer = NULL;
  m_dataInBuffer = NULL;
  m_dataOutBuffer = NULL;
}

/// @brief HLTProcess destructor
HLTProcess::~HLTProcess()
{
  if (!m_isChild) {
    delete m_controlInBuffer;
    //delete m_controlOutBuffer;
    delete m_dataInBuffer;
    delete m_dataOutBuffer;
  }
}

/// @brief Initialize the control line to manager node
/// @return c_Success Initialization done
/// @return c_ChildSuccess The process is child process
EHLTStatus HLTProcess::initControl(int port)
{
  B2INFO("[HLTProcess] \x1b[32mRing buffers initializing for control/monitor line\x1b[0m");
  if (!port)
    m_controlInBuffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(c_ControlPort)).c_str(), gBufferSize);
  else
    m_controlInBuffer = new RingBuffer(boost::lexical_cast<std::string>(port).c_str(), gBufferSize);

  pid_t pid = fork();
  if (pid == 0) {
    m_isChild = true;

    HLTReceiver receiver(c_ControlPort, 1);
    if (port)
      receiver.setPort(port);
    receiver.createConnection();
    receiver.setBuffer();
    receiver.setMode(c_ManagerNode);

    receiver.listening();

    m_controlInBuffer = NULL;
    m_controlOutBuffer = NULL;

    return c_ChildSuccess;
  } else {
    B2INFO("\x1b[33m[HLTProcess] EvtReceiver (Control) " << pid << " forked\x1b[0m");
    m_Control = pid;

    B2INFO("[HLTProcess/Mother] \x1b[32mControl line initialized!\x1b[0m");
    bool flag = false;
    while (!flag) {
      if (m_controlInBuffer->numq() > 0) {
        char tmp[gMaxReceives];
        m_controlInBuffer->remq((int*)tmp);

        NodeInfo tempNodeinfo;
        std::stringstream sNodeInfo;
        sNodeInfo << tmp;

        boost::archive::text_iarchive ia(sNodeInfo);
        ia >> tempNodeinfo;

        m_nodeInfo = tempNodeinfo;
        m_nodeInfo.display();

        flag = true;
      }
    }

    return c_Success;
  }
}

/// @brief Initialize HLTSenders for outgoing data flow
/// @return c_Success Initialization done
/// @return c_ChildSuccess The process is child process
EHLTStatus HLTProcess::initSenders(int port)
{
  B2INFO("[HLTProcess] \x1b[33mOutgoing ring buffer initializing...\x1b[0m");
  m_dataOutBuffer = new RingBuffer(gDataOutBufferKey.c_str(), gBufferSize);

  for (unsigned int i = 0; i < m_nodeInfo.targetIP().size(); ++i) {
    pid_t pidHLTSender = fork();
    if (pidHLTSender == 0) {
      m_isChild = true;

      if (!port) {
        port = c_DataOutPort;
        if (m_nodeInfo.type() == "WN") {
          port = c_DataInPort;
          port += m_nodeInfo.generateKey() - 1;
        } else
          port += m_nodeInfo.unitNo() * 100 + i;
      } else {
        if (m_nodeInfo.type() == "WN") {
          port = 30000;
        } else {
          port = c_DataOutPort;
          port += m_nodeInfo.unitNo() * 100 + i;
        }
      }

      HLTSender hltSender(m_nodeInfo.targetIP()[i], port);
      hltSender.createConnection();
      hltSender.setBuffer(gDataOutBufferKey);
      hltSender.setMode(c_ProcessNode);

      while (1) {
        if (hltSender.broadcasting() == c_TermCalled)
          break;
      }

      m_dataOutBuffer = NULL;

      return c_ChildSuccess;
    } else {
      m_HLTSenders.push_back(pidHLTSender);
      B2INFO("\x1b[33m[HLTProcess] HLTSender " << pidHLTSender << " forked\x1b[0m");
    }
  }

  return c_Success;
}

/// @brief Initialize HLTReceivers for incoming data flow
/// @return c_Success Initialization done
/// @return c_ChildSuccess The process is child process
EHLTStatus HLTProcess::initReceivers(int port)
{
  B2INFO("[HLTProcess] \x1b[33mIncoming ring buffer initializing...\x1b[0m");
  m_dataInBuffer = new RingBuffer(gDataInBufferKey.c_str(), gBufferSize);

  bool testPort = false;
  if (port) {
    testPort = true;
    B2INFO("[HLTProcess] External port " << port << " specified!");
  }

  for (unsigned int i = 0; i < m_nodeInfo.sourceIP().size(); ++i) {
    pid_t pidHLTReceiver = fork();
    if (pidHLTReceiver == 0) {
      m_isChild = true;

      if (!testPort) {
        port = c_DataInPort;
        if (m_nodeInfo.type() == "WN") {
          port = c_DataOutPort;
          port += m_nodeInfo.generateKey() - 1;
        } else
          port += m_nodeInfo.unitNo() * 100 + i;
      } else {
        if (m_nodeInfo.type() != "WN") {
          port = 30000;
        } else {
          port = c_DataOutPort;
          port += m_nodeInfo.generateKey() - 1;
        }
      }

      HLTReceiver hltReceiver(port, m_nodeInfo.sourceIP().size());
      hltReceiver.createConnection();
      hltReceiver.setBuffer(gDataInBufferKey);
      hltReceiver.setMode(c_ProcessNode);

      hltReceiver.listening();

      return c_ChildSuccess;
    } else {
      m_HLTReceivers.push_back(pidHLTReceiver);
      B2INFO("\x1b[33m[HLTProcess] HLTReceiver " << pidHLTReceiver << " forked\x1b[0m");

      if (testPort && i == 4)
        return c_Success;
    }
  }

  return c_Success;
}

/// @brief Run basf2 with predefined module chain for assigned node type properly
/// @return c_Success Processing completed
EHLTStatus HLTProcess::process()
{
  pid_t pidProcess = fork();
  if (pidProcess == 0) {
    m_isChild = true;

    if (m_nodeInfo.type() == "ES") {
      displayMode("Event separator");
      system("basf2 $BELLE2_LOCAL_DIR/daq/data/eventSeparator.py");
    } else if (m_nodeInfo.type() == "WN") {
      displayMode("Worker node");
      system("basf2 $BELLE2_LOCAL_DIR/daq/data/workerNode.py");
    } else if (m_nodeInfo.type() == "EM") {
      displayMode("Event merger");
      system("basf2 $BELLE2_LOCAL_DIR/daq/data/eventMerger.py");
    }
  } else {
    B2INFO("\x1b[33m[HLTProcess] basf2 " << pidProcess << " forked\x1b[0m");
    m_Process = pidProcess;
  }

  return c_Success;
}

/// @brief Check if the process is mother or child process
/// @return true The process is child process
/// @return false The process is mother process
bool HLTProcess::isChild()
{
  return m_isChild;
}

/// @brief Wait for child processes forked (For mother process only)
/// @return c_Success All the child processes are terminated
EHLTStatus HLTProcess::checkChildren()
{
  int status;
  pid_t pid;

  for (std::vector<pid_t>::const_iterator i = m_HLTSenders.begin();
       i != m_HLTSenders.end(); ++i) {
    pid = waitpid(*i, &status, 0);
    B2INFO("[HLTProcess] \x1b[33mHLTSender " << pid << " finished! (exit code="
           << status << ")\x1b[0m");
  }
  for (std::vector<pid_t>::const_iterator i = m_HLTReceivers.begin();
       i != m_HLTReceivers.end(); ++i) {
    pid = waitpid(*i, &status, 0);
    B2INFO("[HLTProcess] \x1b[33mHLTReceiver " << pid << " finished! (exit code="
           << status << ")\x1b[0m");
  }

  B2INFO("[HLTProcess] \x1b[33mTerminating basf2....(Put termination code into ring buffer "
         << m_dataInBuffer->shmid() << " and " << m_dataOutBuffer->shmid() << ")\x1b[0m");
  while (m_dataInBuffer->insq((int*)gTerminate.c_str(), gTerminate.size() / 4 + 1) <= 0) {
    usleep(100);
  }
  while (m_dataOutBuffer->insq((int*)gTerminate.c_str(), gTerminate.size() / 4 + 1) <= 0) {
    usleep(100);
  }

  pid = waitpid(m_Process, &status, 0);
  B2INFO("[HLTProcess] \x1b[33mbasf2 " << pid << " finished! (exit code="
         << status << ")\x1b[0m");

  pid = waitpid(m_Control, &status, 0);
  B2INFO("[HLTProcess] \x1b[33mHLTReceiver (Contol) " << pid << " finished! (exit code="
         << status << ")\x1b[0m");

  return c_Success;
}

/// @brief Display what type of the node is (Just for fun)
/// @param mode Type of the node
void HLTProcess::displayMode(const std::string mode)
{
  B2INFO("\x1b[33m++++++++++++++++++++++++++++++++++++++++++++++++\x1b[0m");
  B2INFO("");
  B2INFO("\x1b[33m           Running " << mode << "\x1b[0m");
  B2INFO("");
  B2INFO("\x1b[33m++++++++++++++++++++++++++++++++++++++++++++++++\x1b[0m");
}
