#include "HistoFileReader.h"

#include <dqm/HistoPackage.h>
#include <dqm/Histo.h>

#include "TH2.h"

#include <iostream>

using namespace Belle2;

HistoFileReader::HistoFileReader(const char* file_path)
  : _file(NULL)
{
  init(file_path);
}

HistoFileReader::~HistoFileReader()
{
  if (_file) _file->Close();
  _file = NULL;
}

TH1* HistoFileReader::getHist(const std::string& name)
{
  if (_hist_m.find(name) != _hist_m.end()) return _hist_m[name];
  else return NULL;
}

bool HistoFileReader::init(const char* file_path)
{
  if (_file) _file->Close();
  _file = TMapFile::Create(file_path);
  _hist_m = RootHistMap();
  TMapRec* mr = _file->GetFirst();
  while (_file->OrgAddress(mr)) {
    TObject* obj = _file->Get(mr->GetName());
    if (obj != NULL) {
      TString class_name = obj->ClassName();
      if (class_name.Contains("TH1") ||  class_name.Contains("TH2")) {
        TH1* h = (TH1*)obj;
        _hist_m.insert(RootHistMap::value_type(obj->GetName(), h));
      }
    }
    mr = mr->GetNext();
  }
  return (_ready = _hist_m.size() > 0);
}

void HistoFileReader::update(HistoPackage* pack)
{
  if (pack == NULL) return;
  _file->Update();
  for (RootHistMap::iterator it = _hist_m.begin();
       it != _hist_m.end(); it++) {
    std::string name = it->first;
    Histo* histo = (Histo*)pack->getHisto(name);
    if (histo != NULL &&
        _hist_m.find(name) != _hist_m.end()) {
      delete it->second;
      TObject* obj = _file->Get(name.c_str());
      TH1* h = it->second = (TH1*)obj;
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

