#ifndef _Belle2_HistoPacakge_h
#define _Belle2_HistoPacakge_h

#include "daq/slc/base/Serializable.h"

#include "daq/slc/dqm/Histo.h"

#include <vector>

namespace Belle2 {

  class HistoPackage : public Serializable {

  private:
    static const char ObjectFlag;

  public:
    HistoPackage();
    HistoPackage(const std::string& name);
    ~HistoPackage() throw();

  public:
    const std::string& getName() const throw() { return m_name; }
    long long getUpdateTime() const throw() { return m_update_time; }
    int getUpdateId() const throw() { return m_update_id; }
    void setName(const std::string& name) { m_name = name; }
    void setUpdateTime(long long time) { m_update_time = time; }
    void setUpdateId(int id) { m_update_id = id; }
    int incrementUpdateId() { return ++m_update_id; }
    void setUpdateTime();
    void reset() throw();
    void clear() throw();

    MonObject* getMonObject(int index) throw() { return m_histo_v[index]; }
    Histo* getHisto(int index) throw() { return (Histo*)m_histo_v[index]; }
    Histo* getHisto(const std::string& name) throw();
    int getNHistos() const throw() { return (int)m_histo_v.size(); }

    const MonObject* getMonObject(int index) const throw() { return m_histo_v[index]; }
    const Histo* getHisto(int index) const throw() { return (Histo*) m_histo_v[index]; }
    bool hasHisto(const std::string& name) throw();

    MonObject* addMonObject(MonObject* h);
    Histo* addHisto(Histo* h);
    MonObject* setMonObject(int index, MonObject* h);
    Histo* setHisto(int index, Histo* h);
    std::string toString() const throw();

  public:
    virtual void writeObject(Writer& writer) const throw(IOException);
    void writeConfig(Writer& writer) const throw(IOException);
    void writeContents(Writer& writer, bool updateAll = false) const
    throw(IOException);
    virtual void readObject(Reader& reader) throw(IOException);
    void readConfig(Reader& reader) throw(IOException);
    void readContents(Reader& reader) throw(IOException);

  private:
    std::vector<MonObject*> m_histo_v;
    std::string m_name;
    long long m_update_time;
    int m_update_id;

  };

};

#endif
