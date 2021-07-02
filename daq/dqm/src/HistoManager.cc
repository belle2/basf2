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

HistoManager::HistoManager(DqmMemFile* memfile)
{
  m_memfile = memfile;
}

HistoManager::~HistoManager()
{
}

bool HistoManager::add(string& subdir, string& name, int pid, TH1* histo)
{
  //  printf ( "HistoManager: adding %s to subdir %s from id %d\n",
  //  name.c_str(), subdir.c_str(), pid );

  // Check for subdirectory
  if (m_subdir.find(subdir) == m_subdir.end()) {
    // Work dir and hist
    map<string, map<int, TH1*>>* newsubdir = new map<string, map<int, TH1*>>;
    map<int, TH1*>* newhlist = new map<int, TH1*> ;
    (*newsubdir)[name] = *newhlist;
    m_subdir[subdir] = *newsubdir;
    // Merge dir and hist
    map<string, TH1*>* newmergedir = new map<string, TH1*>;
    (*newmergedir)[name] = NULL; // TH1 is not yet created
    m_mergedir[subdir] = *newmergedir;
    printf("HistoManager: new list created for subdir %s\n", subdir.c_str());
    delete newsubdir;
    delete newhlist;
    delete newmergedir;
  }

  // Get histogram map of subdir
  map<string, map<int, TH1*>>& dirlist = m_subdir[subdir];
  map<int, TH1*>& hlist = dirlist[name];
  if (hlist.find(pid) == hlist.end()) {
    hlist[pid] = histo;
    //    printf("HistoManager: histogram %s from %d registered in %s\n",
    //           histo->GetName(), pid, subdir.c_str());
    //    histo->Print();
    return true;
  }
  return false;
}

bool HistoManager::update(string& subdir, string& name, int pid, TH1* histo)
{
  // Register the histogram if not yet done
  if (add(subdir, name, pid, histo)) return true;


  // Retrieve the histogram list for the name
  map<string, map<int, TH1*>>& dirlist = m_subdir[subdir];
  map<int, TH1*>& hlist = dirlist[name];

  // Replace histogram
  TH1* prevhisto = hlist[pid];
  if (prevhisto != NULL) delete prevhisto;
  hlist[pid] = histo;
  //  printf ( "HistoManager: histogram %s replaced in subdir %s, entry = %f\n",
  //     name.c_str(), subdir.c_str(), histo->GetEntries());
  //  hlist[pid]->Print();
  // Return
  return true;
}

TH1* HistoManager::get(string& subdir, string& name, int pid)
{
  map<string, map<int, TH1*>>& dirlist = m_subdir[subdir];
  map<int, TH1*>& hlist = dirlist[name];
  TH1* hist = hlist[pid];
  return hist;
}

bool HistoManager::merge()
{
  // Loop over subdir list
  string subdir;
  for (map<string, map<string, map<int, TH1*>>>::iterator is =
         m_subdir.begin(); is != m_subdir.end(); ++is) {
    map<string, map<int, TH1*>>& dirlist = is->second;
    map<string, TH1*>& mergelist = m_mergedir[is->first];
    // Move to the root directory of TMapFile
    //    printf("TMemFile = %8.8x\n", m_memfile->GetMemFile());
    if (m_memfile->GetMemFile() == NULL) exit(-99);
    (m_memfile->GetMemFile())->cd();
    // cd to subdirectory if defined
    subdir = is->first;
    /*
    if ( is->first != "root" ) {
      TDirectory* fdir = (m_memfile->GetmemFile())->GetDirectory();
      fdir->mkdir ( (is->first).c_str() );
      fdir->cd ( (is->first).c_str() );
      printf ( "TMemFile: subdir set to %s\n", (is->first).c_str() );
      fdir->ls();
    }
    */
    // Loop over histogram list
    for (map<string, std::map<int, TH1*> >::iterator it = dirlist.begin();
         it != dirlist.end(); ++it) {
      string name = it->first;
      map<int, TH1*>& hmap = it->second;
      if (mergelist[name] != NULL) {
        TH1* merge_hist = mergelist[name];
        merge_hist->Reset();

      }
      // Loop over pid list
      for (map<int, TH1*>::iterator ih = hmap.begin(); ih != hmap.end();
           ++ih) {
        //        int pid = ih->first;
        TH1* hist = ih->second;
        //  printf ( "Retrieving histo %s from pid = %d\n", hist->GetName(), pid );
        //  hist->Print();
        // Create new histogram in merge list if not exist yet
        if (mergelist[name] == NULL) {
          //    printf ( "HistoManager: adding %s (class %s) to mergelist\n",
          //       hist->GetName(), hist->ClassName() );
          string newname;
          if (subdir != "")
            newname = subdir + "/" + string(hist->GetName());
          else
            newname = string(hist->GetName());
          hist->SetName(newname.c_str());
          TH1* mhist = (TH1*)hist->Clone();
          mergelist[name] = mhist;
          //    mergelist[name] = (TH1*)hist->Clone();
          //    m_mapfile->Add(mergelist[name]);
          //    delete hist;
        }
        // Add histogram if exist
        else {
          TH1* merge_hist = mergelist[name];
          merge_hist->Add(hist);
          merge_hist->SetTitle(hist->GetTitle());
        }
      }
    }
  }
  //  m_mapfile->Update();
  m_memfile->UpdateSharedMem();

  //  m_mapfile->ls();
  //  printf ( "HistoManager: merge called and mapfile updated!!!!!\n" );
  return true;
}

void HistoManager::clear()
{
  for (map<string, map<string, map<int, TH1*>>>::iterator is = m_subdir.begin(); is != m_subdir.end(); ++is) {
    map<string, map<int, TH1*>>& dirlist = is->second;

    for (map<string, std::map<int, TH1*> >::iterator it = dirlist.begin(); it != dirlist.end(); ++it) {
      map<int, TH1*>& hmap = it->second;

      for (map<int, TH1*>::iterator ih = hmap.begin(); ih != hmap.end(); ++ih) {
        TH1* hist = ih->second;
        if (hist != NULL) delete hist;
        //if (hist != NULL) hist->Reset();
      }
      hmap.clear();
    }
    dirlist.clear();
  }
  m_subdir.clear();

  for (map<string, map<string, TH1*> >::iterator is = m_mergedir.begin(); is != m_mergedir.end(); ++is) {
    map<string, TH1*>& dirlist = is->second;

    for (map<string, TH1*>::iterator it = dirlist.begin(); it != dirlist.end(); ++it) {
      TH1* hist = it->second;
      if (hist != NULL) delete hist;
      //it->second = NULL;
      //if (hist != NULL) hist->Reset();
    }
    dirlist.clear();
  }
  m_mergedir.clear();

  printf("HistoManager: clear\n");
  m_memfile->ClearSharedMem();
}

