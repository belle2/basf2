#include "DefaultHistoManager.hh"

#include <dqm/Histo1F.hh>
#include <dqm/Histo2F.hh>
#include <dqm/RootPanel.hh>
#include <dqm/TabbedPanel.hh>
#include <dqm/CanvasPanel.hh>

#include <util/StringUtil.hh>

#include <TH1.h>

using namespace B2DQM;

DefaultHistoManager::DefaultHistoManager(const std::string& name)
  : _name(name)
{

}

HistoPackage* DefaultHistoManager::createPackage(RootHistMap& hist_m)
{
  _pack = new HistoPackage(_name);
  for (RootHistMap::iterator it = hist_m.begin();
       it != hist_m.end(); it++) {
    TH1* h = it->second;
    TString class_name = h->ClassName();
    if (class_name.Contains("TH1")) {
      _pack->addHisto(new Histo1F(h->GetName(),
                                  B2DAQ::form("%s;%s;%s",
                                              h->GetTitle(),
                                              h->GetXaxis()->GetTitle(),
                                              h->GetYaxis()->GetTitle()),
                                  h->GetNbinsX(), h->GetXaxis()->GetXmin(),
                                  h->GetXaxis()->GetXmax()));
    } else if (class_name.Contains("TH2")) {
      _pack->addHisto(new Histo2F(h->GetName(),
                                  B2DAQ::form("%s;%s;%s",
                                              h->GetTitle(),
                                              h->GetXaxis()->GetTitle(),
                                              h->GetYaxis()->GetTitle()),
                                  h->GetNbinsX(), h->GetXaxis()->GetXmin(),
                                  h->GetXaxis()->GetXmax(),
                                  h->GetNbinsY(), h->GetYaxis()->GetXmin(),
                                  h->GetYaxis()->GetXmax()));
    }
  }
  return _pack;
}

RootPanel* DefaultHistoManager::createRootPanel(RootHistMap& hist_m)
{
  RootPanel* root_panel = new RootPanel(_name);
  TabbedPanel* tabpanel = new TabbedPanel("tab_main");
  root_panel->add(tabpanel);
  for (int i = 0; i < _pack->getNHistos(); i++) {
    Histo* histo = _pack->getHisto(i);
    std::string c_name = B2DAQ::form("c_%s", histo->getName().c_str());
    CanvasPanel* canvas = new CanvasPanel(c_name, histo->getTitle());
    canvas->add(histo);
    tabpanel->add(histo->getTitle(), canvas);
  }
  return root_panel;
}


