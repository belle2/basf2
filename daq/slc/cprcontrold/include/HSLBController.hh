#ifndef _B2DAQ_HSLBController_hh
#define _B2DAQ_HSLBController_hh

#include <node/HSLB.hh>

#include <mgt.h>

namespace B2DAQ {

  class HSLBController {

  public:
    HSLBController(int slot = -1, HSLB* hslb = NULL);
    virtual ~HSLBController() throw();

  public:
    void setHSLB(int slot, HSLB* hslb) {
      _slot = slot;
      _hslb = hslb;
    }

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
    int _slot;
    HSLB* _hslb;
    Mgt_t* _mgt;
    
  };

}

#endif
