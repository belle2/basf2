/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_DBObject_hh
#define _Belle2_DBObject_hh

#include <daq/slc/base/AbstractDBObject.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

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
    virtual ~DBObject();
    const DBObject& operator=(const DBObject& obj);

  public:
    int getNObjects(const std::string& name) const;
    DBObjectList& getObjects(const std::string& name);
    const DBObjectList& getObjects(const std::string& name) const;
    DBObject& getObject(const std::string& name, int i = 0);
    const DBObject& getObject(const std::string& name, int i = 0) const;
    void addObjects(const std::string& name, const DBObjectList& obj);
    void addObject(const std::string& name, const DBObject& obj);

  public:
    DBObject& operator()(const std::string& name, int index = 0)
    {
      return getObject(name, index);
    }
    const DBObject& operator()(const std::string& name, int index = 0) const
    {
      return getObject(name, index);
    }

  public:
    StringList getNameList(bool isfull) const;
    void print(bool isfull = true) const;
    const std::string sprint(bool isfull) const;
    void printHTML(bool isfull = true) const;
    const std::string printSQL(const std::string& table, int id) const;
    void search(NameValueList& map, const std::string& name = "", bool isfull = true) const;
    int getDate() const { return m_date; }
    void setDate(const Date& date) { m_date = date.get(); }
    void setDate(int date) { m_date = date; }

  public:
    virtual const void* getValue(const std::string& name) const;
    virtual const std::string& getText(const std::string& name) const;
    virtual void addText(const std::string& name, const std::string& value);
    virtual void addValue(const std::string& name, const void* value,
                          DBField::Type type, int length);
    virtual void setValue(const std::string& name, const void* value, int length);

  public:
    virtual void readObject(Reader& reader);
    virtual void writeObject(Writer& writer) const;

  private:
    FieldValueList m_value_m;
    FieldTextList m_text_m;
    FieldObjectList m_obj_v_m;
    std::string m_empty;
    int m_date;

  protected:
    virtual void reset();

  private:
    void copy(const DBObject& obj);

  };

  typedef std::vector<DBObject> DBObjectList;

}

#endif
