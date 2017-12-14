#ifndef _Belle2_DQMHistMap_h
#define _Belle2_DQMHistMap_h

#include <map>
#include <string>

#include <TH1.h>

namespace Belle2 {

  typedef std::map<std::string, TH1*> TH1Map;

  class DQMHistMap {

  public:
    DQMHistMap(const std::string& file_name = "")
    {
      m_file_name = file_name;
      m_ndirs = 0;
    }
    ~DQMHistMap() {}

  public:
    const std::string& getFileName() { return m_file_name; }
    void addHist(TH1* hist, const std::string& dir, const std::string& name);
    TH1* getHist(const std::string& name);
    bool hasHist(const std::string& name);
    TH1Map& getHists() { return m_hist_m; }
    const TH1Map& getHists() const { return m_hist_m; }
    size_t getNDirectories() const { return m_ndirs; }
    std::map<std::string, std::string>& getDirectories() { return m_dir_m; }
    const std::string& getDirectory(const std::string& name)
    {
      return m_dir_m[name];
    }
    void clear();

  private:
    std::string m_file_name;
    TH1Map m_hist_m;
    size_t m_ndirs;
    std::map<std::string, std::string> m_dir_m;

  };

}

#endif
