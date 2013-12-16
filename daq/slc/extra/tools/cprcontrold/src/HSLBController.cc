#include "HSLBController.h"

#include "daq/slc/xml/XMLParser.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/ConfigFile.h"

#include <mgt/mgt_control.h>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace Belle2;

HSLBController::HSLBController()
  : _slot(-1), _hslb(NULL), _mgt(NULL), _boot_firm(false)
{

}

HSLBController::~HSLBController() throw() {}

bool HSLBController::boot(int slot, DataObject* hslb,
                          XMLElement* el) throw()
{
  if (hslb == NULL || el == NULL) {
    return true;
  } else {
    _slot = slot;
    _hslb = hslb;
    if (el->getTag() == "object" &&
        el->getAttribute("extends") == "HSLB") {
      std::vector<XMLElement*> el_v = el->getElements();
      for (size_t i = 0; i < el_v.size(); i++) {
        XMLElement* el = el_v[i];
        if (el->getTag() == "int" || el->getTag() == "int_array") {
          HSLBRegister reg;
          reg.name = el->getAttribute("name");
          reg.address = strtoul(el->getAttribute("address").c_str(), 0, 0);
          reg.size = strtoul(el->getAttribute("size").c_str(), 0, 0);
          Belle2::debug("name = %s, address = %x, size=%d",
                        reg.name.c_str(), reg.address, reg.size);
          _reg_v.push_back(reg);
        }
      }
    }
  }
  return true;
}

bool HSLBController::reset() throw()
{
  if (_mgt != NULL) {
    mgt_close(_mgt);
    _mgt = NULL;
    return true;
  } else {
    return true;
  }
}

bool HSLBController::load() throw()
{
  int board_type, firmware, hardware;
  ///*
  if (_boot_firm) {
    ConfigFile config;
    std::string path = _hslb->getText("firmware");
    if (path.size() > 0) path = config.get("HSLB_FIRMWARE_PATH") + "/" + path;
    Belle2::debug("%s:%d", __FILE__, __LINE__);
    const char* firm_path = (path.size() > 0) ? path.c_str() : NULL;
    _mgt = mgt_boot(_slot, firm_path, &board_type,  &firmware, &hardware);
    Belle2::debug("%s:%d", __FILE__, __LINE__);
    if (_mgt == NULL) return false;
  }
  if (_hslb == NULL || _mgt == NULL) return true;
  mgt_execute(_mgt, CTL_RESET_LINK);
  /*
  if (mgt_check_FEE(_mgt) != 0) {
    printf("[FATAL] Check FEE error\n");
    return false;
  }
  */
  if (_hslb == NULL) return true;
  int trigger_mode = _hslb->getEnum("trigger_mode");
  mgt_execute(_mgt, trigger_mode);
  Belle2::debug("[DEBUG] Selected trigger mode = %d", trigger_mode);
  for (size_t i = 0; i < _reg_v.size(); i++) {
    HSLBRegister& reg(_reg_v[i]);
    Belle2::debug("[DEBUG] Register write to name = %s", reg.name.c_str());
    size_t length = 0;
    int* value_v = _hslb->getIntArray(reg.name, length);
    if (value_v == NULL) continue;
    if (length == 0) length = 1;
    int address = reg.address;
    for (size_t i = 0; i < length; i++) {
      if (value_v[i] < 0) continue;
      if (reg.size == 1) {
        mgt_set_param(_mgt, address, value_v[i]);
      } else if (reg.size == 2) {
        mgt_set_param2(_mgt, address, value_v[i]);
      }
      address += reg.size;
      Belle2::debug("[DEBUG] Register write to address = 0x%x with value = %d",
                    address, value_v[i]);
      usleep(50000);
    }
  }
  return true;
}

