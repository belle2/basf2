//+
// File : DQMHistAnalysisOutputFile.cc
// Description : DQM Analysis, dump histograms to file (as reference histograms)
//
// Author : B. Spruck
// Date : 25 - Mar - 2017
// based on work from Tomoyuki Konno, Tokyo Metropolitan Univerisity
//-


#include <dqm/analysis/modules/DQMHistAnalysisOutputFile.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TClass.h>
#include <TObject.h>
#include <TCanvas.h>
#include <TMessage.h>
#include "TKey.h"
#include "TIterator.h"
#include "TFile.h"

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisOutputFile)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisOutputFileModule::DQMHistAnalysisOutputFileModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistoFile", m_filename, "Output Histogram Filename", std::string("histo.root"));
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("ref"));
  addParam("SaveHistos", m_saveHistos, "Save Histos (default)", true);
  addParam("SaveCanvases", m_saveCanvases, "Save Canvases (not default)", false);
  B2DEBUG(20, "DQMHistAnalysisOutputFile: Constructor done.");
}


DQMHistAnalysisOutputFileModule::~DQMHistAnalysisOutputFileModule() { }

void DQMHistAnalysisOutputFileModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisOutputFile: initialized.");
}


void DQMHistAnalysisOutputFileModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisOutputFile: beginRun called.");
}


void DQMHistAnalysisOutputFileModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisOutputFile: event called.");
}

void DQMHistAnalysisOutputFileModule::endRun()
{
  B2INFO("DQMHistAnalysisOutputFile: endRun called");

  // Attention, we can not do that in Terminate, as then the memFile is already closed by previous task!
  B2INFO("open file");
  TFile f(m_filename.data(), "recreate");
  if (f.IsOpen()) {
    TDirectory* oldDir = gDirectory;
    oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

    if (m_saveCanvases) {
      // we could loop over histos ... but i think the second one is better
      TSeqCollection* seq;
      seq = gROOT->GetListOfCanvases() ;
      if (seq) {
        B2INFO("found canvases");
        TIter next(seq) ;
        TObject* obj ;

        while ((obj = (TObject*)next())) {
          if (obj->InheritsFrom("TCanvas")) {
            B2INFO("Canvas name: " << obj->GetName() << " title " << obj->GetTitle());
            obj->Write();
          } else {
            B2INFO("Others name: " << obj->GetName() << " title " << obj->GetTitle());
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
                obj = key->ReadObj() ;
                if (obj->InheritsFrom("TH1")) {
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
                    if (myl.Tokenize(dummy, fr, "/")) { // check if its the last one
                      auto e = d->GetDirectory(tok);
                      if (e) {
                        d = e;
                      } else {
                        d = d->mkdir(tok);
                      }
                      d->cd();
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


void DQMHistAnalysisOutputFileModule::terminate()
{
  B2INFO("DQMHistAnalysisOutputFile: terminate called");
// Attention, we can not do that in Terminate, as then the memFile is already closed by previous task!
}

