#ifndef _Belle2_FTSWController_hh
#define _Belle2_FTSWController_hh

#include <daq/slc/base/DataObject.h>

#include <daq/slc/system/PThread.h>

#include <ftsw.h>

namespace Belle2 {

  class FTSWController {

  public:
    FTSWController(DataObject* ftsw = NULL);
    virtual ~FTSWController() throw();

  public:
    void setFTSW(FTSW* ftsw) { _ftsw = ftsw; }

  public:
    virtual bool boot() throw();
    virtual bool reboot() throw();
    virtual bool load() throw();
    virtual bool reload() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();

  private:
    DataObject* _ftsw;
    ftsw_t* _ftsw_fd;
    PThread _thread;

  };

}

#endif
