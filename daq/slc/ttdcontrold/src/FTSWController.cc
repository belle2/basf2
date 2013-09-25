#include "FTSWController.hh"

#include <node/FTSW.hh>
#include <node/FEEModule.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

#include <cstdio>

using namespace B2DAQ;

FTSWController::FTSWController(FTSW* ftsw)
  : _ftsw(ftsw), _ftsw_fd(NULL)
{
}

FTSWController::~FTSWController() throw() {}

bool FTSWController::boot() throw()
{
  if (_ftsw == NULL) return true;

  if (_ftsw->isUsed()) {
    if (!(_ftsw_fd = ftsw_open(_ftsw->getChannel(), FTSW_RDWR))) {
      B2DAQ::debug("Failed to open VME register: channel = %d", _ftsw->getChannel());
      return false;
    }
    char path[512];
    sprintf(path, "/export/b2daq/ttdd/ftsw/%s", _ftsw->getFirmware().c_str());
    int verbose  = 0;
    int forced   = 0;
    int m012 = M012_SELECTMAP;
    if (ftsw_boot_fpga(_ftsw_fd, path, verbose, forced, m012)) {
      B2DAQ::debug("Failed to trigger: channel = %d", _ftsw->getChannel());
      return false;
    }
  }
  return true;
}

bool FTSWController::reboot() throw()
{
  if (_ftsw_fd != NULL) {
    if (ftsw_close(_ftsw_fd) < 0) {
      B2DAQ::debug("Failed to close VME register: channel = %d", _ftsw->getChannel());
    }
    return boot();
  }
  return true;
}

bool FTSWController::load() throw()
{
  if (_ftsw_fd != NULL) {
    /* tempolary defined. should be replaced with correct one */
    if (ftsw_stop_trigger(_ftsw_fd) < 0) {
      B2DAQ::debug("Failed to trigger: channel = %d", _ftsw->getChannel());
      return false;
    }
    /* -------------------------------------------------------*/
  }
  return true;
}

bool FTSWController::reload() throw()
{
  if (_ftsw_fd != NULL) {
    /* tempolary defined. should be replaced with correct one */
    if (ftsw_stop_trigger(_ftsw_fd) < 0) {
      B2DAQ::debug("Failed to trigger: channel = %d", _ftsw->getChannel());
      return false;
    }
    /* -------------------------------------------------------*/
    return load();
  }
  return true;
}

bool FTSWController::start() throw()
{
  if (_ftsw_fd != NULL) {
    return true;
  }
  return true;
}

bool FTSWController::stop() throw()
{
  if (_ftsw_fd != NULL) {
    return true;
  }
  return true;
}

bool FTSWController::resume() throw()
{
  if (_ftsw_fd != NULL) {
    return true;
  }
  return true;
}

bool FTSWController::pause() throw()
{
  if (_ftsw_fd != NULL) {
    return true;
  }
  return true;
}

bool FTSWController::recover() throw()
{
  if (_ftsw_fd != NULL) {
    return true;
  }
  return true;
}

bool FTSWController::abort() throw()
{
  if (_ftsw_fd != NULL) {
    return true;
  }
  return true;
}
