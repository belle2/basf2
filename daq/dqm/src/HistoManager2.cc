/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/dqm/HistoManager2.h"
#include <TROOT.h>
#include <TFile.h>
#include <filesystem>

using namespace Belle2;
using namespace std;

// Constructor / Destructor

HistoManager2::HistoManager2(std::string name)
{
  m_name = name;
  clear();
  merge();
}

HistoManager2::~HistoManager2()
{
}

bool HistoManager2::add(const string& subdir, const string& name, int pid, TH1* histo)
{
  //  printf ( "HistoManager2: adding %s to subdir %s from id %d\n",
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
    printf("HistoManager2: new list created for subdir %s\n", subdir.c_str());
    delete newsubdir;
    delete newhlist;
    delete newmergedir;
  }

  // Get histogram map of subdir
  map<string, map<int, TH1*>>& dirlist = m_subdir[subdir];
  map<int, TH1*>& hlist = dirlist[name];
  if (hlist.find(pid) == hlist.end()) {
    hlist[pid] = histo;
    //    printf("HistoManager2: histogram %s from %d registered in %s\n",
    //           histo->GetName(), pid, subdir.c_str());
    //    histo->Print();
    return true;
  }
  return false;
}

bool HistoManager2::update(const string& subdir, const string& name, int pid, TH1* histo)
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
  //  printf ( "HistoManager2: histogram %s replaced in subdir %s, entry = %f\n",
  //     name.c_str(), subdir.c_str(), histo->GetEntries());
  //  hlist[pid]->Print();
  // Return
  return true;
}

TH1* HistoManager2::get(const string& subdir, const string& name, int pid)
{
  const map<string, map<int, TH1*>>& dirlist = m_subdir[subdir];
  const map<int, TH1*>& hlist = dirlist.at(name);
  TH1* hist = hlist.at(pid);
  return hist;
}

bool HistoManager2::merge()
{
  {
    // Loop over subdir list
    string subdir;
    for (map<string, map<string, map<int, TH1*>>>::iterator is =
           m_subdir.begin(); is != m_subdir.end(); ++is) {
      map<string, map<int, TH1*>>& dirlist = is->second;
      map<string, TH1*>& mergelist = m_mergedir[is->first];
      // Move to the root directory
      gROOT->cd();

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
            //    printf ( "HistoManager2: adding %s (class %s) to mergelist\n",
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
  }
  TFile* fh;
  fh = new TFile(("/dev/shm/tmp_" + m_name).c_str(), "RECREATE");
  if (fh) {
    if (!fh->IsOpen()) {
      printf("HistoManager2 cannot open file : %s\n", ("/dev/shm/tmp_" + m_name).c_str());
      return false;
    }

    // Loop over subdir list
    for (map<string, map<string, map<int, TH1*>>>::iterator is =
           m_subdir.begin(); is != m_subdir.end(); ++is) {
      map<string, map<int, TH1*>>& dirlist = is->second;
      map<string, TH1*>& mergelist = m_mergedir[is->first];
      // No directories, all Hist are in root dir but with a dir prefix in its name
      for (map<string, std::map<int, TH1*> >::iterator it = dirlist.begin();
           it != dirlist.end(); ++it) {
        string name = it->first;
        TH1* merge_hist = mergelist[name];
        if (merge_hist) merge_hist->Write();
      }
    }
    fh->Close();
    delete fh;
  }

  if (rename(("/dev/shm/tmp_" + m_name).c_str(), ("/dev/shm/" + m_name).c_str())) {
    perror("Rename dhm file failed ");
    return false;
  }

  return true;
}

void HistoManager2::clear()
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

  // we delete the outfile(s)
  unlink(("/dev/shm/tmp_" + m_name).c_str());
  unlink(("/dev/shm/" + m_name).c_str());
  printf("HistoManager2: clear\n");
}


void HistoManager2::filedump(std::string outfile)
{
  printf("dump to dqm file = %s\n", outfile.c_str());
  merge(); // Smart move to first merge, but necessary?
  // update/overwrite existing. we could be faster with sendfile, but more code to write
  filesystem::copy_file("/dev/shm/" + m_name, outfile, filesystem::copy_options::overwrite_existing);
}
