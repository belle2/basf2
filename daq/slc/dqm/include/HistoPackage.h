#ifndef _Belle2_HistoPacakge_h
#define _Belle2_HistoPacakge_h

#include "daq/slc/base/Serializable.h"

#include "daq/slc/dqm/Histo.h"

#include <vector>

namespace Belle2 {

  class HistoPackage : public Belle2::Serializable {

  private:
    static const char ObjectFlag;

  public:
    HistoPackage();
    HistoPackage(const std::string& name);
    ~HistoPackage() throw();

  public:
    const std::string& getName() const throw() { return _name; }
    long long getUpdateTime() const throw() { return _update_time; }
    int getUpdateId() const throw() { return _update_id; }
    void setName(const std::string& name) { _name = name; }
    void setUpdateTime(long long time) { _update_time = time; }
    void setUpdateId(int id) { _update_id = id; }
    int incrementUpdateId() { return ++_update_id; }
    void setUpdateTime();
    void reset() throw();
    void clear() throw();

    MonObject* getMonObject(int index) throw() { return _histo_v[index]; }
    Histo* getHisto(int index) throw() { return (Histo*)_histo_v[index]; }
    Histo* getHisto(const std::string& name) throw();
    int getNHistos() const throw() { return (int)_histo_v.size(); }

    const MonObject* getMonObject(int index) const throw() { return _histo_v[index]; }
    const Histo* getHisto(int index) const throw() { return (Histo*) _histo_v[index]; }

    MonObject* addMonObject(MonObject* h);
    Histo* addHisto(Histo* h);
    MonObject* setMonObject(int index, MonObject* h);
    Histo* setHisto(int index, Histo* h);
    std::string toString() const throw();

  public:
    virtual void writeObject(Belle2::Writer& writer) const throw(Belle2::IOException);
    void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);
    void writeContents(Belle2::Writer& writer, bool updateAll = false) const
    throw(Belle2::IOException);
    virtual void readObject(Belle2::Reader& reader) throw(Belle2::IOException);
    void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    void readContents(Belle2::Reader& reader) throw(Belle2::IOException);

  private:
    std::vector<MonObject*> _histo_v;
    std::string _name;
    long long _update_time;
    int _update_id;

  };

};

#endif
