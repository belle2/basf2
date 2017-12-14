#ifndef _Belle2_AbstractNSMCallback_hh
#define _Belle2_AbstractNSMCallback_hh

#include "daq/slc/nsm/Callback.h"
#include "daq/slc/nsm/NSMHandlerException.h"
#include "daq/slc/nsm/NSMMessage.h"
#include "daq/slc/nsm/NSMNode.h"
#include "daq/slc/nsm/NSMCommand.h"

#include "daq/slc/base/TimeoutException.h"

#include <string>
#include <vector>
#include <typeinfo>

namespace Belle2 {

  class NSMCommunicator;
  typedef std::map<std::string, NSMNode> NSMNodeMap;
  typedef std::map<std::string, NSMNodeMap> NSMNodeMapMap;

  class AbstractNSMCallback : public Callback {

    friend class NSMCallback;

  public:
    AbstractNSMCallback(int timeout) throw();
    virtual ~AbstractNSMCallback() throw() {}

  public:
    int addDB(const DBObject& obj);
    bool get(const NSMNode& node, const std::string& name, int& val, int timeout = 5) throw(IOException);
    bool get(const NSMNode& node, const std::string& name, float& val, int timeout = 5) throw(IOException);
    bool get(const NSMNode& node, const std::string& name, std::string& val, int timeout = 5) throw(IOException);
    bool get(const NSMNode& node, const std::string& name, std::vector<int>& val, int timeout = 5) throw(IOException);
    bool get(const NSMNode& node, const std::string& name, std::vector<float>& val, int timeout = 5) throw(IOException);
    bool set(const NSMNode& node, const std::string& name, int val, int timeout = 5) throw(IOException);
    bool set(const NSMNode& node, const std::string& name, float val, int timeout = 5) throw(IOException);
    bool set(const NSMNode& node, const std::string& name, const std::string& val, int timeout = 5) throw(IOException);
    bool set(const NSMNode& node, const std::string& name, const std::vector<int>& val, int timeout = 5) throw(IOException);
    bool set(const NSMNode& node, const std::string& name, const std::vector<float>& val, int timeout = 5) throw(IOException);
    bool get(const NSMNode& node, NSMVHandler* handler, int timeout = 5) throw(IOException);
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
    int wait(double timeout = 5) throw(IOException);

  protected:
    NSMCommunicator& wait(const NSMNode& node, const NSMCommand& cmd,
                          double timeout = 5) throw(IOException);
    virtual void notify(const NSMVar& var) throw() = 0;
    bool try_wait() throw();

  public:
    NSMNode& getNode() throw() { return m_node; }
    const NSMNode& getNode() const throw() { return m_node; }
    void setNode(const NSMNode& node) throw() { m_node = node; }
    int getTimeout() const { return m_timeout; }
    void setTimeout(int timeout) { m_timeout = timeout; }

  protected:
    virtual bool perform(NSMCommunicator& com) throw() = 0;
    void readVar(const NSMMessage& msg, NSMVar& var);

  private:
    NSMNode m_node;
    int m_timeout;
    NSMNodeMapMap m_node_v_m;

  public:
    bool get(const NSMNode& node, NSMVar& var, int timeout = 5)  throw(IOException);
    bool set(const NSMNode& node, const NSMVar& var, int timeout = 5) throw(IOException);

  private:
    template<typename T>
    bool get_t(const NSMNode& node, const std::string& name,
               T& val, int timeout, NSMVar::Type type = NSMVar::NONE, int len = 0) throw(IOException)
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
      for (size_t i = 0; i < m_handler.size(); i++) {
        if (node == m_handler[i]->getNode() &&
            name == m_handler[i]->getName()) {
          NSMVar var;
          m_handler[i]->handleGet(var);
          var >> val;
          return true;
        }
      }
      return false;
    }
    template<typename T>
    bool set_t(const std::string& node, const std::string& name, const T& val)
    {
      for (size_t i = 0; i < m_handler.size(); i++) {
        if (node == m_handler[i]->getNode() &&
            name == m_handler[i]->getName()) {
          try {
            m_handler[i]->set(val);
            if (node.size() == 0) {
              m_handler[i]->get().setNode(getNode().getName());
              notify(m_handler[i]->get());
            }
            return true;
          } catch (const std::bad_cast& e) {
            return false;
          }
        }
      }
      return false;
    }
  };

};

#endif
