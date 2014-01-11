#include "daq/slc/dqm/DQMPackage.h"

#include <daq/slc/dqm/Histo1D.h>
#include <daq/slc/dqm/Histo2D.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>

#include <TH1.h>
#include <TString.h>

using namespace Belle2;

void DQMPackage::setHistMap(DQMHistMap* hist_m)
{
  HistoPackage* pack = getPackage();
  _hist_m = hist_m;
  for (TH1Map::iterator it = _hist_m->getHists().begin();
       it != _hist_m->getHists().end(); it++) {
    TH1* h = it->second;
    TString class_name = h->ClassName();
    if (class_name.Contains("TH1") ||  class_name.Contains("TH2")) {
      if (!pack->hasHisto(h->GetName())) {
        Histo* histo = makeHisto(h);
        if (histo != NULL) {
          pack->addHisto(histo);
        }
      }
    }
  }
}

Histo* DQMPackage::makeHisto(TH1* h) throw()
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

MonColor DQMPackage::makeColor(int num) throw()
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
