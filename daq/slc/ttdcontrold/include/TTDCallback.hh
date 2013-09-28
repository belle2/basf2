#ifndef _B2DAQ_TTDCallback_hh
#define _B2DAQ_TTDCallback_hh

#include "FTSWController.hh"

#include <runcontrol/RCCallback.hh>

#include <nsm/RunStatus.hh>

#include <node/TTDNode.hh>

#include <system/Fork.hh>

namespace B2DAQ {

  class RunStatus;
  class TTDData;

  class TTDCallback : public RCCallback {

  public:
    TTDCallback(TTDNode* node = NULL, 
		TTDData* _data = NULL);
    virtual ~TTDCallback() throw();

  public:
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool abort() throw();

  private:
    RunStatus* _status;
    TTDNode* _node;
    TTDData* _data;
    std::vector<FTSWController> _ftswcon_v;
    Fork _fork;
    
  };

}

#endif
