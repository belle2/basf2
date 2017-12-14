#ifndef _Belle2_DBObject_hh
#define _Belle2_DBObject_hh

#include <daq/slc/base/AbstractDBObject.h>
#include <daq/slc/base/StringUtil.h>

#include <ostream>
#include <stdexcept>

namespace Belle2 {

  class DBObject : public AbstractDBObject {

  public:
    typedef std::vector<DBObject> DBObjectList;
    typedef std::map<std::string, void*> FieldValueList;
    typedef std::map<std::string, std::string> FieldTextList;
    typedef std::map<std::string, DBObjectList> FieldObjectList;

  public:
    struct NameValue {
      std::string name;
      std::string value;
      void* buf;
      DBField::Type type;
    };
    typedef std::vector<NameValue> NameValueList;

  public:
    DBObject();
    DBObject(const std::string& path);
    DBObject(const DBObject& obj);
    virtual ~DBObject() throw();
    const DBObject& operator=(const DBObject& obj) throw();

  public:
    int getNObjects(const std::string& name) const throw();
    DBObjectList& getObjects(const std::string& name) throw(std::out_of_range);
    const DBObjectList& getObjects(const std::string& name) const throw(std::out_of_range);
    DBObject& getObject(const std::string& name, int i = 0) throw(std::out_of_range);
    const DBObject& getObject(const std::string& name, int i = 0) const throw(std::out_of_range);
    void addObjects(const std::string& name, const DBObjectList& obj) throw();
    void addObject(const std::string& name, const DBObject& obj) throw();

  public:
    DBObject& operator()(const std::string& name, int index = 0) throw(std::out_of_range)
    {
      return getObject(name, index);
    }
    const DBObject& operator()(const std::string& name, int index = 0) const throw(std::out_of_range)
    {
      return getObject(name, index);
    }

  public:
    StringList getNameList(bool isfull) const throw();
    void print(bool isfull = true) const throw();
    const std::string sprint(bool isfull) const throw();
    void printHTML(bool isfull = true) const throw();
    void printSQL(const std::string& table, std::ostream& out,
                  const std::string& name = "", int index = -1) const throw();
    void search(NameValueList& map, const std::string& name = "", bool isfull = true) const throw();

  public:
    virtual const void* getValue(const std::string& name) const throw(std::out_of_range);
    virtual const std::string& getText(const std::string& name) const throw(std::out_of_range);
    virtual void addText(const std::string& name, const std::string& value) throw();
    virtual void addValue(const std::string& name, const void* value,
                          DBField::Type type, int length) throw();
    virtual void setValue(const std::string& name, const void* value, int length) throw();

  public:
    virtual void readObject(Reader& reader) throw(IOException);
    virtual void writeObject(Writer& writer) const throw(IOException);

  private:
    FieldValueList m_value_m;
    FieldTextList m_text_m;
    FieldObjectList m_obj_v_m;
    std::string m_empty;

  protected:
    virtual void reset() throw();

  private:
    void copy(const DBObject& obj);

  };

  typedef std::vector<DBObject> DBObjectList;

}

#endif
