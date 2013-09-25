#ifndef _B2DAQ_Serializable_hh
#define _B2DAQ_Serializable_hh

#include "IOException.hh"

namespace B2DAQ {
  
  class Reader;
  class Writer;

  class Serializable {
    
  public:
    virtual ~Serializable() throw() {}
    
  public:
    virtual void readObject(Reader&) throw(IOException) = 0;
    virtual void writeObject(Writer&) const throw(IOException) = 0;
    
  };
  
}

#endif
