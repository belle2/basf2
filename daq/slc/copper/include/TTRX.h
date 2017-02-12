#ifndef _Belle2_TTRX_h
#define _Belle2_TTRX_h

#include <string>

#include <ttrx/ttrx.h>
#include <ttrx/tt4r.h>

namespace Belle2 {

  typedef tt4r_t ttrx_info;

  class TTRX {

  public:
    static const char* getTTRXType(int type);

  public:
    TTRX() : m_ttrx(NULL) {}
    ~TTRX() throw() {}

  public:
    bool open() throw();
    bool isOpened() const throw() { return m_ttrx != NULL; }
    bool close() throw();
    bool boot(const std::string& file) throw();
    ttrx_info& monitor() throw();
    const ttrx_info& getInfo() const throw()
    {
      return m_info;
    }
    ttrx_info& getInfo() throw()
    {
      return m_info;
    }
    bool isError() throw()
    {
      return isBelle2LinkError() || isLinkUpError();
    }
    bool isBelle2LinkError() throw();
    bool isLinkUpError() throw();
    unsigned int read(unsigned int addr) throw();
    int write(unsigned int addr, unsigned int val) throw();

  private:
    ttrx_t* m_ttrx;
    ttrx_info m_info;

  };

}

#endif
