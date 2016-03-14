#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/dqm/HistoPackage.h>
#include <daq/slc/dqm/Histo.h>
#include <daq/slc/dqm/Histo1D.h>
#include <daq/slc/dqm/Histo2D.h>

#include <daq/slc/base/StringUtil.h>

#include "TH2.h"
#include "TFile.h"

#include <iostream>

using namespace Belle2;

DQMFileReader::~DQMFileReader()
{
  if (m_file) m_file->Close();
  m_file = NULL;
}

TH1* DQMFileReader::getHist(const std::string& name)
{
  return m_hist_m.getHist(name);
}

bool DQMFileReader::init()
{
  m_mutex.lock();
  if (m_file) m_file->Close();
  m_file = TMapFile::Create(m_hist_m.getFileName().c_str());
  m_hist_m.clear();
  TMapRec* mr = m_file->GetFirst();
  while (m_file->OrgAddress(mr)) {
    TObject* obj = m_file->Get(mr->GetName());
    if (obj != NULL) {
      TString class_name = obj->ClassName();
      if (class_name.Contains("TH1") ||  class_name.Contains("TH2")) {
        TH1* h = (TH1*)obj;
        std::string name = h->GetName();
        StringList str_v = StringUtil::split(name, '/');
        std::string dir = "";
        if (str_v.size() > 1) {
          dir = str_v[0];
          name = str_v[1];
        }
        h = (TH1*)h->Clone((name + "_copy").c_str());
        m_hist_m.addHist(h, dir, name);

      }
    }
    mr = mr->GetNext();
  }
  bool ready = m_ready = m_hist_m.getHists().size() > 0;
  m_mutex.unlock();
  return ready;
}

int DQMFileReader::update()
{
  m_mutex.lock();
  m_file->Update();
  bool updated = false;
  for (TH1Map::iterator it = m_hist_m.getHists().begin();
       it != m_hist_m.getHists().end(); it++) {
    std::string name = it->first;
    std::string dir = m_hist_m.getDirectory(name);
    if (dir.size() > 0) name = dir + "/" + name;
    TH1* h = it->second;
    TH1* h0 = (TH1*)m_file->Get(name.c_str());
    if (h->GetEntries() != h0->GetEntries()) {
      h->Reset();
      h->Add(h0);
      updated = true;
    }
    delete h0;
  }
  if (updated) m_updateid++;
  int updateid = m_updateid;
  m_mutex.unlock();
  return updateid;
}

bool DQMFileReader::dump(const std::string& dir,
                         unsigned int expno, unsigned int runno)
{
  m_mutex.lock();
  m_file->Update();
  std::string filepath = Form("%s/DQM_%s_%04d_%06d.root",
                              dir.c_str(), m_name.c_str(), expno, runno);
  LogFile::info("created DQM dump file: %s", filepath.c_str());
  TFile* file = new TFile(filepath.c_str(), "recreate");
  for (TH1Map::iterator it = m_hist_m.getHists().begin();
       it != m_hist_m.getHists().end(); it++) {
    std::string name = it->first;
    std::string dir = m_hist_m.getDirectory(name);
    if (dir.size() > 0) name = dir + "/" + name;
    TH1* h = (TH1*)m_file->Get(name.c_str());
    h->Write();
  }
  file->Close();
  delete file;
  m_file->cd();
  m_mutex.unlock();
  return true;
}

