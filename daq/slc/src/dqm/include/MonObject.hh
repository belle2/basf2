#ifndef _B2DQM_MonObject_hh
#define _B2DQM_MonObject_hh

#include "XMLElement.hh"
#include "Shape.hh"

#include <util/Writer.hh>
#include <util/Reader.hh>

#include <string>

namespace B2DQM {

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
    virtual void readObject(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void readConfig(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void readContents(B2DAQ::Reader& reader) throw(B2DAQ::IOException) = 0;
    virtual void writeObject(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual void writeConfig(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual void writeContents(B2DAQ::Writer& writer) const throw(B2DAQ::IOException) = 0;
    virtual std::string toString() const throw();

  protected:
    bool _updated;
    int _update_id;
    char _position_id;
    char _tab_id;

  };

};

#endif
