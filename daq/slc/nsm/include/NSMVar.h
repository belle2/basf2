#ifndef _Belle2_NSMVar_h
#define _Belle2_NSMVar_h

#include <daq/slc/base/Serializable.h>
#include <daq/slc/base/Date.h>

#include <vector>
#include <string>

namespace Belle2 {

  class NSMVar : public Serializable {

  public:
    enum Type {
      NONE = 0,
      INT,
      FLOAT,
      TEXT,
    };

  public:
    static const NSMVar NOVALUE;

  public:
    NSMVar() : m_value(NULL), m_name(), m_type(NONE), m_len(0)
    {
      setDate(Date());
    }
    NSMVar(const std::string& name, Type type, int len, const void* value)
      : m_value(NULL)
    {
      copy(name, type, len, value);
    }
    NSMVar(const std::string& name, const std::string& value)
      : m_value(NULL)
    {
      copy(name, TEXT, value.size(), value.c_str());
    }
    NSMVar(const std::string& name, int value)
      : m_value(NULL)
    {
      copy(name, INT, 0, &value);
    }
    NSMVar(const std::string& name, float value)
      : m_value(NULL)
    {
      copy(name, FLOAT, 0, &value);
    }
    NSMVar(const std::string& name, int len, int* value)
      : m_value(NULL)
    {
      copy(name, INT, len, value);
    }
    NSMVar(const std::string& name, int len, float* value)
      : m_value(NULL)
    {
      copy(name, FLOAT, len, value);
    }
    NSMVar(const std::string& name, const std::vector<int>& value);
    NSMVar(const std::string& name, const std::vector<float>& value);
    NSMVar(const std::string& name) : m_value(NULL), m_name(name), m_type(NONE), m_len(0) {}
    NSMVar(const NSMVar& var) : m_value(NULL) { *this = var; }
    ~NSMVar();

  public:
    const NSMVar& operator=(const NSMVar& var)
    {
      copy(var.m_name, var.m_type, var.m_len, var.m_value, var.m_id, var.m_date);
      m_node = var.m_node;
      m_date = var.m_date;
      return *this;
    }
    const NSMVar& operator=(int val)
    {
      copy(m_name, INT, 0, &val, m_id);
      return *this;
    }
    const NSMVar& operator=(float val)
    {
      copy(m_name, FLOAT, 0, &val, m_id);
      return *this;
    }
    const NSMVar& operator=(const std::string& val)
    {
      if (val.size() > 0) {
        copy(m_name, TEXT, val.size(), val.c_str(), m_id);
      } else {
        copy(m_name, TEXT, 1, "", m_id);
      }
      return *this;
    }
    const NSMVar& operator=(const std::vector<int>& val);
    const NSMVar& operator=(const std::vector<float>& val);
    const NSMVar& operator>>(int& val) const { val = getInt(); return *this; }
    const NSMVar& operator>>(float& val) const { val = getFloat(); return *this; }
    const NSMVar& operator>>(std::string& val) const { val = getText(); return *this; }
    const NSMVar& operator>>(std::vector<int>& val) const;
    const NSMVar& operator>>(std::vector<float>& val) const;

  public:
    void setNode(const std::string& node) { m_node = node; }
    void setName(const std::string& name) { m_name = name; }
    const void* get() const { return m_value; }
    void* get() { return m_value; }
    int size() const;
    const std::string& getNode() const { return m_node; }
    const std::string& getName() const { return m_name; }
    Type getType() const { return m_type; }
    const char* getTypeLabel() const;
    int getLength() const { return m_len; }
    int getInt() const;
    float getFloat() const;
    const char* getText() const;
    int getInt(int i) const;
    float getFloat(int i) const;
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    void setDate(int date) { m_date = date; }
    void setDate(const Date& date) { m_date = date.get(); }
    int getDate() const { return m_date; }

  public:
    virtual void readObject(Reader&);
    virtual void writeObject(Writer&) const;

  public:
    void copy(const std::string& name, Type type, int len,
              const void* value, int id = 0, int date = 0);

  private:
    void* m_value;
    std::string m_node;
    std::string m_name;
    Type m_type;
    int m_len;
    int m_id;
    int m_date;

  };

  typedef std::vector<NSMVar> NSMVarList;

}

#endif
