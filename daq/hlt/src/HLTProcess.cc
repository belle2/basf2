#include <daq/hlt/HLTProcess.h>

using namespace Belle2;

HLTProcess::HLTProcess()
{
  m_isChild = false;

  m_controlInBuffer = NULL;
  m_controlOutBuffer = NULL;
  m_dataInBuffer = NULL;
  m_dataOutBuffer = NULL;
}

HLTProcess::~HLTProcess()
{
  if (!m_isChild) {
    delete m_controlInBuffer;
    //delete m_controlOutBuffer;
    delete m_dataInBuffer;
    delete m_dataOutBuffer;
  }
}

EHLTStatus HLTProcess::initControl()
{
  B2INFO("[HLTProcess] \x1b[32mRing buffers initializing for control/monitor line\x1b[0m");
  m_controlInBuffer = new RingBuffer(c_ControlPort, gBufferSize);
  //m_controlOutBuffer = new RingBuffer (c_MonitorPort, gBufferSize);

  pid_t pid = fork();
  if (pid == 0) {
    m_isChild = true;

    HLTReceiver receiver(c_ControlPort);
    receiver.createConnection();
    receiver.setBuffer();

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

EHLTStatus HLTProcess::initSenders()
{
  m_dataOutBuffer = new RingBuffer(c_DataOutPort, gBufferSize);

  for (unsigned int i = 0; i < m_nodeInfo.targetIP().size(); ++i) {
    pid_t pidHLTSender = fork();
    if (pidHLTSender == 0) {
      m_isChild = true;

      int port = c_DataOutPort;
      if (m_nodeInfo.type() == "WN")
        port = c_DataInPort;

      HLTSender hltSender(m_nodeInfo.targetIP()[i], port);
      hltSender.createConnection();
      hltSender.setBuffer();

      while (1) {
        if (hltSender.broadcasting() == c_TermCalled)
          break;
      }

      m_dataOutBuffer = NULL;

      return c_ChildSuccess;
    } else {
      m_HLTSenders.push_back(pidHLTSender);
      B2INFO("\x1b[33m[HLTProcess] HLTSender " << pidHLTSender << " forked\x1b[0m");
      return c_Success;
    }
  }

  return c_Success;
}

EHLTStatus HLTProcess::initReceivers()
{
  m_dataInBuffer = new RingBuffer(c_DataInPort, gBufferSize);

  for (unsigned int i = 0; i < m_nodeInfo.sourceIP().size(); ++i) {
    pid_t pidHLTReceiver = fork();
    if (pidHLTReceiver == 0) {
      m_isChild = true;

      int port = c_DataInPort;
      if (m_nodeInfo.type() == "WN")
        port = c_DataOutPort;

      HLTReceiver hltReceiver(port);
      hltReceiver.createConnection();
      hltReceiver.setBuffer();

      hltReceiver.listening();

      return c_ChildSuccess;
    } else {
      m_HLTReceivers.push_back(pidHLTReceiver);
      B2INFO("\x1b[33m[HLTProcess] HLTReceiver " << pidHLTReceiver << " forked\x1b[0m");
      return c_Success;
    }
  }

  return c_Success;
}

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
      B2INFO("[HLTProcess] Hi, I'm event merger!");
    }
  } else {
    B2INFO("\x1b[33m[HLTProcess] basf2 " << pidProcess << " forked\x1b[0m");
    m_Process = pidProcess;
  }

  return c_Success;
}

bool HLTProcess::isChild()
{
  return m_isChild;
}

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
  pid = waitpid(m_Process, &status, 0);
  B2INFO("[HLTProcess] \x1b[33mbasf2 " << pid << " finished! (exit code="
         << status << ")\x1b[0m");

  pid = waitpid(m_Control, &status, 0);
  B2INFO("[HLTProcess] \x1b[33mHLTReceiver (Contol) " << pid << " finished! (exit code="
         << status << ")\x1b[0m");

  return c_Success;
}

void HLTProcess::displayMode(const std::string mode)
{
  B2INFO("\x1b[33m++++++++++++++++++++++++++++++++++++++++++++++++\x1b[0m");
  B2INFO("");
  B2INFO("\x1b[33m           Running " << mode << "\x1b[0m");
  B2INFO("");
  B2INFO("\x1b[33m++++++++++++++++++++++++++++++++++++++++++++++++\x1b[0m");
}
