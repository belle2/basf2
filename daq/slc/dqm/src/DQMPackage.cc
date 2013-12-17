#include "daq/slc/dqm/DQMPackage.h"

#include <daq/slc/dqm/Histo1F.h>
#include <daq/slc/dqm/Histo2F.h>

#include <daq/slc/base/StringUtil.h>

#include <TH1.h>
#include <TString.h>

using namespace Belle2;

void DQMPackage::setHistMap(DQMHistMap* hist_m)
{
  _hist_m = hist_m;
  for (TH1Map::iterator it = _hist_m->getHists().begin();
       it != _hist_m->getHists().end(); it++) {
    TH1* h = it->second;
    TString class_name = h->ClassName();
    if (class_name.Contains("TH1")) {
      getPackage()->addHisto(new Histo1F(h->GetName(),
                                         Belle2::form("%s;%s;%s",
                                                      h->GetTitle(),
                                                      h->GetXaxis()->GetTitle(),
                                                      h->GetYaxis()->GetTitle()),
                                         h->GetNbinsX(), h->GetXaxis()->GetXmin(),
                                         h->GetXaxis()->GetXmax()));
    } else if (class_name.Contains("TH2")) {
      getPackage()->addHisto(new Histo2F(h->GetName(),
                                         Belle2::form("%s;%s;%s",
                                                      h->GetTitle(),
                                                      h->GetXaxis()->GetTitle(),
                                                      h->GetYaxis()->GetTitle()),
                                         h->GetNbinsX(), h->GetXaxis()->GetXmin(),
                                         h->GetXaxis()->GetXmax(),
                                         h->GetNbinsY(), h->GetYaxis()->GetXmin(),
                                         h->GetYaxis()->GetXmax()));
    }
  }
}
