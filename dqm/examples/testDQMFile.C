/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// this is an example for storing DQM TCanvases and control variables into an output root file
// The file should containg one DQMFileMetaData object which holds information about the data processing
// Several MonitoringObjects can then be stored in the same file. For example for mumu, Dstar, etc.
// Please always set processingID in the DQMFileMetaData as it is used by the webpage to discriminate
// results from different processing.
// This example uses dummy histograms and variables so change this to read your input data.

{

  // set file meta data 
  Belle2::DQMFileMetaData* meta = new Belle2::DQMFileMetaData();
  // set database tag
  meta->setDatabaseGlobalTag("proc9");
  // set release
  meta->setRelease("release-04-01"); 
  // set run date
  meta->setRunDate("20-11-2019 07:03:01"); 
  // set experiment and run
  meta->setExperimentRun(1,2);
  // set processing id (e.g. online, proc9, proc10)
  meta->setProcessingID("online");
  
  // make monitoring objects
  // use mumu, Dstar, etc for the name
  Belle2::MonitoringObject* mon = new Belle2::MonitoringObject("arich");
  
  // make canvases for the monitoring object
  TCanvas* c2 = new TCanvas();
  c2->SetName("main");
  c2->Divide(2);
  c2->cd(1);
  TH1F* h1 = new TH1F("h1","h1",5,0,5);
  h1->Fill(1);
  h1->Draw();
  TH1F* h2 = new TH1F("h2","h2",5,0,5);
  h2->Fill(3);
  c2->cd(2);
  h2->Draw();
  
  TCanvas* c3 = new TCanvas();
  c3->SetName("resolution");
  h1->Draw();
  
  // add canvases
  mon->addCanvas(c2);
  mon->addCanvas(c3);
  
  // add float control variables
  mon->addVariable("var1",20.0);
  mon->addVariable("var2",21.0);
  mon->addVariable("var3",22.0);
  mon->addVariable("var4",23.0);

  // add string control variables
  std::string comment("This is very nice run");
  mon->addVariable("comment", comment);

  // one can add more monitoring objects ....
    
  // write file
  f = new TFile("test_e0001r00002.root","RECREATE");
  meta->Write();
  mon->Write();
  f->Close();
  
}
