#include "HSLBController.h"

#include "mgt_control.h"

#include <base/HSLB.h>
#include <base/FEEModule.h>
#include <base/Debugger.h>
#include <base/StringUtil.h>

#include <cstdlib>
#include <cstdio>

using namespace Belle2;

HSLBController::HSLBController(int slot, HSLB* hslb)
  : _slot(slot), _hslb(hslb), _mgt(NULL) {}

HSLBController::~HSLBController() throw() {}

bool HSLBController::boot() throw()
{
  if (_hslb == NULL) return true;
  int board_type, firmware, hardware;
  if (_hslb->isUsed()) {
    const char* firmware_path = getenv("HSLB_FIRMEWATE_PATH");
    if (firmware_path == NULL) return false;
    const std::string path = Belle2::form("%s/%s", firmware_path,
                                          _hslb->getFirmware().c_str());
    Belle2::debug("[DEBUG] Firmware path = %s", path.c_str());
    _mgt = mgt_boot(_slot, path.c_str(),
                    &board_type, &firmware, &hardware);
    if (_mgt == NULL) return false;
  }
  return true;
}

bool HSLBController::reset() throw()
{
  if (_mgt != NULL) {
    // mgt_execute(_mgt, CTL_RESET_LINK);
    mgt_close(_mgt);
    _mgt = NULL;
    return true;
  } else {
    return true;
  }
}

bool HSLBController::load() throw()
{
  if (_hslb == NULL || !_hslb->isUsed() || _mgt == NULL) return true;
  FEEModule* module = _hslb->getFEEModule();
  if (module != NULL) {
    //mgt_execute(_mgt, CTL_LINK);
    if (mgt_check_FEE(_mgt) != 0) {
      printf("CHECK FEE ERROR\n");
      return false;
    }
    mgt_execute(_mgt, _hslb->getTriggerMode());
    //mgt_execute(_mgt, CTL_VERBOSE);
    mgt_execute(_mgt, CTL_TRIGGER);
    FEEModule::RegisterList& reg_v(module->getRegisters());
    for (size_t i = 0; i < reg_v.size(); i++) {
      FEEModule::Register& reg(reg_v[i]);
      for (size_t ch = 0; ch < reg.length(); ch++) {
        if (reg.getValue(ch) < 0) continue;
        int address = 0;
        if (reg.getSize() == 1) {
          address = reg.getAddress() + ch;
          mgt_set_param(_mgt, address, reg.getValue(ch));
        } else if (reg.getSize() == 2) {
          address = reg.getAddress() + ch * 2;
          mgt_set_param2(_mgt, address, reg.getValue(ch));
        }
        Belle2::debug("[DEBUG] Register write to address = 0x%x with value = %d",
                      address, reg.getValue(ch));
      }
    }
  }
  return true;
}

/*
bool HSLBController::start() throw()
{
  return true;
  if (_mgt != NULL && _hslb != NULL) {
    return mgt_execute(_mgt, _hslb->getTrgMode());
  } else {
    return true;
  }
}

bool HSLBController::stop() throw()
{
  return true;
  if (_mgt != NULL) {
    return mgt_execute(_mgt, CTL_HOLD_TRG);
  } else {
    return true;
  }
}
*/
