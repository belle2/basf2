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

#include <dqm/core/modules/DQMHistAutoCanvas.h>
#include <TStyle.h>
#include <fstream>

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
  addParam("IncludeFolders", m_inclfolders, "List of histograms to automatically create canvases, empty for all",
           std::vector<std::string>());
  addParam("ExcludeFolders", m_exclfolders, "List of folders to exclude from create canvases, empty for none, \"all\" for all",
           std::vector<std::string>());
  addParam("IncludeListFile", m_listfile, "List canvases to create canvases from, empty for using include/exclide folder parameter",
           std::string(""));
  B2DEBUG(1, "DQMHistAutoCanvas: Constructor done.");
}

void DQMHistAutoCanvasModule::beginRun()
{
  m_canvaslist.clear();
  if (m_listfile != "") {
    std::ifstream inputFile(m_listfile);
    // Check if the file is open
    if (!inputFile.is_open()) {
      B2WARNING("cannot open histogram list file " << m_listfile);
      return;
    }
    // Read lines from the file and store them in the vector
    std::string line;

    while (std::getline(inputFile, line)) {
      m_canvaslist.insert(line);
    }
    // Close the file
    inputFile.close();
  }
}

void DQMHistAutoCanvasModule::terminate()
{
  m_cs.clear();
}

void DQMHistAutoCanvasModule::event()
{
  gStyle->SetOptStat(0);
  gStyle->SetStatStyle(1);
  gStyle->SetOptDate(22);// Date and Time in Bottom Right, does it work?

  // There may be a more clever way instead of doing the iteration my myself here
  // This only works because the getHistList is not const anymore
  for (auto& it : getHistList()) {
    bool give_canvas = false;
    TString histoname = it.first;

    // the following lines prevent any histogram outside a directory to be processed
    auto split_result = StringSplit(histoname.Data(), '/');
    if (split_result.size() <= 1) continue;
    auto dirname = split_result.at(0); // extract dirname, get hist name is in histogram itself
    std::string cname;
    std::string hname;
    if (split_result.size() > 1) { // checked above already
      hname = split_result.at(1);
      if ((dirname + "/" + hname) == "softwaretrigger/skim") hname = "skim_hlt";
      cname = dirname + "/c_" + hname;
    } else {
      hname = histoname.Data();
      cname = "c_" + hname;
    }
    std::replace(cname.begin(), cname.end(), '.', '_');

    // Now find out if we want to have a canvas at all

    // case 1: list file
    if (m_listfile != "") {
      give_canvas = m_canvaslist.find(cname) != m_canvaslist.end();
    } else {
      // case 2: include/exclude      tname = "c_" + hname;

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
          for (auto& wanted_folder : m_inclfolders) {
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
    }

    if (give_canvas) {
      B2DEBUG(1, "Auto Hist->Canvas for " << histoname);
      histoname.ReplaceAll("/", "_");
      std::string name = histoname.Data();
      if (m_cs.find(name) == m_cs.end()) {
        // no canvas exists yet, create one
        TCanvas* c = new TCanvas(cname.c_str(), ("c_" + hname).c_str());
        m_cs.insert(std::pair<std::string, std::unique_ptr<TCanvas>>(name, c));
        B2DEBUG(1, "DQMHistAutoCanvasModule: new canvas " << c->GetName());
      }

      TCanvas* c = m_cs[name].get(); // access already created canvas
      c->cd();
      // Maybe we want to have it Cleared? Otherwise colored border could stay?
      // but if, then only if histogram has changed!

      // not so nice as we actually touch the histogram by iterator
      // we could use findHist function, but then we do another lookup within iteration
      auto hist = it.second.getHist();
      if (hist) {
        if (hist->GetDimension() == 1) {
          // assume users are expecting non-0-suppressed axis
          if (hist->GetMinimum() > 0) hist->SetMinimum(0);
          hist->Draw("hist");
          // reference only for 1dim and only if *both* not empty
          if (hist->Integral() != 0) { // ignore empty histogram
            // default scaling to number of entries
            auto refCopy = findRefHist(it.first, ERefScaling::c_RefScaleEntries, hist);
            if (refCopy and abs(refCopy->Integral()) > 0) { // only if we have entries in reference
              // Adjust the y scale to cover the reference
              if (refCopy->GetMaximum() > hist->GetMaximum())
                hist->SetMaximum(1.1 * refCopy->GetMaximum());

              refCopy->Draw("hist,same");

              /* We could consider to add some auto-comparison here later, thus adding coloring option....
                double data = 0;
                if (m_color) {
                data = hist1->KolmogorovTest(hist2, ""); // returns p value (0 bad, 1 good), N - do not compare normalized
                }
              */
            }
          }
        } else if (hist->GetDimension() == 2) {
          // ... but not in 2d
          hist->Draw("colz");
        } // else 3D or others?
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
