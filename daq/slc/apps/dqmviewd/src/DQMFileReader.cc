#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/dqm/HistoPackage.h>
#include <daq/slc/dqm/Histo.h>
#include <daq/slc/dqm/Histo1D.h>
#include <daq/slc/dqm/Histo2D.h>

#include "TH2.h"
#include "TFile.h"

#include <iostream>

using namespace Belle2;

DQMFileReader::~DQMFileReader()
{
  if (_file) _file->Close();
  _file = NULL;
}

TH1* DQMFileReader::getHist(const std::string& name)
{
  return _hist_m.getHist(name);
}

bool DQMFileReader::init()
{
  _mutex.lock();
  if (_file) _file->Close();
  LogFile::debug("Reading TMapfile: %s", _hist_m.getFileName().c_str());
  _file = TMapFile::Create(_hist_m.getFileName().c_str());
  _hist_m.clear();
  TMapRec* mr = _file->GetFirst();
  while (_file->OrgAddress(mr)) {
    TObject* obj = _file->Get(mr->GetName());
    if (obj != NULL) {
      TString class_name = obj->ClassName();
      if (class_name.Contains("TH1") ||  class_name.Contains("TH2")) {
        TH1* h = (TH1*)obj;
        LogFile::debug("%s in %s", h->GetName(), _name.c_str());
        TString name = h->GetName();
        h = (TH1*)h->Clone(name + "_copy");
        _hist_m.addHist(h, name);
      }
    }
    mr = mr->GetNext();
  }
  bool ready = _ready = _hist_m.getHists().size() > 0;
  _mutex.unlock();
  return ready;
}

int DQMFileReader::update()
{
  _mutex.lock();
  _file->Update();
  bool updated = false;
  for (TH1Map::iterator it = _hist_m.getHists().begin();
       it != _hist_m.getHists().end(); it++) {
    std::string name = it->first;
    TH1* h = it->second;
    TH1* h0 = (TH1*)_file->Get(name.c_str());
    if (h->GetEntries() != h0->GetEntries()) {
      h->Reset();
      h->Add(h0);
      updated = true;
    }
    delete h0;
    if (_updateid % 1000 == 0) {
      LogFile::debug("Entries of %s = %d", name.c_str(), (int)h->GetEntries());
    }
  }
  if (updated) _updateid++;
  int updateid = _updateid;
  _mutex.unlock();
  return updateid;
}

bool DQMFileReader::dump(const std::string& dir,
                         unsigned int expno, unsigned int runno)
{
  _mutex.lock();
  _file->Update();
  std::string filepath = Form("%s/DQM_%s_%04d_%06d.root",
                              dir.c_str(), _name.c_str(), expno, runno);
  LogFile::debug("created DQM dump file: %s", filepath.c_str());
  TFile* file = new TFile(filepath.c_str(), "recreate");
  for (TH1Map::iterator it = _hist_m.getHists().begin();
       it != _hist_m.getHists().end(); it++) {
    std::string name = it->first;
    TH1* h = (TH1*)_file->Get(name.c_str());
    h->Write();
  }
  file->Close();
  delete file;
  _file->cd();
  _mutex.unlock();
  return true;
}

