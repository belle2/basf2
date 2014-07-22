#ifndef _Belle2_TTRXController_h
#define _Belle2_TTRXController_h

#include <daq/slc/database/ConfigObject.h>
#include <daq/slc/base/IOException.h>

#include <ttrx/ttrx.h>
#include <ttrx/tt4r.h>

namespace Belle2 {

  typedef tt4r_t ttrx_info;

  class TTRXController {

  public:
    static const char* getTTRXType(int type);

  public:
    TTRXController() : m_ttrx(NULL) {}
    ~TTRXController() throw() {}

  public:
    bool open() throw();
    bool isOpened() const throw() { return m_ttrx != NULL; }
    bool close() throw();
    bool boot(const std::string& file) throw();
    ttrx_info& monitor() throw();
    const ttrx_info& getInfo(int i) const throw() {
      return m_info;
    }
    ttrx_info& getInfo(int i) throw() {
      return m_info;
    }
    bool isError() throw() {
      return isBelle2LinkError() || isLinkUpError();
    }
    bool isBelle2LinkError() throw();
    bool isLinkUpError() throw();

  private:
    ttrx_t* m_ttrx;
    ttrx_info m_info;

  };

}

#endif
