/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAutoCanvas.cc
// Description : This module auto-plots each histogram in its canvas
//-

#include <dqm/analysis/modules/DQMHistAutoCanvas.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAutoCanvas);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAutoCanvasModule::DQMHistAutoCanvasModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("AutoCanvasFolders", m_acfolders, "List of histograms to automatically create canvases, empty for all",
           std::vector<std::string>());
  addParam("ExcludeFolders", m_exclfolders, "List of folders to exclude from create canvases, empty for none, \"all\" for all",
           std::vector<std::string>());
  B2DEBUG(1, "DQMHistAutoCanvas: Constructor done.");
}

void DQMHistAutoCanvasModule::event()
{
  // There may be a more clever way instead of doing the iteration my myself here
  // This only works because the getHistList is not const anymore
  for (auto& it : getHistList()) {
    bool give_canvas = false;
    TString histoname = it.first;

    // the following lines prevent any histogram outside a directory to be processed
    auto split_result = StringSplit(histoname.Data(), '/');
    if (split_result.size() <= 1) continue;
    auto dirname = split_result.at(0); // extract dirname, get hist name is in histogram itself
    if (m_exclfolders.size() == 0) { //If none specified, canvases for all histograms
      give_canvas = true;
    } else {
      bool in_excl_folder = false;
      if (m_exclfolders.size() == 1 && m_exclfolders[0] == "all") {
        in_excl_folder = true;
      } else {
        for (auto& excl_folder : m_exclfolders) {
          if (excl_folder == dirname) {
            in_excl_folder = true;
            break;
          }
        }
      }

      if (in_excl_folder) {
        for (auto& wanted_folder : m_acfolders) {
          B2DEBUG(1, "==" << wanted_folder << "==" << dirname << "==");
          if (wanted_folder == std::string(histoname)) {
            give_canvas = true;
            break;
          }
        }
      } else {
        give_canvas = true;
      }
    }

    if (give_canvas) {
      B2DEBUG(1, "Auto Hist->Canvas for " << histoname);
      histoname.ReplaceAll("/", "_");
      std::string name = histoname.Data();
      if (m_cs.find(name) == m_cs.end()) {
        // no canvas exists yet, create one
        if (split_result.size() > 1) {
          std::string hname = split_result.at(1);
          if ((dirname + "/" + hname) == "softwaretrigger/skim") hname = "skim_hlt";
          TCanvas* c = new TCanvas((dirname + "/c_" + hname).c_str(), ("c_" + hname).c_str());
          m_cs.insert(std::pair<std::string, TCanvas*>(name, c));
        } else {
          // but this case is explicity excluded above?
          std::string hname = histoname.Data();
          TCanvas* c = new TCanvas(("c_" + hname).c_str(), ("c_" + hname).c_str());
          m_cs.insert(std::pair<std::string, TCanvas*>(name, c));
        }
      }
      TCanvas* c = m_cs[name]; // access already created canvas
      B2DEBUG(1, "DQMHistAnalysisInput: new canvas " << c->GetName());
      c->cd();

      // not so nice as we actually touch the histogram by iterator
      // we could use findHist function, but then we do another lookup within iteration
      auto hist = it.second.getHist();
      if (hist) {
        if (hist->GetDimension() == 1) {
          // assume users are expecting non-0-suppressed axis
          if (hist->GetMinimum() > 0) hist->SetMinimum(0);
          hist->Draw("hist");
        } else if (hist->GetDimension() == 2) {
          // ... but not in 2d
          hist->Draw("colz");
        }
      }

      // set Canvas "name" update flag if histo was updated
      // UpdateCanvas(name, findHist(histoname.Data()) != nullptr);
      // we already have the HistObject, so we can directly get it
      UpdateCanvas(name, it.second.isUpdated());

      // Mark Canvas as repaint needed, but is this needed?
      c->Update();
    }
  }
}
