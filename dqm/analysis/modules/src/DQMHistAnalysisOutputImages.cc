/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisOutputImages.cc
// Description : DQM Output, send Canvases to jsroot server.
//-


#include <dqm/analysis/modules/DQMHistAnalysisOutputImages.h>
#include <TROOT.h>
#include <TClass.h>
#include <TObject.h>
#include <TCanvas.h>
#include <ctime>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisOutputImages);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisOutputImagesModule::DQMHistAnalysisOutputImagesModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("CanvasSaveDefault", m_canvasSaveDefault, "Save untagged canvases", false);
  addParam("OutputPath", m_outputPath, "Set output path", std::string(""));
  addParam("asPNG", m_asPNG, "Save as PNG files", true);
  addParam("asJPEG", m_asJPEG, "Save as JPEG files", false);
  addParam("asPDF", m_asPDF, "Save as PDF files", false);
  addParam("asJSON", m_asJSON, "Save as JSON files", false);
  addParam("asROOT", m_asROOT, "Save as ROOT files", false);
  B2DEBUG(20, "DQMHistAnalysisOutputImages: Constructor done.");
}

void DQMHistAnalysisOutputImagesModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisOutputImages: event called.");

  TSeqCollection* seq = gROOT->GetListOfCanvases();
  TIter nextkey(seq);
  TObject* obj = 0;

  std::string outpath;
  if (m_outputPath != "") outpath = m_outputPath + "/"; // make sure slash is added, but only if path not empty

  auto& clist = getCanvasUpdatedList();
  int saved_canvases = 0;

  while ((obj = (TObject*)nextkey())) {
    if (obj->IsA()->InheritsFrom("TCanvas")) {
      TCanvas* c = (TCanvas*) obj;
      auto process_canvas = m_canvasSaveDefault;

      auto it = clist.find(c->GetName());
      if (it != clist.end()) {
        process_canvas = it->second;
      }
      if (!process_canvas) continue;
      saved_canvases++;

      if (m_asPNG) c->Print((outpath + c->GetName() + ".png").c_str());
      if (m_asJPEG) c->Print((outpath + c->GetName() + ".jpg").c_str());
      if (m_asPDF) c->Print((outpath + c->GetName() + ".pdf").c_str());
      if (m_asROOT) c->Print((outpath + c->GetName() + ".root").c_str());
      if (m_asJSON) c->Print((outpath + c->GetName() + ".json").c_str());
    }
  }
  B2INFO("Saved " << saved_canvases << " of " << seq->GetEntries() << " objects.");
}

