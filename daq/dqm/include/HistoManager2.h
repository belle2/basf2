/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <map>
#include "TH1.h"

namespace Belle2 {

  class HistoManager2 {
  public:
    HistoManager2(std::string name);
    ~HistoManager2();

    // Register histogram
    bool add(const std::string& subdir, const std::string& name, int pid, TH1* histo);
    bool update(const std::string& subdir, const std::string& name, int pid, TH1* histo);

    TH1* get(const std::string& subdir, const std::string& name, int pid);

    bool merge(void);
    void clear(void);
    void filedump(std::string outfile);

  private:
    // List to handle histograms sent from different nodes
    // <subdirname, map<histoname, map<pid, TH1*>>>
    std::map<std::string, std::map<std::string, std::map<int, TH1*>> > m_subdir;

    // List to handle histograms on TMapFile
    // <subdirname, map<histoname, TH1*>>
    std::map<std::string, std::map<std::string, TH1*>> m_mergedir;

    std::string m_name;
  };
}
