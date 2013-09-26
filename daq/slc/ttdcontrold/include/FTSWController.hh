#ifndef _B2DAQ_FTSWController_hh
#define _B2DAQ_FTSWController_hh

#include <node/FTSW.hh>

#include <system/PThread.hh>

#include <ftsw.h>

namespace B2DAQ {

  class FTSWController {

  public:
    FTSWController(FTSW* ftsw = NULL);
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
    FTSW* _ftsw;
    ftsw_t* _ftsw_fd;
    PThread _thread;

  };

}

#endif
