#ifndef _Belle2_SVDFTBFEE_h
#define _Belle2_SVDFTBFEE_h

#include <daq/slc/copper/FEE.h>

namespace Belle2 {

  class SVDFTBFEE : public FEE {

  public:
    SVDFTBFEE();
    virtual ~SVDFTBFEE() throw() {}

  public:
    virtual void init(RCCallback& callback, HSLB& hslb);
    virtual void boot(HSLB& hslb, const DBObject& obj);
    virtual void load(HSLB& hslb, const DBObject& obj);

    class NSMVHandlerInitRand : public NSMVHandlerInt {
    public:
      NSMVHandlerInitRand(const std::string& name)
        : NSMVHandlerInt(name, true, false, 0) {}
      virtual ~NSMVHandlerInitRand() throw() {}
      virtual bool handleSetInt(int val);
    };

  };

}

#endif
