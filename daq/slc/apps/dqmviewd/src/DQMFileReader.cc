#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include <daq/slc/dqm/HistoPackage.h>
#include <daq/slc/dqm/Histo.h>
#include <daq/slc/dqm/Histo1D.h>
#include <daq/slc/dqm/Histo2D.h>

#include <daq/slc/base/Debugger.h>

#include "TH2.h"

#include <iostream>

using namespace Belle2;

DQMFileReader::DQMFileReader() : _file(NULL)
{
  _hist_m = new DQMHistMap();
}

DQMFileReader::~DQMFileReader()
{
  delete _hist_m;
  if (_file) _file->Close();
  _file = NULL;
}

TH1* DQMFileReader::getHist(const std::string& name)
{
  return _hist_m->getHist(name);
}

bool DQMFileReader::init(const char* file_path)
{
  if (_file) _file->Close();
  _file = TMapFile::Create(file_path);
  _hist_m->clear();
  TMapRec* mr = _file->GetFirst();
  while (_file->OrgAddress(mr)) {
    TObject* obj = _file->Get(mr->GetName());
    if (obj != NULL) {
      TString class_name = obj->ClassName();
      if (class_name.Contains("TH1") ||  class_name.Contains("TH2")) {
        TH1* h = (TH1*)obj;
        _hist_m->addHist(h);
      }
    }
    mr = mr->GetNext();
  }
  return (_ready = _hist_m->getHists().size() > 0);
}

void DQMFileReader::update(HistoPackage* pack)
{
  if (pack == NULL) return;
  _file->Update();
  for (TH1Map::iterator it = _hist_m->getHists().begin();
       it != _hist_m->getHists().end(); it++) {
    std::string name = it->first;
    Histo* histo = (Histo*)pack->getHisto(name);
    if (histo != NULL && _hist_m->hasHist(name)) {
      delete it->second;
      TObject* obj = _file->Get(name.c_str());
      TH1* h = (TH1*)obj;
      _hist_m->addHist(h);
      if (histo->getDim() == 1) {
        for (int nbinx = 0; nbinx < h->GetNbinsX(); nbinx++) {
          histo->setBinContent(nbinx, h->GetBinContent(nbinx + 1));
        }
      } else if (histo->getDim() == 2) {
        for (int nbiny = 0; nbiny < h->GetNbinsY(); nbiny++) {
          for (int nbinx = 0; nbinx < h->GetNbinsX(); nbinx++) {
            histo->setBinContent(nbinx, nbiny, h->GetBinContent(nbinx + 1, nbiny + 1));
          }
        }
      }
      histo->setUpdated(true);
    }
  }
}

