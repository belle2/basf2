
#include "COPPERNode.h"

#include "DataSender.h"
#include "HSLB.h"
#include "Host.h"
#include "FEEModule.h"

#include "StringUtil.h"
#include "Debugger.h"

#include <iostream>
#include <sstream>

using namespace Belle2;

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

int COPPERNode::getParams(const Command& command, unsigned int* pars,
                          std::string& datap)
{
  int npar = 0;
  std::stringstream ss; ss.str("");
  if (command == Command::BOOT) {
    pars[npar++] = getID();
    pars[npar++] = 0;
    ss << _sender->getHost() << " "
       << _sender->getScript();
    for (size_t i = 0; i < MAX_HSLBS; i++) {
      if (_hslb_v[i] != NULL) {
        pars[1] |= (0x01 & _hslb_v[i]->isUsed()) << i;
        ss << " " << _hslb_v[i]->getFirmware();
        FEEModule* module = _hslb_v[i]->getFEEModule();
        if (module != NULL) {
          FEEModule::RegisterList& reg_v(module->getRegisters());
          pars[npar++] = reg_v.size();
          for (size_t j = 0; j < reg_v.size(); j++) {
            FEEModule::Register& reg(reg_v[j]);
            pars[npar++] = reg.getSize();
            pars[npar++] = reg.getAddress();
            pars[npar++] = reg.length();
          }
        } else {
          pars[npar++] = 0;
        }
      } else {
        ss << " none";
      }
    }
  } else if (command == Command::LOAD) {
    for (size_t i = 0; i < 4; i++) {
      if (_hslb_v[i] != NULL) {
        FEEModule* module = _hslb_v[i]->getFEEModule();
        pars[npar++] = _hslb_v[i]->getTriggerMode();
        if (module != NULL) {
          FEEModule::RegisterList& reg_v(module->getRegisters());
          for (size_t j = 0; j < reg_v.size(); j++) {
            FEEModule::Register& reg(reg_v[j]);
            for (size_t ch = 0; ch < reg.length(); ch++) {
              pars[npar++] = reg.getValue(ch);
            }
          }
        }
      }
    }
  }
  datap = ss.str();
  return npar;
}

void COPPERNode::setParams(const Command& command,
                           int npar, const unsigned int* pars,
                           const std::string& datap)
{
  int par_i = 0;
  if (command == Command::BOOT) {
    par_i = 2;
    std::vector<std::string> str_v = Belle2::split(datap, ' ');
    setID(pars[0]);
    _sender->setHost(str_v[0]);
    _sender->setScript(str_v[1]);
    for (size_t i = 0; i < MAX_HSLBS; i++) {
      if (_hslb_v[i] == NULL) {
        _hslb_v[i] = new HSLB();
        _hslb_v[i]->setFEEModule(new FEEModule());
      }
      _hslb_v[i]->setUsed((pars[1] >> i) & 0x01);
      _hslb_v[i]->setFirmware(str_v[i + 2]);
      size_t nreg = pars[par_i++];
      FEEModule* module = _hslb_v[i]->getFEEModule();
      module->clearRegisters();
      for (size_t j = 0; j < nreg; j++) {
        FEEModule::Register reg;
        reg.setSize(pars[par_i++]);
        reg.setAddress(pars[par_i++]);
        reg.setLength(pars[par_i++]);
        module->addRegister(reg);
      }
    }
  } else if (command == Command::LOAD) {
    for (size_t i = 0; i < MAX_HSLBS; i++) {
      if (_hslb_v[i] != NULL && _hslb_v[i]->isUsed()) {
        FEEModule* module = _hslb_v[i]->getFEEModule();
        _hslb_v[i]->setTriggerMode(pars[par_i++]);
        if (module != NULL) {
          FEEModule::RegisterList& reg_v(module->getRegisters());
          for (size_t j = 0; j < reg_v.size(); j++) {
            FEEModule::Register& reg(reg_v[j]);
            for (size_t ch = 0; ch < reg.length(); ch++) {
              reg.setValue(ch, pars[par_i++]);
            }
          }
        }
      }
    }
  }
}
