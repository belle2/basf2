#include "HSLBController.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <mgt/mgt_control.h>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace Belle2;

HSLBController::HSLBController()
  : _mgt(NULL)
{

}

HSLBController::~HSLBController() throw() {}

bool HSLBController::reset() throw()
{
  if (_mgt != NULL) {
    mgt_close(_mgt);
    _mgt = NULL;
  }
  return true;
}

bool HSLBController::load(const ConfigObject& obj) throw()
{
  if (_mgt == NULL) {
    int board_type, firmware, hardware;
    const char* firm_path = NULL;
    _mgt = mgt_boot(obj.getIndex(), firm_path,
                    &board_type, &firmware, &hardware);
    if (_mgt == NULL) {
      LogFile::error("Failed to HSLB:%c", 'a' + obj.getIndex());
      return false;
    }
  }
  mgt_execute(_mgt, CTL_LINK);
  int triggermode = 0;
  if (obj.getEnum("triggermode") == "verbose") {
    triggermode = CTL_VERBOSE;
  } else if (obj.getEnum("triggermode") == "simple") {
    triggermode = CTL_SIMPLE;
  }
  if (triggermode > 0) {
    mgt_execute(_mgt, triggermode);
    LogFile::debug("Selected trigger mode = %d", triggermode);
  } else {
    LogFile::error("trigger mode was not selected");
    mgt_close(_mgt);
    return false;
  }
  int address = 0;
  int index = 0;
  if (obj.hasObject("register")) {
    const ConfigObjectList& obj_v(obj.getObjects("register"));
    for (size_t i = 0; i < obj_v.size(); i++) {
      const DBObject& cobj(obj_v[i]);
      LogFile::debug("Register name = %s", cobj.getText("name").c_str());
      int size = cobj.getShort("size");
      if (cobj.getInt("address") > 0) {
        address = cobj.getInt("address");
        index = 0;
      }
      if (address == 0) {
        LogFile::error("No address was found for %s",
                       cobj.getText("name").c_str());
        mgt_close(_mgt);
        return false;
      }
      int value = cobj.getInt("value");
      if (size == 1)  mgt_set_param(_mgt, address, value);
      else if (size == 2) mgt_set_param2(_mgt, address, value);
      address += size;
      LogFile::debug("Register write to address = 0x%x with value = %d",
                     address, value);
      index++;
      usleep(50000);
    }
  }
  return true;
}

