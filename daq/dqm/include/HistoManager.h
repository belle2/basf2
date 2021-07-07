/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef HISTOMANAGER_H
#define HISTOMANAGER_H

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

