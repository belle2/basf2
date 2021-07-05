/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_AbstractNSMCallback_hh
#define _Belle2_AbstractNSMCallback_hh

#include "daq/slc/nsm/Callback.h"
#include "daq/slc/nsm/NSMMessage.h"
#include "daq/slc/nsm/NSMNode.h"
#include "daq/slc/nsm/NSMCommand.h"

#include <string>
#include <vector>

namespace Belle2 {

  class NSMCommunicator;
  typedef std::map<std::string, NSMNode> NSMNodeMap;
  typedef std::map<std::string, NSMNodeMap> NSMNodeMapMap;

  class AbstractNSMCallback : public Callback {

    friend class NSMCallback;

  public:
    AbstractNSMCallback(int timeout);
    virtual ~AbstractNSMCallback() {}

  public:
    int addDB(const DBObject& obj);
    bool get(const NSMNode& node, const std::string& name, int& val, int timeout = 5);
    bool get(const NSMNode& node, const std::string& name, float& val, int timeout = 5);
    bool get(const NSMNode& node, const std::string& name, std::string& val, int timeout = 5);
    bool get(const NSMNode& node, const std::string& name, std::vector<int>& val, int timeout = 5);
    bool get(const NSMNode& node, const std::string& name, std::vector<float>& val, int timeout = 5);
    bool set(const NSMNode& node, const std::string& name, int val, int timeout = 5);
    bool set(const NSMNode& node, const std::string& name, float val, int timeout = 5);
    bool set(const NSMNode& node, const std::string& name, const std::string& val, int timeout = 5);
    bool set(const NSMNode& node, const std::string& name, const std::vector<int>& val, int timeout = 5);
    bool set(const NSMNode& node, const std::string& name, const std::vector<float>& val, int timeout = 5);
    bool get(const NSMNode& node, NSMVHandler* handler, int timeout = 5);
    bool get(DBObject& obj);
    bool get(const std::string& name, int& val) { return get("", name, val); }
    bool get(const std::string& name, float& val) { return get("", name, val); }
    bool get(const std::string& name, std::string& val) { return get("", name, val); }
    bool get(const std::string& name, std::vector<int>& val) { return get("", name, val); }
    bool get(const std::string& name, std::vector<float>& val) { return get("", name, val); }
    bool set(const std::string& name, int val) { return set("", name, val); }
    bool set(const std::string& name, float val) { return set("", name, val); }
    bool set(const std::string& name, const std::string& val) { return set("", name, val); }
    bool set(const std::string& name, const std::vector<int>& val) { return set("", name, val); }
    bool set(const std::string& name, const std::vector<float>& val) { return set("", name, val); }
    bool get(const std::string& node, const std::string& name, int& val) { return get_t(node, name, val); }
    bool get(const std::string& node, const std::string& name, float& val) { return get_t(node, name, val); }
    bool get(const std::string& node, const std::string& name, std::string& val) { return get_t(node, name, val); }
    bool get(const std::string& node, const std::string& name, std::vector<int>& val) { return get_t(node, name, val); }
    bool get(const std::string& node, const std::string& name, std::vector<float>& val) { return get_t(node, name, val); }
    bool set(const std::string& node, const std::string& name, int val) { return set_t(node, name, val); }
    bool set(const std::string& node, const std::string& name, float val) { return set_t(node, name, val); }
    bool set(const std::string& node, const std::string& name, const std::string& val) { return set_t(node, name, val); }
    bool set(const std::string& node, const std::string& name, const std::vector<int>& val) { return set_t(node, name, val); }
    bool set(const std::string& node, const std::string& name, const std::vector<float>& val) { return set_t(node, name, val); }
    int wait(double timeout = 5);
    NSMCommunicator& wait(const NSMNode& node, const NSMCommand& cmd,
                          double timeout = 5);
    virtual void notify(const NSMVar& var) = 0;
    bool try_wait();

  public:
    NSMNode& getNode() { return m_node; }
    const NSMNode& getNode() const { return m_node; }
    void setNode(const NSMNode& node) { m_node = node; }
    int getTimeout() const { return m_timeout; }
    void setTimeout(int timeout) { m_timeout = timeout; }

  protected:
    virtual bool perform(NSMCommunicator& com) = 0;
    void readVar(const NSMMessage& msg, NSMVar& var);

  private:
    NSMNode m_node;
    int m_timeout;
    NSMNodeMapMap m_node_v_m;

  public:
    bool get(const NSMNode& node, NSMVar& var, int timeout = 5) ;
    bool set(const NSMNode& node, const NSMVar& var, int timeout = 5);

  private:
    template<typename T>
    bool get_t(const NSMNode& node, const std::string& name,
               T& val, int timeout, NSMVar::Type type = NSMVar::NONE, int len = 0)
    {
      NSMVar var(name);
      var.setNode(node.getName());
      if (get(node, var, timeout)) {
        if (type == NSMVar::NONE ||
            (type == var.getType() &&
             ((len == 0 && var.getLength() == 0) ||
              (len > 0 && var.getLength() > 0)))) {
          var >> val;
          return true;
        }
      }
      return false;
    }
    template<typename T>
    bool get_t(const std::string& node, const std::string& name, T& val)
    {
      NSMVHandler* handler = getHandler_p(node, name);
      if (handler) {
        NSMVar var;
        handler->handleGet(var);
        var >> val;
        return true;
      }
      return false;
    }
    template<typename T>
    bool set_t(const std::string& node, const std::string& name, const T& val)
    {
      NSMVHandler* handler = getHandler_p(node, name);
      if (handler) {
        handler->set(val);
        if (node.size() == 0) {
          notify(handler->get());
        }
        return true;
      }
      return false;
    }
  };

};

#endif
