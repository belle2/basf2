#include "COPPERNode.hh"

#include "DataSender.hh"
#include "HSLB.hh"
#include "Host.hh"

#include <util/StringUtil.hh>

#include <sstream>

using namespace B2DAQ;

COPPERNode::COPPERNode(const std::string& name)
  : NSMNode(name, "copper_node"), _sender()
{
  _hslb_v[0] = _hslb_v[1] = _hslb_v[2] = _hslb_v[3] = NULL;
  _sender = new DataSender();
}

COPPERNode::~COPPERNode() throw()
{
  delete _sender;
}

const std::string COPPERNode::getSQLFields() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLFields();
  for (size_t i = 0; i < MAX_HSLBS; i++) {
    ss << ", hslb_id_" << i << " smallint";
  }
  ss << _sender->getSQLFields();
  return ss.str();
}

const std::string COPPERNode::getSQLLabels() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLLabels();
  for (size_t i = 0; i < MAX_HSLBS; i++) {
    ss << ", hslb_id_" << i;
  }
  ss << _sender->getSQLLabels();
  return ss.str();
}

const std::string COPPERNode::getSQLValues() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLValues();
  for (size_t i = 0; i < MAX_HSLBS; i++) {
    int id = (_hslb_v[i] != NULL) ? _hslb_v[i]->getID() : -1;
    ss << ", " << id;
  }
  ss << _sender->getSQLValues();
  return ss.str();
}

