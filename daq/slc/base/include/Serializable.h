#ifndef _Belle2_Serializable_hh
#define _Belle2_Serializable_hh

namespace Belle2 {

  class Reader;
  class Writer;

  class Serializable {

  public:
    virtual ~Serializable() {}

  public:
    virtual void readObject(Reader&) = 0;
    virtual void writeObject(Writer&) const = 0;

  };

}

#endif
