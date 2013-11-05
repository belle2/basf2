#include "HSLBController.h"

#include "xml/XMLParser.h"

#include "base/Debugger.h"
#include "base/StringUtil.h"

#include <mgt/mgt_control.h>

#include <cstdlib>
#include <cstdio>

using namespace Belle2;

HSLBController::HSLBController(int slot, DataObject* hslb)
  : _slot(slot), _hslb(hslb), _mgt(NULL)
{
  std::string path = getenv("B2SC_XML_PATH");
  path += "/" + hslb->getClassName() + ".xml";
  XMLParser parser;
  XMLElement* root = parser.parse(path);
  if (root->getTag() == "object" &&
      root->getAttribute("extends") == "HSLB") {
    std::vector<XMLElement*> el_v = root->getElements();
    for (size_t i = 0; i < el_v.size(); i++) {
      XMLElement* el = el_v[i];
      if (el->getTag() == "int") {
        HSLBRegister reg;
        reg.name = el->getAttribute("name");
        reg.address = strtoul(el->getAttribute("address").c_str(), 0, 0);
        reg.size = strtoul(el->getAttribute("size").c_str(), 0, 0);
        _reg_v.push_back(reg);
      } else if (el->getTag() == "int_array") {
        HSLBRegister reg;
        reg.name = el->getAttribute("name");
        reg.address = strtoul(el->getAttribute("address").c_str(), 0, 0);
        reg.size = strtoul(el->getAttribute("size").c_str(), 0, 0);
        int length = strtoul(el->getAttribute("length").c_str(), 0, 0);
        for (int l = 0; l < length; l++) {
          HSLBRegister reg_l = reg;
          reg_l.name = Belle2::form(reg.name + "_%d", l);
          reg_l.address += reg_l.size * l;
          _reg_v.push_back(reg_l);
        }
      }
    }
  }
  delete root;
}

HSLBController::~HSLBController() throw() {}

bool HSLBController::boot() throw()
{
  if (_hslb == NULL) return true;
  int board_type, firmware, hardware;
  const char* firmware_path = getenv("HSLB_FIRMEWATE_PATH");
  if (firmware_path == NULL) return false;
  const std::string path = Belle2::form("%s/%s", firmware_path,
                                        _hslb->getTextValue("firmware").c_str());
  Belle2::debug("[DEBUG] Firmware path = %s", path.c_str());
  _mgt = mgt_boot(_slot, path.c_str(),
                  &board_type, &firmware, &hardware);
  if (_mgt == NULL) return false;
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
  if (_hslb == NULL || _mgt == NULL) return true;
  //mgt_execute(_mgt, CTL_LINK);
  if (mgt_check_FEE(_mgt) != 0) {
    printf("[FATAL] Check FEE error\n");
    return false;
  }
  mgt_execute(_mgt, _hslb->getEnumValue("trigger_mode"));
  Belle2::debug("[DEBUG] Selected trigger mode = %d",
                _hslb->getEnumValue("trigger_mode"));
  //mgt_execute(_mgt, CTL_VERBOSE);
  mgt_execute(_mgt, CTL_TRIGGER);
  for (size_t i = 0; i < _reg_v.size(); i++) {
    HSLBRegister& reg(_reg_v[i]);
    int value = _hslb->getIntValue(reg.name);
    if (value < 0) continue;
    if (reg.size == 1) {
      mgt_set_param(_mgt, reg.address, value);
    } else if (reg.size == 2) {
      mgt_set_param2(_mgt, reg.address, value);
    }
    Belle2::debug("[DEBUG] Register write to address = 0x%x with value = %d",
                  reg.address, value);
  }
  return true;
}

