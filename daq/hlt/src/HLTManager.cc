#include <daq/hlt/HLTManager.h>

// temporary headers
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <daq/hlt/B2Socket.h>

using namespace Belle2;

HLTManager::HLTManager()
{
  m_nodeInfoMap.clear();
  m_senders.clear();
  m_controlBuffers.clear();

  m_isChild = false;
}

HLTManager::~HLTManager()
{
  if (!isChild()) {
    for (std::vector<int>::const_iterator i = m_controlBuffers.begin();
         i != m_controlBuffers.end(); ++i) {
      RingBuffer* buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(*i)).c_str(), gBufferSize);
      delete buffer;
    }
  }
}

EHLTStatus HLTManager::initSenders()
{
  for (std::map<int, NodeInfo>::const_iterator i = m_nodeInfoMap.begin();
       i != m_nodeInfoMap.end(); ++i) {
    pid_t pid = fork();
    RingBuffer* buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(c_ControlPort + (*i).first)).c_str(), gBufferSize);

    if (pid == 0) {
      m_isChild = true;

      NodeInfo nodeinfo;
      nodeinfo = (*i).second;

      HLTSender sender(nodeinfo.selfIP(), c_ControlPort);
      sender.createConnection();
      sender.setBuffer(c_ControlPort + (*i).first);

      std::string temp = encodeNodeInfo((*i).first);
      B2INFO("[HLTFramework] nodeinfo = " << temp.c_str());
      while (buffer->insq((int*)temp.c_str(), (temp.size()) / 4 + 1) <= 0) {
        B2INFO("\x1b[31m[HLTReceiver] Ring buffer overflow. Retrying...");
        usleep(100);
      }

      //sender.broadcasting();
      sender.broadcasting(temp);
      //sender.broadcasting("Terminate");
      sender.broadcasting(gTerminate);

      return c_ChildSuccess;
    } else {
      m_senders.push_back(pid);
      m_controlBuffers.push_back(c_ControlPort + (*i).first);
      B2INFO("\x1b[33m[HLTManager] EvtSender " << pid << " forked\x1b[0m");
    }
  }

  return c_Success;
}

EHLTStatus HLTManager::storeNodeInfo(XMLParser* xml)
{
  std::vector<int> mapKeys;
  xml->getAllKeys(mapKeys);

  // nodeinfo allocator
  for (std::vector<int>::const_iterator i = mapKeys.begin();
       i != mapKeys.end(); ++i) {
    if (m_nodeInfoMap.insert(std::pair<int, NodeInfo>(*i, NodeInfo())).second == false) {
      B2ERROR("Failed to initialize node information in the map!");
      return c_FuncError;
    }
  }

  for (std::map<int, NodeInfo>::const_iterator i = m_nodeInfoMap.begin();
       i != m_nodeInfoMap.end(); ++i) {
    int key = (*i).first;
    NodeInfo tempNodeinfo((*i).second);

    xml->fill(key, tempNodeinfo);

    m_nodeInfoMap[key] = tempNodeinfo;
  }

  return c_Success;
}

std::string HLTManager::encodeNodeInfo(unsigned int key)
{
  std::stringstream ss;

  boost::archive::text_oarchive oa(ss);
  oa << m_nodeInfoMap[key];

  return ss.str();
}

void HLTManager::decodeNodeInfo(std::string nodeinfo)
{
  NodeInfo tempNodeinfo;
  std::stringstream sNodeinfo;
  sNodeinfo << nodeinfo;

  boost::archive::binary_iarchive ia(sNodeinfo);
  ia >> tempNodeinfo;

  tempNodeinfo.display();
}

bool HLTManager::isChild()
{
  return m_isChild;
}

EHLTStatus HLTManager::checkChildren()
{
  int status;
  pid_t pid;

  for (std::vector<pid_t>::const_iterator i = m_senders.begin();
       i != m_senders.end(); ++i) {
    pid = waitpid(*i, &status, 0);
    B2INFO("[HLTManager] \x1b[33mHLTSender " << pid << " finished! (exit code="
           << status << ")\x1b[0m");
  }

  return c_Success;
}

void HLTManager::printNodeInfo()
{
  for (std::map<int, NodeInfo>::const_iterator i = m_nodeInfoMap.begin();
       i != m_nodeInfoMap.end(); ++i) {
    NodeInfo nodeinfo((*i).second);
    B2INFO("NodeInfo: [" << nodeinfo.type() << "] (" << nodeinfo.selfIP()
           << ") Exp#=" << nodeinfo.expNo() << " run#=" << nodeinfo.runStart()
           << "~" << nodeinfo.runEnd());
    B2INFO("          key=" << (*i).first << " unit#=" << nodeinfo.unitNo()
           << " node#=" << nodeinfo.nodeNo()
           << " manager=" << nodeinfo.managerIP());
    B2INFO("          sources (" << nodeinfo.sourceIP().size() << "):");
    for (unsigned int j = 0; j < nodeinfo.sourceIP().size(); ++j)
      B2INFO("                    " << nodeinfo.sourceIP()[j]);
    B2INFO("          targets (" << nodeinfo.targetIP().size() << "):");
    for (unsigned int j = 0; j < nodeinfo.targetIP().size(); ++j)
      B2INFO("                    " << nodeinfo.targetIP()[j]);
  }
}
