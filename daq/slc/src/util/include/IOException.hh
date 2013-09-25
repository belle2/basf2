#ifndef _B2DAQ_IOException_hh
#define _B2DAQ_IOException_hh

#include "Exception.hh"

namespace B2DAQ {

  class IOException : public Exception {

  public:
    IOException(const std::string& file_name, 
		const int line_no, 
		const std::string& comment) throw()
      : Exception(file_name, line_no, comment) {}
    
  };

}

#endif
