#include "FTSWController.h"

#include "FTSWFIFOListener.h"

#include <daq/slc/base/FTSW.h>
#include <daq/slc/base/FEEModule.h>

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>

#include <cstdlib>
#include <cstdio>
#include <sys/time.h>

using namespace Belle2;

FTSWController::FTSWController(FTSW* ftsw)
  : _ftsw(ftsw), _ftsw_fd(NULL)
{
}

FTSWController::~FTSWController() throw() {}

bool FTSWController::boot() throw()
{
  if (_ftsw == NULL) return true;

  if (_ftsw->isUsed()) {
    if (!(_ftsw_fd = open_ftsw(_ftsw->getChannel(), FTSW_RDWR))) {
      Belle2::debug("Failed to open VME register: channel = %d", _ftsw->getChannel());
      return false;
    }
    const char* firmware_path = getenv("FTSW_FIRMWARE_PATH");
    if (firmware_path == NULL) {
      Belle2::debug("FTSW_FIRMWARE_PATH is not set");
      return false;
    }
    char path[512];
    sprintf(path, "%s/%s", firmware_path,
            _ftsw->getFirmware().c_str());
    int verbose  = 0;
    int forced   = 0;
    int m012 = M012_SELECTMAP;
    if (boot_ftsw_fpga(_ftsw_fd, path, verbose, forced, m012)) {
      Belle2::debug("Failed to trigger: channel = %d", _ftsw->getChannel());
      return false;
    }
    int fpgaid  = read_ftsw(_ftsw_fd, FTSWREG_FPGAID);
    int fpgaver = read_ftsw(_ftsw_fd, FTSWREG_FPGAVER) & 0xffff;
    if ((fpgaid == 0x46543255 && fpgaver >= 17) ||  /* FT2U */
        (fpgaid == 0x46543355 && fpgaver >= 17)) {  /* FT3U */
      struct timeval tv;
      gettimeofday(&tv, 0);
      int sec = tv.tv_sec;
      for (int n = 0; ; n++) {
        gettimeofday(&tv, 0);
        if (sec != tv.tv_sec) {
          write_ftsw(_ftsw_fd, FTSWREG_SETUTIM, tv.tv_sec);
          printf("utime is set, n=%d sec=%ld usec=%ld\n",
                 n, tv.tv_sec, tv.tv_usec);
          break;
        }
      }
    }
  }
  return true;
}

bool FTSWController::reboot() throw()
{
  if (_ftsw_fd != NULL) {
    if (close_ftsw(_ftsw_fd) < 0) {
      Belle2::debug("Failed to close VME register: channel = %d", _ftsw->getChannel());
    }
    return boot();
  }
  return true;
}

bool FTSWController::load() throw()
{
  if (_ftsw_fd != NULL) {
    /* tempolary defined. should be replaced with correct one */
    if (stop_ftsw_trigger(_ftsw_fd) < 0) {
      Belle2::debug("Failed to trigger: channel = %d", _ftsw->getChannel());
      return false;
    }
    /* -------------------------------------------------------*/
    _thread = PThread(new FTSWFIFOListener(_ftsw_fd));
  }
  return true;
}

bool FTSWController::reload() throw()
{
  if (_ftsw_fd != NULL) {
    _thread.cancel();
    /* tempolary defined. should be replaced with correct one */
    if (stop_ftsw_trigger(_ftsw_fd) < 0) {
      Belle2::debug("Failed to trigger: channel = %d", _ftsw->getChannel());
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
