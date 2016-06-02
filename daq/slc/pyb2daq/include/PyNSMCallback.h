#ifndef _Belle2_PyNSMCallback_h
#define _Belle2_PyNSMCallback_h

#include <daq/slc/nsm/NSMCallback.h>

namespace Belle2 {

  class PyNSMCallback {

  private:
    static NSMCallback* g_callback;

  public:
    static void setCallback(NSMCallback* callback)
    {
      g_callback = callback;
    }

  public:
    void addInt(const char* name, int val)
    {
      if (g_callback) {
        g_callback->add(new NSMVHandlerInt(name, true, true, val));
      }
    }
    void addFloat(const char* name, float val)
    {
      if (g_callback) {
        g_callback->add(new NSMVHandlerFloat(name, true, true, val));
      }
    }
    void addText(const char* name, const char* val)
    {
      if (g_callback) {
        g_callback->add(new NSMVHandlerText(name, true, true, val));
      }
    }
    void setInt(const char* name, int val) { _set(name, val); }
    void setFloat(const char* name, float val) { _set(name, val); }
    void setText(const char* name, const char* val)
    {
      if (g_callback) {
        g_callback->set(name, val);
      }
    }
    void log(LogFile::Priority pri, const char* msg)
    {
      if (g_callback) {
        g_callback->log(pri, msg);
      }
    }

  private:
    template<typename T> void _set(const char* name, T val)
    {
      if (g_callback) {
        g_callback->set(name, val);
      }
    }
  };

}

#endif
