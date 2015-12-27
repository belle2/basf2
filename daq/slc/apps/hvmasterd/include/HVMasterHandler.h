#ifndef _Belle2_HVMasterHandler_h
#define _Belle2_HVMasterHandler_h

#include <daq/slc/nsm/NSMVHandler.h>

#include <daq/slc/hvcontrol/HVNode.h>

#define HVHANDLER_TEXT_GET(CLASS)         \
  class CLASS : public NSMVHandlerText, HandlerHV {     \
  public:                 \
    CLASS(HVMasterCallback& callback,          \
          const std::string& name, HVNode& node)       \
      : NSMVHandlerText(name, true, false),        \
        HandlerHV(callback, node) {}         \
    virtual ~CLASS() throw() {}            \
    virtual bool handleGetText(std::string& val);        \
  }

#define HVHANDLER_TEXT(CLASS)           \
  class CLASS : public NSMVHandlerText, HandlerHV {     \
  public:                 \
    CLASS(HVMasterCallback& callback,          \
          const std::string& name, HVNode& node)       \
      : NSMVHandlerText(name, true, true),         \
        HandlerHV(callback, node) {}         \
    virtual ~CLASS() throw() {}            \
    virtual bool handleGetText(std::string& val);        \
    virtual bool handleSetText(const std::string& val);      \
  }

#define HVHANDLER_INT(CLASS)            \
  class CLASS : public NSMVHandlerInt, HandlerHV {      \
  public:                 \
    CLASS(HVMasterCallback& callback,          \
          const std::string& name, HVNode& node)      \
      : NSMVHandlerInt(name, true, true),       \
        HandlerHV(callback, node) {}         \
    virtual ~CLASS() throw() {}            \
    virtual bool handleGetInt(int& val);         \
    virtual bool handleSetInt(int val);          \
  }

namespace Belle2 {

  class HVMasterCallback;

  class HandlerHV {
  public:
    HandlerHV(HVMasterCallback& callback, HVNode& node)
      : m_callback(callback), m_hvnode(node) {}
    virtual ~HandlerHV() throw() {}
  protected:
    HVMasterCallback& m_callback;
    HVNode& m_hvnode;
  };

  HVHANDLER_TEXT(NSMVHandlerHVConfig);
  HVHANDLER_TEXT_GET(NSMVHandlerHVNodeState);
  HVHANDLER_TEXT(NSMVHandlerHVRequest);
  HVHANDLER_INT(NSMVHandlerHVNodeUsed);

}

#endif
