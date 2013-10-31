#include "NodeSystem.h"

#include "StringUtil.h"

#include <sstream>

using namespace Belle2;

const int NodeSystem::MODE_GLOBAL = 1;
const int NodeSystem::MODE_LOCAL = 0;

NodeSystem::~NodeSystem() throw()
{
  for (std::vector<Host*>::iterator it = _host_v.begin();
       it != _host_v.end(); it++) {
    delete *it;
  }
  for (std::vector<FEEModule*>::iterator it = _module_v.begin();
       it != _module_v.end(); it++) {
    delete *it;
  }
  for (std::vector<HSLB*>::iterator it = _hslb_v.begin();
       it != _hslb_v.end(); it++) {
    delete *it;
  }
  for (std::vector<NSMNode*>::iterator it = _node_v.begin();
       it != _node_v.end(); it++) {
    delete *it;
  }
  for (std::vector<NodeGroup*>::iterator it = _group_v.begin();
       it != _group_v.end(); it++) {
    delete *it;
  }
}

void NodeSystem::setVersion(const std::string& label, int version)
{
  if (label == "host") {
    for (std::vector<Host*>::iterator it = _host_v.begin();
         it != _host_v.end(); it++) {
      (*it)->setVersion(version);
    }
  } else if (label == "copper_node") {
    for (std::vector<COPPERNode*>::iterator it = _copper_v.begin();
         it != _copper_v.end(); it++) {
      (*it)->setVersion(version);
    }
  } else if (label == "hslb") {
    for (std::vector<HSLB*>::iterator it = _hslb_v.begin();
         it != _hslb_v.end(); it++) {
      (*it)->setVersion(version);
    }
  } else if (label == "ttd_node") {
    for (std::vector<TTDNode*>::iterator it = _ttd_v.begin();
         it != _ttd_v.end(); it++) {
      (*it)->setVersion(version);
    }
  } else if (label == "ftsw") {
    for (std::vector<FTSW*>::iterator it = _ftsw_v.begin();
         it != _ftsw_v.end(); it++) {
      (*it)->setVersion(version);
    }
  } else if (label == "ro_node") {
    for (std::vector<RONode*>::iterator it = _ro_v.begin();
         it != _ro_v.end(); it++) {
      (*it)->setVersion(version);
    }
  } else {
    for (std::vector<FEEModule*>::iterator it = _module_v_m[label].begin();
         it != _module_v_m[label].end(); it++) {
      (*it)->setVersion(version);
    }
  }
}

void NodeSystem::addNodeGroup(NodeGroup* group)
{
  _group_v.push_back(group);
  std::vector<NSMNode*>& node_v(group->getNodes());
  for (std::vector<NSMNode*>::iterator it = node_v.begin();
       it != node_v.end(); it++) {
    _node_v.push_back(*it);
  }
}

const std::string NodeSystem::getSQLFields() const throw()
{
  std::stringstream ss;
  ss << "version int, date timestamp, "
     << "operation_mode int, run_type text, description text, "
     << "hosts_ver int, copper_node_ver int, hslb_ver int, ttd_node_ver int, "
     << "ftsw_ver int, ro_node_ver int";
  for (std::map<std::string, std::vector<FEEModule*> >::iterator
       it = _module_v_m.begin(); it != _module_v_m.end(); it++) {
    ss << ", " << it->first << "_ver int";
  }
  return ss.str();
}

const std::string NodeSystem::getSQLLabels() const throw()
{
  std::stringstream ss;
  ss << "version, date, "
     << "operation_mode, run_type, description, "
     << "hosts_ver, copper_node_ver, hslb_ver, ttd_node_ver, "
     << "ftsw_ver, ro_node_ver";
  for (std::map<std::string, std::vector<FEEModule*> >::iterator
       it = _module_v_m.begin(); it != _module_v_m.end(); it++) {
    ss << ", " << it->first << "_ver";
  }
  return ss.str();
}

const std::string NodeSystem::getSQLValues() const throw()
{
  std::stringstream ss;
  ss << _version << ", current_timestamp, " << _operation_mode << ", '"
     << _run_type << "', '" << Belle2::replace(_description, "'", "^!^") << "', "
     << ((_host_v.size() > 0) ? _host_v[0]->getVersion() : 0) << ", "
     << ((_copper_v.size() > 0) ? _copper_v[0]->getVersion() : 0) << ", "
     << ((_hslb_v.size() > 0) ? _hslb_v[0]->getVersion() : 0) << ", "
     << ((_ttd_v.size() > 0) ? _ttd_v[0]->getVersion() : 0) << ", "
     << ((_ftsw_v.size() > 0) ? _ftsw_v[0]->getVersion() : 0) << ", "
     << ((_ro_v.size() > 0) ? _ro_v[0]->getVersion() : 0);
  for (std::map<std::string, std::vector<FEEModule*> >::iterator it = _module_v_m.begin();
       it != _module_v_m.end(); it++) {
    ss  << ", " << ((it->second.size() > 0) ? it->second[0]->getVersion() : 0);
  }
  ss << "";
  return ss.str();
}
