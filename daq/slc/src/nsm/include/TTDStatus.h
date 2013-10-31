#ifndef _Belle2_TTDStatus_hh
#define _Belle2_TTDStatus_hh

#include "NSMData.h"

namespace Belle2 {

  class TTDStatus : public NSMData {

  public:
    TTDStatus(const std::string& data_name) throw();
    virtual ~TTDStatus() throw();

  public:
    virtual void read(NSMNode* node) throw(NSMHandlerException);
    virtual void write(NSMNode* node) throw(NSMHandlerException);
    virtual void serialize(NSMNode* node, int& npar, int* pars, std::string& data) throw(IOException);
    virtual void deserialize(NSMNode* node, int npar, int* pars, const std::string& data) throw(IOException);
    int getEventNumber();
    int getEventTotal();
    bool isRunning();

  private:
    void* _status;

  };

}

#endif
