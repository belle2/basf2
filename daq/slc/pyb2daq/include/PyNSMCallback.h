#ifndef _Belle2_PyNSMCallback_h
#define _Belle2_PyNSMCallback_h

#include <daq/slc/nsm/NSMCallback.h>

#include <boost/python.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/ref.hpp>

namespace Belle2 {

  class PyNSMCallback {

  public:
    PyNSMCallback() {}
    ~PyNSMCallback() {}

  public:
    void setCallback(NSMCallback* callback)
    {
      m_callback = callback;
    }

  public:
    virtual void run(const char* confname);
    virtual void init() {}
    virtual void term() {}
    virtual void timeout() {}

  public:
    void addInt(const char* name, int val) throw(NSMHandlerException)
    {
      m_callback->add(new NSMVHandlerInt(name, true, true, val));
    }
    void addFloat(const char* name, float val) throw(NSMHandlerException)
    {
      m_callback->add(new NSMVHandlerFloat(name, true, true, val));
    }
    void addText(const char* name, const char* val) throw(NSMHandlerException)
    {
      m_callback->add(new NSMVHandlerText(name, true, true, val));
    }
    void setInt(const char* name, int val) throw(NSMHandlerException)
    {
      m_callback->set(name, val);
    }
    void setFloat(const char* name, float val) throw(NSMHandlerException)
    {
      m_callback->set(name, val);
    }
    void setText(const char* name, const char* val) throw(NSMHandlerException)
    {
      m_callback->set(name, val);
    }
    int getInt(const char* name) throw(NSMHandlerException)
    {
      int val = 0;
      m_callback->get(name, val);
      return val;
    }
    float getFloat(const char* name) throw(NSMHandlerException)
    {
      float val = 0;
      m_callback->get(name, val);
      return val;
    }
    const char* getText(const char* name) throw(NSMHandlerException)
    {
      static std::string val;
      m_callback->get(name, val);
      return val.c_str();
    }
    void setNodeInt(const char* node, const char* name, int val) throw(NSMHandlerException)
    {
      m_callback->set(NSMNode(node), name, val);
    }
    void setNodeFloat(const char* node, const char* name, float val) throw(NSMHandlerException)
    {
      m_callback->set(NSMNode(node), name, val);
    }
    void setNodeText(const char* node, const char* name, const char* val) throw(NSMHandlerException)
    {
      m_callback->set(NSMNode(node), name, val);
    }
    int getNodeInt(const char* node, const char* name) throw(NSMHandlerException)
    {
      int val = 0;
      m_callback->get(NSMNode(node), name, val);
      return val;
    }
    float getNodeFloat(const char* node, const char* name) throw(NSMHandlerException)
    {
      float val = 0;
      m_callback->get(NSMNode(node), name, val);
      return val;
    }
    const char* getNodeText(const char* node, const char* name) throw(NSMHandlerException)
    {
      static std::string val;
      m_callback->get(NSMNode(node), name, val);
      return val.c_str();
    }
    void log(LogFile::Priority pri, const char* msg) throw(NSMHandlerException)
    {
      m_callback->log(pri, msg);
    }

  protected:
    NSMCallback* m_callback;

  };

  class PyNSMCallbackImp : public NSMCallback {

  public:
    PyNSMCallbackImp(PyNSMCallback* py) : m_py(py) {}
    virtual ~PyNSMCallbackImp() throw() {}

  public:
    virtual void init(NSMCommunicator&) throw()
    {
      m_py->init();
    }
    virtual void term() throw()
    {
      m_py->term();
    }
    virtual void timeout(NSMCommunicator&) throw()
    {
      m_py->timeout();
    }

  protected:
    PyNSMCallback* m_py;
  };


  class PyNSMCallbackWrapper : public PyNSMCallback,
    public boost::python::wrapper<PyNSMCallback> {

  public:
    virtual void init();
    virtual void term();
    virtual void timeout();

  };

}

#endif
