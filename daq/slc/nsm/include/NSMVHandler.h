#ifndef _Belle2_NSMVHandler_h
#define _Belle2_NSMVHandler_h

#include <daq/slc/nsm/NSMVar.h>

#include <vector>

namespace Belle2 {

  class NSMVHandler {

    friend class Callback;
    friend class AbstractNSMCallback;
    friend class NSMCallback;

  public:
    static NSMVHandler* create(const NSMVar& var);

  public:
    NSMVHandler(const std::string& node, const std::string& name,
                bool useget, bool useset)
      : m_id(0), m_timestamp(0), m_node(node), m_name(name),
        m_useget(useget), m_useset(useset), m_var(name) {}
    virtual ~NSMVHandler() throw() {}

  public:
    virtual bool handleGet(NSMVar& var)
    {
      var = m_var;
      return true;
    }
    virtual bool handleSet(const NSMVar& var)
    {
      m_var = var;
      return true;
    }

  public:
    const std::string& getNode() const { return m_node; }
    const std::string& getName() const { return m_name; }
    bool useGet() const { return m_useget; }
    bool useSet() const { return m_useset; }
    void setUseGet(bool use) { m_useget = use; }
    void setUseSet(bool use) { m_useset = use; }
    int getTimeStamp() const { return m_timestamp; }
    int getId() const { return m_id; }
    void setNode(const std::string& node) { m_node = node; }
    void setId(int id) { m_id = id; }
    void set(const NSMVar& var) { m_var = var; }
    void set(int val) { m_var = val; }
    void set(float val) { m_var = val; }
    void set(const std::string& val) { m_var = val; }
    void set(const std::vector<int>& val) { m_var = val; }
    void set(const std::vector<float>& val) { m_var = val; }
    const NSMVar& get() const { return m_var; }
    NSMVar& get() { return m_var; }
    NSMVar::Type getType() const { return m_var.getType(); }
    const char* getTypeLabel() const { return m_var.getTypeLabel(); }

  protected:
    int m_id;
    int m_timestamp;
    std::string m_node;
    std::string m_name;
    bool m_useget;
    bool m_useset;
    NSMVar m_var;

  };

  typedef std::vector<NSMVHandler*> NSMVHandlerList;

  class NSMVHandlerInt : public NSMVHandler {

  public:
    NSMVHandlerInt(const std::string& name,
                   bool useget, bool useset, int val = 0)
      : NSMVHandler("", name, useget, useset)
    {
      m_var = val;
    }
    NSMVHandlerInt(const std::string& node, const std::string& name,
                   bool useget, bool useset, int val = 0)
      : NSMVHandler(node, name, useget, useset)
    {
      m_var = val;
    }
    virtual ~NSMVHandlerInt() throw() {}

  public:
    virtual bool handleGet(NSMVar& var);
    virtual bool handleSet(const NSMVar& var);

  public:
    virtual bool handleGetInt(int& val)
    {
      val = m_var.getInt();
      return true;
    }
    virtual bool handleSetInt(int val)
    {
      m_var = val;
      return true;
    }

  };

  class NSMVHandlerFloat : public NSMVHandler {

  public:
    NSMVHandlerFloat(const std::string& name,
                     bool useget, bool useset, float val = 0)
      : NSMVHandler("", name, useget, useset)
    {
      m_var = val;
    }
    NSMVHandlerFloat(const std::string& node, const std::string& name,
                     bool useget, bool useset, float val = 0)
      : NSMVHandler(node, name, useget, useset)
    {
      m_var = val;
    }
    virtual ~NSMVHandlerFloat() throw() {}

  private:
    virtual bool handleGet(NSMVar& var);
    virtual bool handleSet(const NSMVar& var);

  public:
    virtual bool handleGetFloat(float& val)
    {
      val = m_var.getFloat();
      return true;
    }
    virtual bool handleSetFloat(float val)
    {
      m_var = val;
      return true;
    }

  };

  class NSMVHandlerText : public NSMVHandler {

  public:
    NSMVHandlerText(const std::string& name,
                    bool useget, bool useset, const std::string& val = "")
      : NSMVHandler("", name, useget, useset)
    {
      m_var = val;
    }
    NSMVHandlerText(const std::string& node, const std::string& name,
                    bool useget, bool useset, const std::string& val = "")
      : NSMVHandler(node, name, useget, useset)
    {
      m_var = val;
    }
    virtual ~NSMVHandlerText() throw() {}

  private:
    virtual bool handleGet(NSMVar& var);
    virtual bool handleSet(const NSMVar& var);

  public:
    virtual bool handleGetText(std::string& val)
    {
      val = m_var.getText();
      return true;
    }
    virtual bool handleSetText(const std::string& val)
    {
      m_var = val;
      return true;
    }

  };

  class NSMVHandlerIntArray : public NSMVHandler {

  public:
    NSMVHandlerIntArray(const std::string& name,
                        bool useget, bool useset,
                        const std::vector<int>& val = std::vector<int>())
      : NSMVHandler("", name, useget, useset)
    {
      m_var = val;
    }
    NSMVHandlerIntArray(const std::string& node, const std::string& name,
                        bool useget, bool useset,
                        const std::vector<int>& val = std::vector<int>())
      : NSMVHandler(node, name, useget, useset)
    {
      m_var = val;
    }
    virtual ~NSMVHandlerIntArray() throw() {}

  public:
    virtual bool handleGet(NSMVar& var);
    virtual bool handleSet(const NSMVar& var);

  public:
    virtual bool handleGetIntArray(std::vector<int>& val);
    virtual bool handleSetIntArray(const std::vector<int>& val)
    {
      m_var = val;
      return true;
    }

  };

  class NSMVHandlerFloatArray : public NSMVHandler {

  public:
    NSMVHandlerFloatArray(const std::string& name,
                          bool useget, bool useset,
                          const std::vector<float>& val = std::vector<float>())
      : NSMVHandler("", name, useset, useget)
    {
      m_var = val;
    }
    NSMVHandlerFloatArray(const std::string& node, const std::string& name,
                          bool useget, bool useset,
                          const std::vector<float>& val = std::vector<float>())
      : NSMVHandler(node, name, useset, useget)
    {
      m_var = val;
    }
    virtual ~NSMVHandlerFloatArray() throw() {}

  public:
    virtual bool handleGet(NSMVar& var);
    virtual bool handleSet(const NSMVar& var);

  public:
    virtual bool handleGetFloatArray(std::vector<float>& val);
    virtual bool handleSetFloatArray(const std::vector<float>& val)
    {
      m_var = val;
      return true;
    }

  };

  class NSMCallback;

  class NSMVHandlerRef : public NSMVHandler {

  public:
    NSMVHandlerRef(NSMCallback& callback,
                   const std::string& name,
                   const std::string& refname)
      : NSMVHandler("", name, true, true),
        m_callback(callback), m_refname(refname) {}
    virtual ~NSMVHandlerRef() throw() {}

  public:
    virtual bool handleGet(NSMVar& var);
    virtual bool handleSet(const NSMVar& var);

  private:
    NSMCallback& m_callback;
    std::string m_refname;

  };

};

#endif
