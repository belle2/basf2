#ifndef _Belle2_COPPERCallback_hh
#define _Belle2_COPPERCallback_hh

#include "HSLBController.h"

#include "daq/slc/readout/ROController.h"

#include "daq/slc/nsm/RCCallback.h"

namespace Belle2 {

  class ProcessListener;
  class XMLElement;

  class COPPERCallback : public RCCallback {

  public:
    COPPERCallback(NSMNode* node, const std::string& rc_config);
    virtual ~COPPERCallback() throw();

  public:
    virtual void init() throw();
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool abort() throw();

  private:
    std::string _rc_config;
    ROController _con;
    HSLBController _hslbcon_v[4];
    int _confno;
    XMLElement* _el_v[4];

  };

}

#endif
