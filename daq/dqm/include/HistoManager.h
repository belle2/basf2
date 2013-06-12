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

#include "TH1.h"
#include "TMapFile.h"

namespace Belle2 {

  class HistoManager {
  public:
    HistoManager(TMapFile* mapfile);
    ~HistoManager();

    // Register histogram
    bool add(std::string& name, int pid, TH1* histo);
    bool update(std::string& name, int pid, TH1* histo);

    TH1* get(std::string& name, int pid);

    //    int remove ( std::string& name );



    bool merge();

  private:
    std::map<std::string, std::map<int, TH1*> > m_list;
    std::map<std::string, TH1*>  m_merge;
    TMapFile* m_mapfile;
  };
}

#endif

