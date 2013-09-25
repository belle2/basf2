#ifndef _B2DAQ_TTDData_hh
#define _B2DAQ_TTDData_hh

#include "NSMData.hh"

namespace B2DAQ {

  class TTDData : public NSMData {

  public:
    TTDData(const std::string& data_name) throw();
    TTDData() throw() : NSMData() {}
    virtual ~TTDData() throw() {}

  public:
    virtual void read(NSMNode* node) throw(NSMHandlerException);
    virtual void write(NSMNode* node) throw(NSMHandlerException);


  };

}

#endif
