#ifndef _Belle2_ObjectListTamplate_h
#define _Belle2_ObjectListTamplate_h

#include <daq/slc/base/Serializable.h>
#include <daq/slc/base/Writer.h>
#include <daq/slc/base/Reader.h>

#include <string>
#include <vector>

namespace Belle2 {

  template <class OBJ>
  class ObjectListTemplate : public Serializable {

  public:
    ObjectListTemplate() : m_configid(0) {}
    ObjectListTemplate(const std::string& name, int id = 0)
      : m_configid(id), m_configname(name) {}
    virtual ~ObjectListTemplate() throw() {}

  public:
    void resize(size_t nobj = 0) throw() { m_obj_v = std::vector<OBJ>(nobj); }
    OBJ& operator[](int i) throw() { return m_obj_v[i]; }
    const OBJ& operator[](int i) const throw() { return m_obj_v[i]; }
    void push_back(const OBJ& obj) throw() { m_obj_v.push_back(obj); }
    size_t size() const throw() { return m_obj_v.size(); }
    typename std::vector<OBJ>::iterator begin() throw() { return m_obj_v.begin(); }
    typename std::vector<OBJ>::iterator end() throw() { return m_obj_v.end(); }
    int getConfigId() const throw() { return m_configid; }
    void setConfigId(int id) throw() { m_configid = id; }
    const std::string& getConfigName() const throw() { return m_configname; }
    void setConfigName(const std::string& name) throw() { m_configname = name; }
    const std::string& getNode() const throw() { return m_node; }
    void setNode(const std::string& node) throw() { m_node = node; }
    const std::string& getTable() const throw() { return m_table; }
    void setTable(const std::string& table) throw() { m_table = table; }

  public:
    virtual void writeObject(Writer& writer) const throw(IOException) {
      writer.writeInt(m_configid);
      writer.writeString(m_configname);
      writer.writeString(m_node);
      writer.writeString(m_table);
      writer.writeInt(m_obj_v.size());
      for (size_t i = 0; i < m_obj_v.size(); i++) {
        writer.writeObject(m_obj_v[i]);
      }
    }
    virtual void readObject(Reader& reader) throw(IOException) {
      m_configid = reader.readInt();
      m_configname = reader.readString();
      m_node = reader.readString();
      m_table = reader.readString();
      m_obj_v.resize(reader.readInt());
      for (size_t i = 0; i < m_obj_v.size(); i++) {
        reader.readObject(m_obj_v[i]);
      }
    }

  private:
    int m_configid;
    std::string m_configname;
    std::string m_node;
    std::string m_table;
    mutable std::vector<OBJ> m_obj_v;

  };

}

#endif
