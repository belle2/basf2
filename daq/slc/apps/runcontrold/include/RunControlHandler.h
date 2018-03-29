#ifndef _Belle2_RunControlHandler_h
#define _Belle2_RunControlHandler_h

#include "daq/slc/nsm/NSMVHandler.h"
#include "daq/slc/runcontrol/RCNode.h"

#define RCHANDLER_TEXT_GET(CLASS)         \
  class CLASS : public NSMVHandlerText, HandlerRC {     \
  public:                 \
    CLASS(RunControlCallback& callback,          \
          const std::string& name, RCNode& node)       \
      : NSMVHandlerText(name, true, false),        \
        HandlerRC(callback, node) {}         \
    virtual ~CLASS() throw() {}            \
    virtual bool handleGetText(std::string& val);        \
  }

#define RCHANDLER_TEXT(CLASS)           \
  class CLASS : public NSMVHandlerText, HandlerRC {     \
  public:                 \
    CLASS(RunControlCallback& callback,          \
          const std::string& name, RCNode& node)       \
      : NSMVHandlerText(name, true, true),         \
        HandlerRC(callback, node) {}         \
    virtual ~CLASS() throw() {}            \
    virtual bool handleGetText(std::string& val);        \
    virtual bool handleSetText(const std::string& val);      \
  }

#define RCHANDLER_INT(CLASS)            \
  class CLASS : public NSMVHandlerInt, HandlerRC {      \
  public:                 \
    CLASS(RunControlCallback& callback,          \
          const std::string& name, RCNode& node)      \
      : NSMVHandlerInt(name, true, true),       \
        HandlerRC(callback, node) {}         \
    virtual ~CLASS() throw() {}            \
    virtual bool handleGetInt(int& val);         \
    virtual bool handleSetInt(int val);          \
  }

namespace Belle2 {

  class RunControlCallback;

  class HandlerRC {
  public:
    HandlerRC(RunControlCallback& callback, RCNode& node)
      : m_callback(callback), m_rcnode(node) {}
    virtual ~HandlerRC() throw() {}
  protected:
    RunControlCallback& m_callback;
    RCNode& m_rcnode;
  };

  RCHANDLER_TEXT(NSMVHandlerRCConfig);
  RCHANDLER_TEXT_GET(NSMVHandlerRCState);
  RCHANDLER_TEXT(NSMVHandlerRCRequest);
  RCHANDLER_INT(NSMVHandlerRCNodeUsed);

  class NSMVHandlerRCExpNumber : public NSMVHandlerInt {
  public:
    NSMVHandlerRCExpNumber(RunControlCallback& callback,
                           const std::string& name, int val)
      : NSMVHandlerInt(name, true, true, val),
        m_callback(callback) {}
    virtual ~NSMVHandlerRCExpNumber() throw() {}
    virtual bool handleSetInt(int val);
  protected:
    RunControlCallback& m_callback;
  };

  class NSMVHandlerRCUsed : public NSMVHandlerInt {
  public:
    NSMVHandlerRCUsed(RunControlCallback& callback,
                      const std::string& name, bool used)
      : NSMVHandlerInt(name, true, true, (int)used),
        m_callback(callback) {}
    virtual ~NSMVHandlerRCUsed() throw() {}
    virtual bool handleSetInt(int val);
  private:
    RunControlCallback& m_callback;
  };

  class NSMVHandlerRCGlobalAll : public NSMVHandlerInt {
  public:
    NSMVHandlerRCGlobalAll(RunControlCallback& callback,
                           const std::string& name)
      : NSMVHandlerInt(name, true, true, (int)1),
        m_callback(callback) {}
    virtual ~NSMVHandlerRCGlobalAll() throw() {}
    virtual bool handleSetInt(int val);
  private:
    RunControlCallback& m_callback;
  };
}

#endif
