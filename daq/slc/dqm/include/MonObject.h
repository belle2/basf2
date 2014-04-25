#ifndef _Belle2_MonObject_h
#define _Belle2_MonObject_h

#include "daq/slc/dqm/Shape.h"

#include "daq/slc/base/Writer.h"
#include "daq/slc/base/Reader.h"

#include <string>

namespace Belle2 {

  class MonObject : public Shape {

  public:
    MonObject() throw();
    MonObject(const std::string& name) throw();
    virtual ~MonObject() throw();

  public:
    bool isUpdated() const throw() { return m_updated; }
    int getUpdateId() const throw() { return m_update_id; }
    int getPositionId() const throw() { return m_position_id; }
    int getTabId() const throw() { return m_tab_id; }
    void setUpdated(bool updated) throw() { m_updated = updated; }
    void setUpdateId(int uid) throw() { m_update_id = uid; }
    void setPositionId(int pid) throw() { m_position_id = pid; }
    void setTabId(int tid) throw() { m_tab_id = tid; }

  public:
    virtual void reset() throw() = 0;
    virtual std::string getDataType() const throw() = 0;
    virtual void readObject(Reader& reader) throw(IOException);
    virtual void readConfig(Reader& reader) throw(IOException);
    virtual void readContents(Reader& reader) throw(IOException) = 0;
    virtual void writeObject(Writer& writer) const throw(IOException);
    virtual void writeConfig(Writer& writer) const throw(IOException);
    virtual void writeContents(Writer& writer) const throw(IOException) = 0;
    virtual std::string toString() const throw();

  protected:
    bool m_updated;
    int m_update_id;
    char m_position_id;
    char m_tab_id;

  };

};

#endif
