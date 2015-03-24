#ifndef _Belle2_COPPER_h
#define _Belle2_COPPER_h

#include "daq/slc/copper/copper_info.h"

#include <daq/slc/base/IOException.h>

namespace Belle2 {

  class COPPER {

  public:
    COPPER(): m_fd(-1) {}
    ~COPPER() throw() {}

  public:
    bool open() throw();
    bool close() throw();
    int read(int request) throw(IOException);
    copper_info& monitor() throw(IOException);
    void print() throw();
    const copper_info& getInfo() const throw()
    {
      return m_info;
    }
    bool isOpened() throw() { return m_fd > 0; }
    bool isFifoFull() throw();
    bool isFifoEmpty() throw();
    bool isLengthFifoFull() throw();
    bool isError() throw()
    {
      return isFifoFull() || isLengthFifoFull();
    }

  private:
    int m_fd;
    copper_info m_info;

  };

}

#endif
