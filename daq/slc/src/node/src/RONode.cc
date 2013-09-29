#include "RONode.hh"

#include <util/StringUtil.hh>

#include <sstream>
#include <iostream>

using namespace B2DAQ;

void RONode::clearSenders() throw()
{
  _sender_i = 0;
  _sender_v = std::vector<DataSender*>(MAX_SENDERS);
}

void RONode::addSender(DataSender* sender) throw()
{
  if (_sender_i < MAX_SENDERS) {
    _sender_v[_sender_i] = sender;
    _sender_i++;
  }
}

const std::string RONode::getSQLFields() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLFields()
     << ", script text";
  for (size_t i = 0; i < MAX_SENDERS; i++) {
    ss << ", sender_id_" << i << " smallint";
  }
  return ss.str();
}

const std::string RONode::getSQLLabels() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLLabels()
     << ", script";
  for (size_t i = 0; i < MAX_SENDERS; i++) {
    ss << ", sender_id_" << i;
  }
  return ss.str();
}

const std::string RONode::getSQLValues() const throw()
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

int RONode::getParams(const Command& command, unsigned int* pars,
                      std::string& datap)
{
  int npar = 0;
  pars[npar++] = _sender_i;
  std::stringstream ss; ss.str("");
  if (command == Command::BOOT) {
    ss << _script << " ";
    for (size_t i = 0; i < _sender_i; i++) {
      ss << _sender_v[i]->getHost() << " ";
    }
  } else if (command == Command::LOAD) {
  }
  datap = ss.str();
  return npar;
}

void RONode::setParams(const Command& command, int npar,
                       const unsigned int* pars,
                       const std::string& datap)
{
  int par_i = 0;
  if (command == Command::BOOT) {
    std::vector<std::string> str_v = B2DAQ::split(datap, ' ');
    _sender_i = pars[par_i++];
    _script = str_v[0];
    for (size_t i = 0; i < _sender_i; i++) {
      if (_sender_v[i] == NULL) {
        _sender_v[i] = new DataSender();
      }
      _sender_v[i]->setHost(str_v[i + 1]);
    }
  } else if (command == Command::LOAD) {

  }
}
