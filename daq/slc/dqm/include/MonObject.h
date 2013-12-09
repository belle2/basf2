#ifndef _Belle2_MonObject_hh
#define _Belle2_MonObject_hh

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
    bool isUpdated() const throw() { return _updated; }
    int getUpdateId() const throw() { return _update_id; }
    int getPositionId() const throw() { return _position_id; }
    int getTabId() const throw() { return _tab_id; }
    void setUpdated(bool updated) throw() { _updated = updated; }
    void setUpdateId(int uid) throw() { _update_id = uid; }
    void setPositionId(int pid) throw() { _position_id = pid; }
    void setTabId(int tid) throw() { _tab_id = tid; }

  public:
    virtual void reset() throw() = 0;
    virtual std::string getDataType() const throw() = 0;
    virtual void readObject(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void readContents(Belle2::Reader& reader) throw(Belle2::IOException) = 0;
    virtual void writeObject(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void writeContents(Belle2::Writer& writer) const throw(Belle2::IOException) = 0;
    virtual std::string toString() const throw();

  protected:
    bool _updated;
    int _update_id;
    char _position_id;
    char _tab_id;

  };

};

#endif
