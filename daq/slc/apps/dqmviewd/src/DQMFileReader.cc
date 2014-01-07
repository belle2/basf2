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
}

DQMFileReader::~DQMFileReader()
{
  if (_file) _file->Close();
  _file = NULL;
}

TH1* DQMFileReader::getHist(const std::string& name)
{
  return _hist_m.getHist(name);
}

bool DQMFileReader::init(const char* file_path, HistoPackage* pack)
{
  if (_file) _file->Close();
  _file = TMapFile::Create(file_path);
  _hist_m.clear();
  TMapRec* mr = _file->GetFirst();
  while (_file->OrgAddress(mr)) {
    TObject* obj = _file->Get(mr->GetName());
    if (obj != NULL) {
      TString class_name = obj->ClassName();
      if (class_name.Contains("TH1") ||  class_name.Contains("TH2")) {
        TH1* h = (TH1*)obj;
        _hist_m.addHist(h);
        if (!pack->hasHisto(h->GetName())) {
          Histo* histo = makeHisto(h);
          if (histo != NULL) {
            pack->addHisto(histo);
          }
        }
      }
    }
    mr = mr->GetNext();
  }
  return (_ready = _hist_m.getHists().size() > 0);
}

void DQMFileReader::update(HistoPackage* pack)
{
  if (pack == NULL) return;
  _file->Update();
  for (TH1Map::iterator it = _hist_m.getHists().begin();
       it != _hist_m.getHists().end(); it++) {
    std::string name = it->first;
    Histo* histo = (Histo*)pack->getHisto(name);
    if (histo != NULL && _hist_m.hasHist(name)) {
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

Histo* DQMFileReader::makeHisto(TH1* h) throw()
{
  const std::string name(h->GetName());
  const std::string title(h->GetTitle());
  int nbinx = h->GetXaxis()->GetNbins();
  double xmin = h->GetXaxis()->GetXmin();
  double xmax = h->GetXaxis()->GetXmax();
  Histo* histo = NULL;
  TString class_name = h->ClassName();
  if (class_name.Contains("TH1")) {
    histo = new Histo1D(name, "", nbinx, xmin, xmax);
  } else if (class_name.Contains("TH2")) {
    int nbiny = h->GetYaxis()->GetNbins();
    double ymin = h->GetYaxis()->GetXmin();
    double ymax = h->GetYaxis()->GetXmax();
    histo = new Histo2D(name, "", nbinx, xmin, xmax,
                        nbiny, ymin, ymax);
  }
  if (histo != NULL) {
    MonColor fill_color = makeColor(h->GetFillColor());
    histo->setFill(new FillProperty(fill_color));
    MonColor line_color = makeColor(h->GetLineColor());
    if (line_color == MonColor::NON && fill_color == MonColor::NON &&
        !class_name.Contains("TH2")) {
      line_color = MonColor::BLACK;
    }
    int width = h->GetLineWidth();
    histo->setLine(new LineProperty(line_color, width));
    if (std::string(h->GetOption()).find("TEXT") != std::string::npos) {
      histo->setDrawOption("ALT");
      MonColor font_color = makeColor(h->GetMarkerColor());
      if (font_color == MonColor::NON) font_color = MonColor::BLACK;
      histo->setFont(new FontProperty(font_color, h->GetMarkerSize() * 0.5));
    }
    histo->setTitle(title);
    histo->getAxisX().setTitle(h->GetXaxis()->GetTitle());
    histo->getAxisY().setTitle(h->GetYaxis()->GetTitle());
    if (h->GetMinimum() != h->GetBinContent(h->GetMinimumBin())) {
      histo->fixMinimum(h->GetMinimum());
    }
    if (h->GetMaximum() != h->GetBinContent(h->GetMaximumBin())) {
      histo->fixMaximum(h->GetMaximum());
    }
  }
  return histo;
}

MonColor DQMFileReader::makeColor(int num) throw()
{
  MonColor color;
  switch (num) {
    case 1 : color = MonColor::BLACK; break;
    case 2 : color = MonColor::RED; break;
    case 3 : color = MonColor::GREEN; break;
    case 4 : color = MonColor::BLUE; break;
    case 5 : color = MonColor::YELLOW; break;
    case 6 : color = MonColor::MAGENTA; break;
    case 7 : color = MonColor::CYAN; break;
    case 8 : color = MonColor::RUNNING_GREEN; break;
    case 10 : color = MonColor::WHITE; break;
    case 11 : color = MonColor::LIGHT_GRAY; break;
    case 12 : color = MonColor::DARK_GRAY; break;
    case 13 : color = MonColor::GRAY; break;
    default: color = MonColor::NON; break;
  }
  return color;
}
