//+
// File : HistoManager.cc
// Description : Manage histograms in hserver
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - May - 2013
//-

#include "daq/dqm/HistoManager.h"

using namespace Belle2;
using namespace std;

// Constructor / Destructor

HistoManager::HistoManager(TMapFile* mapfile)
{
  m_mapfile = mapfile;
}

HistoManager::~HistoManager()
{
}

bool HistoManager::add(string& name, int pid, TH1* histo)
{
  //  printf ( "HistoManager: adding %s from id %d\n", name.c_str(), pid );
  if (m_list.find(name) == m_list.end()) {
    map<int, TH1*> newhlist;
    m_list[name] = newhlist;
    m_merge[name] = NULL;
    //    printf ( "HistoManager: new list created\n" );
  }
  map<int, TH1*>& hlist = m_list[name];
  if (hlist.find(pid) == hlist.end()) {
    hlist[pid] = histo;
    //    printf ( "HistoManager: histogram registered\n" );
    return true;
  }
  return false;
}

bool HistoManager::update(string& name, int pid, TH1* histo)
{
  // Register the histogram if not yet done
  if (add(name, pid, histo)) return true;

  // Retrieve the histogram list for the name
  map<int, TH1*>& hlist = m_list[name];

  // Replace histogram
  TH1* prevhisto = hlist[pid];
  delete prevhisto;
  hlist[pid] = histo;
  //  printf ( "HistoManager: histogram %s replaced, entry = %f\n",
  //     name.c_str(), histo->GetEntries());

  // Return
  return true;
}

TH1* HistoManager::get(string& name, int pid)
{
  map<int, TH1*> hlist = m_list[name];
  TH1* hist = hlist[pid];
  return hist;
}

bool HistoManager::merge()
{
  // Loop over histogram list
  for (map<string, std::map<int, TH1*> >::iterator it = m_list.begin();
       it != m_list.end(); ++it) {
    string name = it->first;
    map<int, TH1*>& hmap = it->second;
    if (m_merge[name] != NULL) {
      TH1* merge_hist = m_merge[name];
      merge_hist->Reset();
    }
    // Loop over pid list
    for (map<int, TH1*>::iterator ih = hmap.begin(); ih != hmap.end();
         ++ih) {
      int pid = ih->first;
      TH1* hist = ih->second;
      // Create new histogram in merge list if not exist yet
      if (m_merge[name] == NULL) {
        m_merge[name] = (TH1*)hist->Clone();
        m_mapfile->Add(m_merge[name]);
      }
      // Add histogram if exist
      else {
        TH1* merge_hist = m_merge[name];
        merge_hist->Add(hist);
      }
    }
  }
  m_mapfile->Update();
  //  printf ( "HistoManager: merge called and mapfile updated!!!!!\n" );
}


