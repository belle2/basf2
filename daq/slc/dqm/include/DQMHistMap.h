#ifndef _Belle2_DQMHistMap_h
#define _Belle2_DQMHistMap_h

#include <map>
#include <string>

#include <TH1.h>

namespace Belle2 {

  typedef std::map<std::string, TH1*> TH1Map;

  class DQMHistMap {

  public:
    DQMHistMap(const std::string& file_name = "") {
      m_file_name = file_name;
    }
    ~DQMHistMap() {}

  public:
    const std::string& getFileName() { return m_file_name; }
    void addHist(TH1* hist, const char* name);
    TH1* getHist(const std::string& name);
    bool hasHist(const std::string& name);
    TH1Map& getHists() { return m_hist_m; }
    const TH1Map& getHists() const { return m_hist_m; }
    void clear();

  private:
    std::string m_file_name;
    TH1Map m_hist_m;

  };

}

#endif
