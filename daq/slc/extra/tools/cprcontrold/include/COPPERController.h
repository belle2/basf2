#ifndef _Belle2_COPPERController_h
#define _Belle2_COPPERController_h

#include <daq/slc/database/ConfigObject.h>
#include <daq/slc/base/IOException.h>

#include <mgt/mgt.h>

#include "copper_info.h"

namespace Belle2 {

  class COPPERController {

  public:
    COPPERController(): m_fd(-1) {}
    ~COPPERController() throw() {}

  public:
    bool open() throw();
    bool close() throw();
    int read(int request) throw(IOException);
    copper_info& monitor() throw();
    const copper_info& getInfo() const throw() {
      return m_info;
    }
    bool isFifoFull() throw();
    bool isFifoEmpty() throw();
    bool isLengthFifoFull() throw();

  private:
    int m_fd;
    copper_info m_info;

  };

}

#endif
