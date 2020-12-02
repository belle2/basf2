#ifndef HISTOMANAGER_H
#define HISTOMANAGER_H
//+
// File : HistoManager.h
// Description : Manage histograms in hserver
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - May - 2013
//-

#include <string>
#include <map>

#include <daq/dqm/DqmMemFile.h>

#include "TH1.h"

namespace Belle2 {

  class HistoManager {
  public:
    HistoManager(DqmMemFile* mapfile);
    ~HistoManager();

    // Register histogram
    bool add(std::string& subdir, std::string& name, int pid, TH1* histo);
    bool update(std::string& subdir, std::string& name, int pid, TH1* histo);

    TH1* get(std::string& subdir, std::string& name, int pid);

    //    int remove ( std::string& name );



    bool merge();
    void clear();

  private:
    // List to handle histograms sent from different nodes
    // <subdirname, map<histoname, map<pid, TH1*>>>
    std::map<std::string, std::map<std::string, std::map<int, TH1*>> > m_subdir;
    //OBSOLETE    std::map<std::string, std::map<int, TH1*> > m_list;

    // List to handle histograms on TMapFile
    // <subdirname, map<histoname, TH1*>>
    std::map<std::string, std::map<std::string, TH1*>> m_mergedir;
    //OBSOLETE    std::map<std::string, TH1*>  m_merge;

    // TMapFile
    DqmMemFile* m_memfile;
  };
}

#endif

