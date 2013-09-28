#include "DataReceiverNode.hh"

#include <util/StringUtil.hh>

#include <sstream>
#include <iostream>

using namespace B2DAQ;

void DataReceiverNode::clearSenders() throw()
{
  _sender_i = 0;
  _sender_v = std::vector<DataSender*>(MAX_SENDERS);
}

void DataReceiverNode::addSender(DataSender* sender) throw()
{
  if (_sender_i < MAX_SENDERS) {
    _sender_v[_sender_i] = sender;
    _sender_i++;
  }
}

const std::string DataReceiverNode::getSQLFields() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLFields()
     << ", script text";
  for (size_t i = 0; i < MAX_SENDERS; i++) {
    ss << ", sender_id_" << i << " smallint";
  }
  return ss.str();
}

const std::string DataReceiverNode::getSQLLabels() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLLabels()
     << ", script";
  for (size_t i = 0; i < MAX_SENDERS; i++) {
    ss << ", sender_id_" << i;
  }
  return ss.str();
}

const std::string DataReceiverNode::getSQLValues() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLValues()
     << ", '" << _script << "'";
  for (size_t i = 0; i < MAX_SENDERS; i++) {
    if (i < _sender_i) {
      ss << "," << _sender_v[i]->getID();
    } else {
      ss << ", -1";
    }
  }
  return ss.str();
}
