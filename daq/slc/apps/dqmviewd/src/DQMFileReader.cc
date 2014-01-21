#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/dqm/HistoPackage.h>
#include <daq/slc/dqm/Histo.h>
#include <daq/slc/dqm/Histo1D.h>
#include <daq/slc/dqm/Histo2D.h>

#include <daq/slc/base/Debugger.h>

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
        _hist_m.addHist(h);
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
  _updateid++;
  for (TH1Map::iterator it = _hist_m.getHists().begin();
       it != _hist_m.getHists().end(); it++) {
    std::string name = it->first;
    TH1* h_copy = (TH1*)(it->second)->Clone();
    delete it->second;
    TObject* obj = _file->Get(name.c_str());
    TH1* h = (TH1*)obj;
    TString class_name = h->ClassName();
    if (strcmp(h_copy->ClassName(), h->ClassName()) != 0 ||
        ((class_name.Contains("TH1") && (h->GetNbinsX() != h_copy->GetNbinsX())) ||
         ((class_name.Contains("TH2") && (h->GetNbinsX() != h_copy->GetNbinsX() ||
                                          h->GetNbinsY() != h_copy->GetNbinsY()))))) {
      _updateid = 0;
    }
    if (_updateid % 10 == 0) {
      LogFile::debug("Entries of %s = %d", h->GetName(), (int)h->GetEntries());
    }
    delete h_copy;
    _hist_m.addHist(h);
  }
  int updateid = _updateid;
  _mutex.unlock();
  return updateid;
}

bool DQMFileReader::dump(const std::string& dir,
                         unsigned int expno, unsigned int runno)
{
  _mutex.lock();
  std::string filepath = Form("%s/DQM_%s_%04d_%06d.root",
                              dir.c_str(), _name.c_str(), expno, runno);
  LogFile::debug("created DQM dump file: %s", filepath.c_str());
  TFile* file = new TFile(filepath.c_str(), "recreate");
  for (TH1Map::iterator it = _hist_m.getHists().begin();
       it != _hist_m.getHists().end(); it++) {
    TObject* obj = _file->Get(it->first.c_str());
    obj->Write();
  }
  file->Close();
  delete file;
  _file->cd();
  _mutex.unlock();
  return true;
}

