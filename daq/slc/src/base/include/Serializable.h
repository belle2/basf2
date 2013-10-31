#ifndef _Belle2_Serializable_hh
#define _Belle2_Serializable_hh

#include "IOException.h"

namespace Belle2 {

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
