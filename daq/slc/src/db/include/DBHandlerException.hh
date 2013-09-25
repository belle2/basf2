#ifndef _B2DAQ_DBHandlerException_hh
#define _B2DAQ_DBHandlerException_hh

#include <util/IOException.hh>

namespace B2DAQ {

  class DBHandlerException : public IOException {

  public:
    DBHandlerException(const std::string& file_name, 
		const int line_no, 
		const std::string& comment) throw()
      : IOException(file_name, line_no, comment) {}
    
  };

}

#endif
