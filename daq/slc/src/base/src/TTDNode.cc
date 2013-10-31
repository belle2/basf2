#include "TTDNode.h"

#include "FTSW.h"

#include "StringUtil.h"

#include <sstream>

using namespace Belle2;

void TTDNode::clearFTSWs() throw()
{
  for (size_t i = 0; i < MAX_FTSWS; i++)
    _ftsw_v[i] = NULL;
  _ftsw_i = 0;
}

const std::string TTDNode::getSQLFields() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLFields();
  for (size_t i = 0; i < MAX_FTSWS; i++) {
    ss << ", ftsw_id_" << i << " smallint";
  }
  return ss.str();
}

const std::string TTDNode::getSQLLabels() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLLabels();
  for (size_t i = 0; i < MAX_FTSWS; i++) {
    ss << ", ftsw_id_" << i;
  }
  return ss.str();
}

const std::string TTDNode::getSQLValues() const throw()
{
  std::stringstream ss;
  ss << NSMNode::getSQLValues();
  for (size_t i = 0; i < MAX_FTSWS; i++) {
    int id = (i < _ftsw_i && _ftsw_v[i] != NULL) ? _ftsw_v[i]->getID() : -1;
    ss << ", " << id;
  }
  return ss.str();
}

int TTDNode::getParams(const Command& command, unsigned int* pars,
                       std::string& datap)
{
  int npar = 0;
  std::stringstream ss; ss.str("");
  if (command == Command::BOOT) {
    pars[npar++] = _ftsw_i;
    pars[npar++] = 0;
    for (size_t i = 0; i < _ftsw_i; i++) {
      if (_ftsw_v[i] != NULL) {
        pars[1] |= _ftsw_v[i]->isUsed() << i;
        pars[npar++] =  _ftsw_v[i]->getChannel();
        ss << _ftsw_v[i]->getFirmware() << " ";
      }
    }
  } else if (command == Command::LOAD) {
  } else if (command == Command::TRIGFT) {
    for (size_t i = 0; i < _ftsw_i; i++) {
      if (_ftsw_v[i] != NULL) {
        pars[npar++] = _ftsw_v[i]->getTriggerMode();
        pars[npar++] = _ftsw_v[i]->getDummyRate();
        pars[npar++] = _ftsw_v[i]->getTriggerLimit();
      }
    }
  }
  datap = ss.str();
  return npar;
}

void TTDNode::setParams(const Command& command, int npar,
                        const unsigned int* pars,
                        const std::string& datap)
{
  int par_i = 0;
  if (command == Command::BOOT) {
    _ftsw_i = pars[par_i++];
    par_i++;
    std::vector<std::string> str_v = Belle2::split(datap, ' ');
    for (size_t i = 0; i < _ftsw_i; i++) {
      if (_ftsw_v[i] == NULL) {
        _ftsw_v[i] = new FTSW();
      }
      _ftsw_v[i]->setUsed(0x01 & (pars[1] >> i));
      _ftsw_v[i]->setChannel(pars[par_i++]);
      _ftsw_v[i]->setFirmware(str_v[i]);
    }
  } else if (command == Command::LOAD) {
  } else if (command == Command::TRIGFT) {
    for (size_t i = 0; i < _ftsw_i; i++) {
      if (_ftsw_v[i] != NULL) {
        _ftsw_v[i]->setTriggerMode(pars[par_i++]);
        _ftsw_v[i]->setDummyRate(pars[par_i++]);
        _ftsw_v[i]->setTriggerLimit(pars[par_i++]);
      }
    }
  }
}

