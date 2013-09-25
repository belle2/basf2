#ifndef _B2DAQ_DataReceiverData_hh
#define _B2DAQ_DataReceiverData_hh

#include "NSMData.hh"

namespace B2DAQ {

  class DataReceiverData : public NSMData {

  public:
    DataReceiverData(const std::string& data_name) throw();
    DataReceiverData() throw() : NSMData() {}
    virtual ~DataReceiverData() throw() {}

  public:
    virtual void read(NSMNode* node) throw(NSMHandlerException);
    virtual void write(NSMNode* node) throw(NSMHandlerException);


  };

}

#endif
