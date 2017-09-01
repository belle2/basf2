#ifndef _Belle2_ECLShaperControllerHandler_h
#define _Belle2_ECLShaperControllerHandler_h

namespace Belle2 {

  class ECLShaperStatusHandler : public NSMVHandlerText {

  public:
    ECLShaperStatusHandler(ECLShaperControllerCallback& callback,
                           const std::string& name, int sh_num)
      : NSMVHandlerText(name, true, false),
        m_callback(callback), m_sh_num(sh_num) {}
    virtual ~ECLShaperStatusHandler() throw() {}

  public:
    bool handleGetText(std::string& val)
    {
      try {
        val = m_callback.col_status(m_sh_num);
        return true;
      } catch (const RCHandlerException& e) {
        LogFile::error(e.what());
      }
      return false;
    }

  private:
    ECLShaperControllerCallback& m_callback;
    int m_sh_num;
  };

  class ECLShaperBootHandler : public NSMVHandlerInt {

  public:
    ECLShaperBootHandler(ECLShaperControllerCallback& callback,
                         const std::string& name)
      : NSMVHandlerInt(name, false, true),
        m_callback(callback) {}
    virtual ~ECLShaperBootHandler() throw() {}

  public:
    bool handleSetInt(int val)
    {
      try {
        m_callback.boot(m_callback.getDBObject());
        return true;
      } catch (const RCHandlerException& e) {
        LogFile::error(e.what());
      }
      return false;
    }

  private:
    ECLShaperControllerCallback& m_callback;

  };

}

#endif
