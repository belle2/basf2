#ifndef _Belle2_PyRCCallback_h
#define _Belle2_PyRCCallback_h

#include <daq/slc/pyb2daq/DAQDBObject.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <boost/python.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/ref.hpp>

namespace Belle2 {

  class PyRCCallback {

  public:
    PyRCCallback() {}
    ~PyRCCallback() {}

  public:
    void setCallback(RCCallback* callback)
    {
      m_callback = callback;
    }

  public:
    virtual void run(const char* confname);
    virtual void initialize(const DAQDBObject& db) {}
    virtual void boot(const DAQDBObject& db) {}
    virtual void load(const DAQDBObject& db) {}
    virtual void start(int expno, int runno) {}
    virtual void stop() {}
    virtual void recover(const DAQDBObject& db) {}
    virtual bool resume(int subno) { return true; }
    virtual bool pause() {return true; }
    virtual void abort() {}
    virtual void monitor() {}

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
    RCCallback* m_callback;

  };

  class PyRCCallbackImp : public RCCallback {

  public:
    PyRCCallbackImp(PyRCCallback* py) : m_py(py) {}
    virtual ~PyRCCallbackImp() throw() {}

  public:
    virtual void initialize(const DBObject& db) throw()
    {
      m_py->initialize(DAQDBObject(db));
    }
    virtual void boot(const DBObject& db) throw(RCHandlerException)
    {
      m_py->boot(DAQDBObject(db));
    }
    virtual void load(const DBObject& db) throw(RCHandlerException)
    {
      m_py->load(DAQDBObject(db));
    }
    virtual void start(int expno, int runno) throw(RCHandlerException)
    {
      m_py->start(expno, runno);
    }
    virtual void stop() throw(RCHandlerException)
    {
      m_py->stop();
    }
    virtual void recover(const DBObject& db) throw(RCHandlerException)
    {
      m_py->recover(DAQDBObject(db));
    }
    virtual bool resume(int subno) throw(RCHandlerException)
    {
      return m_py->resume(subno);
    }
    virtual bool pause() throw(RCHandlerException)
    {
      return m_py->pause();
    }
    virtual void abort() throw(RCHandlerException)
    {
      m_py->abort();
    }
    virtual void monitor() throw(RCHandlerException)
    {
      m_py->monitor();
    }

  protected:
    PyRCCallback* m_py;
  };


  class PyRCCallbackWrapper : public PyRCCallback,
    public boost::python::wrapper<PyRCCallback> {

  public:
    virtual void initialize(const DAQDBObject& db);
    virtual void boot(const DAQDBObject& db);
    virtual void load(const DAQDBObject& db);
    virtual void start(int expno, int runno);
    virtual void stop();
    virtual void recover(const DAQDBObject& db);
    virtual bool resume(int subno);
    virtual bool pause();
    virtual void abort();
    virtual void monitor();

  };

}

#endif
