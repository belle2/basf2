/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistComparitor.cc
// Description : DQM Histogram analysis module, compares histo with references
//-


#include <framework/core/ModuleParam.templateDetails.h>
#include <dqm/analysis/modules/DQMHistComparitor.h>
#include <TStyle.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistComparitor);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistComparitorModule::DQMHistComparitorModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistoList", m_histlist,
           "['histname', 'refhistname', 'canvas', 'delta', 'color', 'minentry', 'Chi/KS', 'warnlvl', 'errlvl', 'fitresult_pvname', 'status_pvname'],[...],...");
  B2DEBUG(1, "DQMHistComparitor: Constructor done.");
}

DQMHistComparitorModule::~DQMHistComparitorModule()
{
}

void DQMHistComparitorModule::initialize()
{
  gStyle->SetOptStat(0);
  gStyle->SetStatStyle(1);
  gStyle->SetOptDate(22);// Date and Time in Bottom Right, does no work

  for (auto& it : m_histlist) {
    if (it.size() != 11) {
      B2WARNING("Histolist with wrong nr of parameters (" << it.size() << "), histo=" << (it.size() > 0 ? it.at(0) : std::string("")));
      continue;
    }

    auto n = new CMPNODE;
    n->histName = it.at(0);
    n->refName = it.at(1) == "" ? n->histName : it.at(1) ; // only use if set
    n->canvas = new TCanvas(it.at(2).c_str());

    n->deltaflag = it.at(3) != "";
    n->colorflag = it.at(4) != "";
    n->min_entries = atoi(it.at(5).c_str()); // if delta hist is used, value must be lower or equal delta entries value
    if (it.at(6) == "KS") n->algo = 1;
    else n->algo = 0; // chi as default
    n->warning = atof(it.at(7).c_str());
    n->error = atof(it.at(8).c_str());

    n->pvfit = it.at(9);
    if (n->pvfit != "") registerEpicsPV(n->pvfit);
    n->pvstatus = it.at(10);
    if (n->pvstatus != "") registerEpicsPV(n->pvstatus);
    m_pnode.push_back(n);
  }

  B2DEBUG(20, "DQMHistComparitor: initialized.");
}


void DQMHistComparitorModule::beginRun()
{
  B2DEBUG(20, "DQMHistComparitor: beginRun called.");
}

void DQMHistComparitorModule::event()
{
  gStyle->SetOptStat(0);
  gStyle->SetStatStyle(0);
  gStyle->SetOptDate(22);// Date and Time in Bottom Right, does no work

  for (auto& it : m_pnode) {
    TH1* histo = nullptr;
    B2DEBUG(20, "== Search for " << it->histName << " with ref " << it->refName << "==");
    if (it->deltaflag) histo = getDelta(it->histName); // default: last delta
    if (!histo) histo = findHist(it->histName, true); // only if changed
    if (!histo) {
      // B2DEBUG(20, "NOT Found " << it->histName);
      // Dont compare if missing or not updated ...
      continue;
    }
    // if compare normalized ... need configuration
    auto refhist = findRefHist(it->refName, ERefScaling::c_RefScaleEntries, histo);
    if (!refhist) {
      B2DEBUG(20, "NOT Found " << it->refName);
      // Dont compare if any of hists is missing ...
      continue;
    }
    B2DEBUG(20, "Compare " << it->histName << " with ref " << it->refName);

    it->canvas->UseCurrentStyle();
    it->canvas->Clear();
    it->canvas->cd();

    double data = 0;
    if (it->algo == 1) {
      data = histo->KolmogorovTest(refhist, ""); // returns p value (0 bad, 1 good), N - do not compare normalized
      B2DEBUG(20, "KS: " << data << " , " << it->warning << " , " << it->error);
    } else { // default
      data = histo->Chi2Test(refhist, "NORM,UU"); // return p value (0 bad, 1 good), ignores normalization
      B2DEBUG(20, "Chi: " << data << " , " << it->warning << " , " << it->error);
    }
    // TODO other type?
    //     data= BinByBinTest(hits1,refhist);// user function (like Peters test)
    //     B2DEBUG(20, "User: " << data<< " , " <<it->warning<< " , " <<it->error);
    if (it->pvfit != "") setEpicsPV(it->pvfit, data);

    refhist->SetLineStyle(3);// 2 or 3
    refhist->SetLineColor(3);

    /*
      // I think no need for this anymore
      TIter nextkey(it->canvas->GetListOfPrimitives());
      TObject* obj = NULL;
      while ((obj = (TObject*)nextkey())) {
        if (obj->IsA()->InheritsFrom("TH1")) {
          if (string(obj->GetName()) == string(refhist->GetName())) {
            delete obj;
          }
        }
      }
    */

    refhist->Draw("hist");
    refhist->SetStats(kFALSE);
    if (refhist->GetMaximum() > histo->GetMaximum())
      histo->SetMaximum(1.1 * refhist->GetMaximum());
    histo->Draw("hist");
    refhist->Draw("hist,same");
    // think about the order, reference should be behind histogram?

    auto status_data = makeStatus(histo->GetEntries() >= it->min_entries, data < it->warning,
                                  data < it->error); // this must be double for EPICS below!
    if (it->pvstatus != "") setEpicsPV(it->pvstatus, status_data);
    if (it->colorflag) colorizeCanvas(it->canvas, status_data);

    //auto tt = new TPaveText(.80, 1, 1, .90, "NDC");
    //tt->SetFillColor(kWhite);
    //tt->AddText(("Prob: " + std::to_string(data)).data());
    //tt->Draw();
    // delete tt;// do not delete it, otherwise it vanishes from the Canvas!

    it->canvas->Modified();
    it->canvas->Update();
  }
}

void DQMHistComparitorModule::endRun()
{
  B2DEBUG(20, "DQMHistComparitor: endRun called");
}

void DQMHistComparitorModule::terminate()
{
  B2DEBUG(20, "DQMHistComparitor: terminate called");
}
