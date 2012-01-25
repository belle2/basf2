#ifndef NODEINFO_H
#define NODEINFO_H

#include <string>
#include <vector>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>

namespace Belle2 {
  class NodeInfo {
  public:
    NodeInfo();
    NodeInfo(const NodeInfo& nodeinfo);
    ~NodeInfo();

    NodeInfo& operator=(const NodeInfo& nodeinfo);

    void clear();

    void unitNo(int unitNo);
    const int unitNo();
    void nodeNo(int nodeNo);
    const int nodeNo();
    const int generateKey();

    void expNo(int expNo);
    const int expNo();
    void runStart(int runStart);
    const int runStart();
    void runEnd(int runEnd);
    const int runEnd();

    void type(std::string type);
    const std::string type();

    void managerIP(std::string managerIP);
    const std::string managerIP();
    void selfIP(std::string selfIP);
    const std::string selfIP();
    void sourceIP(std::string sourceIP);
    const std::vector<std::string> sourceIP();
    void targetIP(std::string targetIP);
    const std::vector<std::string> targetIP();

    void display();

  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, unsigned int ver) {
      ar& m_unitNo;
      ar& m_nodeNo;
      ar& m_expNo;
      ar& m_runStart;
      ar& m_runEnd;
      ar& m_type;
      ar& m_managerIP;
      ar& m_selfIP;
      ar& m_sourceIP;
      ar& m_targetIP;
    }

    int m_unitNo, m_nodeNo;
    int m_expNo, m_runStart, m_runEnd;

    std::string m_type;

    std::string m_managerIP;
    std::string m_selfIP;
    std::vector<std::string> m_sourceIP;
    std::vector<std::string> m_targetIP;
  };
}

#endif
