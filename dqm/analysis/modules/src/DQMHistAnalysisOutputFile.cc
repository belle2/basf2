/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisOutputFile.h>
#include <TROOT.h>
#include <TObject.h>
#include "TKey.h"
#include "TFile.h"
#include <sstream>
#include <iomanip>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisOutputFile);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


DQMHistAnalysisOutputFileModule::DQMHistAnalysisOutputFileModule()
  : DQMHistAnalysisModule()
{

  setDescription("Module to save histograms from DQMHistAnalysisModules");
  //Parameter definition

  addParam("OutputFolder", m_folder, "Output file path", std::string(""));
  addParam("FilePrefix", m_prefix,
           "prefix of the output filename {prefix}dqm_canvas_e####r######.root is generated (unless Filename is set)", std::string(""));
  addParam("Filename", m_filename, "name of the output file (default is {prefix}dqm_canvas_e####r######.root)", std::string(""));
  addParam("SaveHistos", m_saveHistos, "Save Histos (not default)", false);
  addParam("SaveCanvases", m_saveCanvases, "Save Canvases (default)", true);
  addParam("SavePerEvent", m_savePerEvent, "Whether save to file for each event", true);
  addParam("SavePerRun", m_savePerRun, "Whether save to file for each run (not usable in online analysis!)", false);
  B2DEBUG(20, "DQMHistAnalysisOutputFile: Constructor done.");
}


DQMHistAnalysisOutputFileModule::~DQMHistAnalysisOutputFileModule() { }

void DQMHistAnalysisOutputFileModule::initialize()
{
  m_eventMetaDataPtr.isOptional();
  B2DEBUG(20, "DQMHistAnalysisOutputFile: initialized.");
}

void DQMHistAnalysisOutputFileModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisOutputFile: event called.");
  if (m_savePerEvent) save_to_file();
}

void DQMHistAnalysisOutputFileModule::endRun()
{
  B2INFO("DQMHistAnalysisOutputFile: endRun called");
  if (m_savePerRun) save_to_file();
}


void DQMHistAnalysisOutputFileModule::terminate()
{
  B2INFO("DQMHistAnalysisOutputFile: terminate called");
}

void DQMHistAnalysisOutputFileModule::save_to_file()
{

  std::stringstream ss;
  ss << m_folder << "/";
  if (m_filename != "") ss << m_filename;
  else {
    int exp = 0;
    int run = 0;
    if (m_eventMetaDataPtr) {
      exp = m_eventMetaDataPtr->getExperiment();
      run = m_eventMetaDataPtr->getRun();
    }
    ss <<  m_prefix <<  "dqm_canvas_e";
    ss << std::setfill('0') << std::setw(4) << exp;
    ss << "r" << std::setfill('0') << std::setw(6) << run;
    ss << ".root";
  }

  TFile f(ss.str().data(), "recreate");

  if (f.IsOpen()) {
    if (m_saveCanvases) {
      TSeqCollection* seq;
      seq = gROOT->GetListOfCanvases() ;
      if (seq) {
        B2INFO("found canvases");
        TIter next(seq) ;
        TObject* obj ;
        while ((obj = (TObject*)next())) {
          if (obj->InheritsFrom("TCanvas")) {
            B2DEBUG(1, "Saving canvas " << obj->GetName());
            obj->Write();
          }
        }
      }
    }

    if (m_saveHistos) {
      TSeqCollection* files;
      files = gROOT->GetListOfFiles() ;
      if (files) {
        B2INFO("found keys");
        TIter nextfile(files) ;
        TObject* file ;

        while ((file = (TObject*)nextfile())) {
          if (file->InheritsFrom("TFile")) {
            B2INFO("File name: " << file->GetName() << " title " << file->GetTitle());
            if (file == &f || file->GetName() == m_filename) continue;

            TList* list = ((TFile*)file)->GetListOfKeys() ;
            if (list) {
              TIter next(list) ;
              TKey* key ;
              TObject* obj ;

              while ((key = (TKey*)next())) {
                TString skey(key->GetClassName());
                if (skey.BeginsWith(TString("Belle2::"))) continue;
                TClass clkey(key->GetClassName());
                if (clkey.InheritsFrom("TH1")) {
                  obj = key->ReadObj() ;
                  B2INFO("Histo name: " << obj->GetName() << " title " << obj->GetTitle());
                  TDirectory* old, *d;
                  d = old = gDirectory;
                  TString myl = obj->GetName();
                  TString tok;
                  Ssiz_t from = 0;
                  while (myl.Tokenize(tok, from, "/")) {
                    TString dummy;
                    Ssiz_t fr;
                    fr = from;
                    if (myl.Tokenize(dummy, fr,
                                     "/")) { // check if its the last one
                      auto e = d->GetDirectory(tok);
                      if (e) {
                        d = e;
                        d->cd();
                      } else {
                        d->mkdir(tok);
                        d->cd(tok);
                        d = gDirectory;
                      }
                    } else {
                      break;
                    }
                  }
                  ((TH1*)obj)->SetName(tok);
                  obj->Write();
                  old->cd();
                }
              }
            }
          } else {
            B2INFO("Others name: " << file->GetName() << " title " << file->GetTitle());
          }
        }
      }
    }

    f.Write();
    f.Close();
  }
}
