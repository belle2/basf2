#ifndef _Belle2_FieldInfo_h
#define _Belle2_FieldInfo_h

#include <string>

namespace Belle2 {

  class DBRecord;
  class DBObject;

  class FieldInfo {

  public:
    enum Type {
      BOOL = 1, CHAR, SHORT, INT, LONG,
      FLOAT, DOUBLE, TEXT, ENUM, OBJECT,
      NSM_CHAR, NSM_INT16, NSM_INT32, NSM_INT64,
      NSM_BYTE8, NSM_UINT16, NSM_UINT32,
      NSM_UINT64, NSM_FLOAT, NSM_DOUBLE,
      NSM_OBJECT
    };

  public:
    class Property {
    public:
      Property()
        : m_type(), m_length(0),
          m_offset(0) {}
      Property(Type type, int length, int offset = 0)
        : m_type(type), m_length(length),
          m_offset(offset) {}
      ~Property() {}

    public:
      Type getType() const throw() { return m_type; }
      int getLength() const throw() { return m_length; }
      int getTypeSize() const throw();
      int getOffset() const throw() { return m_offset; }
      void setType(Type type) throw() { m_type = type; }
      void setLength(int length) throw() { m_length = length; }

    private:
      Type m_type;
      int m_length;
      int m_offset;

    };

  public:
    FieldInfo() { m_type = TEXT; }
    FieldInfo(const std::string& name, Type type,
              const std::string& tablename,
              int revision, int length = 0, int id = 0)
      : m_name(name), m_id(id), m_type(type),
        m_tablename(tablename), m_revision(revision),
        m_length(length) {}
    ~FieldInfo() throw() {}

  public:
    const std::string& getName() const throw() { return m_name; }
    int getId() const throw() { return m_id; }
    Type getType() const throw() { return m_type; }
    const std::string getTypeAlias() const throw();
    const std::string getTypeName() const throw();
    const std::string& getTable() const throw() { return m_tablename; }
    int getRevision() const throw() { return m_revision; }
    int getLength() const throw() { return m_length; }
    void setName(const std::string& name) throw() { m_name = name; }
    void setId(int id) throw() { m_id = id; }
    void setType(Type type) throw() { m_type = type; }
    void setTable(const std::string& name) throw() { m_tablename = name; }
    void setRevision(int revision) throw() { m_revision = revision; }
    void setLength(int length) throw() { m_length = length; }

  public:
    const std::string setSQL() const throw();
    void setSQL(const DBRecord& record, DBObject& obj) throw();
    static const std::string getSQL(const DBObject& obj,
                                    const std::string& pname) throw();
    static const std::string getSQL(const DBObject& obj) throw();

  private:
    std::string m_name;
    int m_id;
    Type m_type;
    std::string m_tablename;
    int m_revision;
    int m_length;

  };

}

#endif
