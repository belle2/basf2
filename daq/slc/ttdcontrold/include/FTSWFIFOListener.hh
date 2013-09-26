#ifndef _B2DAQ_FTSWFIFOListener_hh
#define _B2DAQ_FTSWFIFOListener_hh

#include <ftsw.h>

namespace B2DAQ {

  class FTSWFIFOListener {

  public:
    FTSWFIFOListener(ftsw_t* ftsw_fd)
      : _ftsw_fd(ftsw_fd) {}
    ~FTSWFIFOListener() throw() {}

  public:
    void run();
    
  private:
    ftsw_t* _ftsw_fd;

  };

}

#endif

