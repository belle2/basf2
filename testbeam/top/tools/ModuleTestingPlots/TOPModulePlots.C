/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matthew Barrett                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
 
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "TFile.h"
#include "TChain.h"
#include "TTree.h"
#include "TSystem.h"
#include "TH1.h"
#include "TH2.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TApplication.h"
#include "TString.h"
#include "TMath.h"
#include "TCanvas.h"
#include <map>
#include <vector>

#include "topEvent.hh"
//Set only one of the following two lines to "true" based on the input ROOT format being used:
//Please also modify the equivalent declaration in event.hh
bool useFall2015ROOTFormat() {return true;}
bool useTopcafROOTFormat()   {return false;}


bool verbose() {return false;}
void setUpChain(TString fileName, TChain* chain);
void setupBranches(TChain* tree, topEvent &event);
std::pair<Int_t, Int_t> getRunRange(TChain* chain, topEvent &event);
void plotHistograms(std::map<TString, TH1F*> &histograms, TCanvas* canvas);
void writeHistograms(std::map<TString, TH1F*> &histograms);
void write2DHistograms(std::map<TString, TH2F*> &histograms);
void createIntermediateHistograms(std::map<TString, TH1F*> &histograms, TString identifier);
void fillIntermediateHistograms(TChain* chain, topEvent &event, std::map<TString, TH1F*> &histograms);
void createRatioHistograms(std::map<TString, TH1F*> &histograms);
void fillRatioHistograms(std::map<TString, TH1F*> &ratioHistograms, std::map<TString, TH1F*> &histograms1, std::map<TString, TH1F*> &histograms2);
void setRatioHistogramTitles(std::map<TString, TH1F*> &ratioHistograms, std::pair<Int_t, Int_t> &runRange1, std::pair<Int_t, Int_t> &runRange2);
TString runRangeForTitle(std::pair<Int_t, Int_t> &runRange);
TString combinedRunRangeForTitle(std::pair<Int_t, Int_t> &runRange1, std::pair<Int_t, Int_t> &runRange2);
void createOccupancyHistograms(std::map<TString, TH2F*> &histograms, TString identifier);
void fillOccupancyHistograms(TChain* chain, topEvent &event, std::map<TString, TH2F*> &histograms);
void createOccupancyRatioHistograms(std::map<TString, TH2F*> &histograms1, std::map<TString, TH2F*> &histograms2);
void setOccupancyRatioHistogramTitles(std::map<TString, TH2F*> &histograms1, std::map<TString, TH2F*> &histograms2, std::pair<Int_t, Int_t> &runRange1, std::pair<Int_t, Int_t> &runRange2);


void TOPModulePlots(TString file1, TString file2, TString outputFileName) {

  bool makePngs(false);  //make png files in plots directory
  bool makeAllPngs(false); // make lots of png files in plots directory
  bool writeAllHistograms(true); //Save many histograms to ROOT file

  TString treeName;
  if (useFall2015ROOTFormat()) {treeName = "top";}
  if (useTopcafROOTFormat())   {treeName = "Event";}

  TChain* tree1 = new TChain(treeName);
  TChain* tree2 = new TChain(treeName);

  std::cout << "Setting up first input data:" << std::endl;
  setUpChain(file1, tree1);
  std::cout << "Setting up second input data:" << std::endl;
  setUpChain(file2, tree2);


  topEvent event1;
  topEvent event2;  
  
  setupBranches(tree1, event1);
  setupBranches(tree2, event2);

  std::pair<Int_t, Int_t> runRange1 = getRunRange(tree1, event1);
  std::pair<Int_t, Int_t> runRange2 = getRunRange(tree2, event2);

  TFile* outputFile = new TFile(outputFileName, "recreate");

  std::map<TString, TH1F*> histograms1;
  std::map<TString, TH1F*> histograms2;

  std::map<TString, TH1F*> ratioHistograms;
  createRatioHistograms(ratioHistograms);

  createIntermediateHistograms(histograms1, "1");
  createIntermediateHistograms(histograms2, "2");

  fillIntermediateHistograms(tree1, event1, histograms1);  
  fillIntermediateHistograms(tree2, event2, histograms2);
 
  fillRatioHistograms(ratioHistograms, histograms1, histograms2);
  setRatioHistogramTitles(ratioHistograms, runRange1, runRange2);


  // Occupancy histograms
  std::map<TString, TH2F*> occupancyHistograms1;  
  std::map<TString, TH2F*> occupancyHistograms2;

  createOccupancyHistograms(occupancyHistograms1, "1");
  createOccupancyHistograms(occupancyHistograms2, "2");  

  fillOccupancyHistograms(tree1, event1, occupancyHistograms1);
  fillOccupancyHistograms(tree2, event2, occupancyHistograms2);

  createOccupancyRatioHistograms(occupancyHistograms1, occupancyHistograms2);
  setOccupancyRatioHistogramTitles(occupancyHistograms1, occupancyHistograms2, runRange1, runRange2);


  //Make plots and save histograms to ROOT file:
  TCanvas* c1 = new TCanvas("c1", "c1");  
   
  if (makePngs) { 
    plotHistograms(ratioHistograms, c1);
    if (makeAllPngs) {
      plotHistograms(histograms1, c1);    
      plotHistograms(histograms2, c1);
    }
  }

 
  writeHistograms(ratioHistograms);   
  write2DHistograms(occupancyHistograms1); 
  write2DHistograms(occupancyHistograms2); 

  if (writeAllHistograms) {
    writeHistograms(histograms1);
    writeHistograms(histograms2);
  }


  delete tree1;
  delete tree2;
  
  for (std::map<TString, TH1F*>::iterator iter(histograms1.begin()); iter != histograms1.end(); ++iter) {delete iter->second;}
  for (std::map<TString, TH1F*>::iterator iter(histograms2.begin()); iter != histograms2.end(); ++iter) {delete iter->second;}
  for (std::map<TString, TH1F*>::iterator iter(ratioHistograms.begin()); iter != ratioHistograms.end(); ++iter) {delete iter->second;}

  delete outputFile;


} //end of TOPModulePlots()


// Function definitions:

void setUpChain(TString fileName, TChain* chain) {

  if (fileName.Contains(".root")) {
    std::cout << "Using file: " << fileName << std::endl;    
    chain->Add(fileName);
  } else if (fileName.Contains(".txt"))  {
    ifstream fileList;
    fileList.open(fileName.Data());
    if (fileList.fail()) {std::cerr << "Error: could not open file " << fileName << std::endl;}

    TString fileToAdd;
    while (!fileList.eof()) {
      fileList >> fileToAdd;
      if (!fileToAdd.IsWhitespace()) {
        std::cout << "Adding file: " << fileToAdd << std::endl; 
        chain->Add(fileToAdd);       
      }
    }     
  } else {
    std::cerr << "Error: please provide a file name that is either a ROOT file \"*.root\" or a text file \"*.txt\" that contains a list of ROOT files." << std::endl;    
    gApplication->Terminate();
  }
  
} //end of setUpChain 


void setupBranches(TChain* tree, topEvent &event) {  
  /**
    * Function to set up branches of tree, based on the type of input ROOT files being used.
    */

  if (useFall2015ROOTFormat()) {
    tree->SetBranchAddress("nhit", &event.nHit);
    tree->SetBranchAddress("runNum", &event.runNumber);    
    tree->SetBranchAddress("eventNum", &event.eventNumber);
    tree->SetBranchAddress("ch", &event.pixel);
    tree->SetBranchAddress("tdc", &event.time); //for now
    tree->SetBranchAddress("tdc", &event.tdc);
    tree->SetBranchAddress("adc", &event.charge);
    tree->SetBranchAddress("pmt", &event.pmt);
  }

  if (useTopcafROOTFormat()) {
    tree->SetBranchAddress("nhit", &event.nHit);
    tree->SetBranchAddress("runNum", &event.runNumber);    
    tree->SetBranchAddress("eventNum", &event.eventNumber);
    tree->SetBranchAddress("pixel", &event.pixel);
    tree->SetBranchAddress("fine_time", &event.time);
    tree->SetBranchAddress("tdc", &event.tdc);
    tree->SetBranchAddress("amplitude", &event.charge);
    tree->SetBranchAddress("pmt", &event.pmt);    
  }

} //end of setUpBranches


std::pair<Int_t, Int_t> getRunRange(TChain* chain, topEvent &event) {

  Int_t runMin(999999);
  Int_t runMax(0);
  Long64_t nEvents(chain->GetEntries());

  chain->SetBranchStatus("*", 0);
  chain->SetBranchStatus("runNum", 1);
  
  for (Long64_t iEvent(0); iEvent < nEvents; ++iEvent) {
    chain->GetEntry(iEvent);
    if (event.runNumber < runMin) {runMin = event.runNumber;} 
    if (event.runNumber > runMax) {runMax = event.runNumber;}
  }

  chain->SetBranchStatus("*", 1);

  std::cout << "runMin = " << runMin << "\trunMax = " << runMax << std::endl; 

  return std::pair<Int_t, Int_t>(runMin, runMax);

} //end of getRunRange


void plotHistograms(std::map<TString, TH1F*> &histograms, TCanvas* canvas) {

  for (std::map<TString, TH1F*>::iterator iter(histograms.begin()); iter != histograms.end(); ++iter) {
    TString saveName("plots/");
    saveName += iter->first;
    saveName += ".png";
    
    std::cout << "Plotting " << saveName << std::endl;
    iter->second->Draw("e");
    canvas->SaveAs(saveName);    
    
  }  
} //end of plotHistograms


void writeHistograms(std::map<TString, TH1F*> &histograms) {
  for (std::map<TString, TH1F*>::iterator iter(histograms.begin()); iter != histograms.end(); ++iter) {
    iter->second->Write();   
  }  
} //end of writeHistograms


void write2DHistograms(std::map<TString, TH2F*> &histograms) {
  for (std::map<TString, TH2F*>::iterator iter(histograms.begin()); iter != histograms.end(); ++iter) {
    iter->second->Write();   
  }  
} //end of write2DHistograms


void createIntermediateHistograms(std::map<TString, TH1F*> &histograms, TString identifier) {

  //identifier is a string to ensure that each TH1F has a unique name for ROOT's internal memory management, and when written to a ROOT file.

  TString key = "eventNumber";
  TString name = key + identifier;
  histograms[key] = new TH1F(name, key, 100, 0, 10000);

  for (Int_t iPMT(1); iPMT < 33; ++iPMT) {
    TString pmtString("_pmt");
    pmtString += iPMT;

    key = "time" + pmtString;
    name = key + "_" + identifier;
    if (verbose()) {std::cout << "Creating histogram " << key << std::endl;}
    histograms[key] = new TH1F(name, key, 100, 0, 1100);

    key = "charge" + pmtString;
    name = key + identifier;
    histograms[key] = new TH1F(name, key, 100, 0, 5000);

  } //loop over PMT numbers

} //end of createIntermediateHistograms


void fillIntermediateHistograms(TChain* chain, topEvent &event, std::map<TString, TH1F*> &histograms) {


  Long64_t nEvents(chain->GetEntries());
  
  for (Long64_t iEvent(0); iEvent < nEvents; ++iEvent) {
    chain->GetEntry(iEvent);
    histograms["eventNumber"]->Fill(event.eventNumber);
        
    Int_t nHit(event.nHit);
    for (Int_t iHit(0); iHit < nHit; ++iHit) {


        TString fillName("time_pmt");
        fillName += event.pmt[iHit];
        histograms[fillName]->Fill(event.tdc[iHit]);

        fillName = "charge_pmt";
        fillName += event.pmt[iHit];
        histograms[fillName]->Fill(event.charge[iHit]);

    }//nHit loop
    
  } // nEvent loop

} //end of fillIntermediateHistograms


void createRatioHistograms(std::map<TString, TH1F*> &histograms) {

  // time RMS histograms
  histograms["timeRMS_1"] = new TH1F("timeRMS_1", "timeRMS_1", 32, 1, 33);
  histograms["timeRMS_2"] = new TH1F("timeRMS_2", "timeRMS_2", 32, 1, 33);
  histograms["timeRMS_1"]->SetXTitle("PMT number");
  histograms["timeRMS_1"]->SetYTitle("time RMS / ns");
  histograms["timeRMS_2"]->SetXTitle("PMT number");
  histograms["timeRMS_2"]->SetYTitle("time RMS / ns");

  // charge mean histograms
  histograms["chargeMean_1"] = new TH1F("chargeMean_1", "chargeMean_1", 32, 1, 33);
  histograms["chargeMean_2"] = new TH1F("chargeMean_2", "chargeMean_2", 32, 1, 33);
  histograms["chargeMean_1"]->SetXTitle("PMT number");
  histograms["chargeMean_1"]->SetYTitle("mean charge");
  histograms["chargeMean_2"]->SetXTitle("PMT number");
  histograms["chargeMean_2"]->SetYTitle("mean charge");

  //charge RMS histograms
  histograms["chargeRMS_1"] = new TH1F("chargeRMS_1", "chargeRMS_1", 32, 1, 33);
  histograms["chargeRMS_2"] = new TH1F("chargeRMS_2", "chargeRMS_2", 32, 1, 33);
  histograms["chargeRMS_1"]->SetXTitle("PMT number");
  histograms["chargeRMS_1"]->SetYTitle("RMS charge");
  histograms["chargeRMS_2"]->SetXTitle("PMT number");
  histograms["chargeRMS_2"]->SetYTitle("RMS charge");
  histograms["chargeRMS_2"]->SetOption("e");

} // end of createRatioHistograms


void fillRatioHistograms(std::map<TString, TH1F*> &ratioHistograms, std::map<TString, TH1F*> &histograms1, std::map<TString, TH1F*> &histograms2) {

  for (Int_t iPMT(1); iPMT < 33; ++iPMT) {
    TString histoName("charge_pmt");
    histoName += iPMT;
    ratioHistograms["chargeMean_1"]->Fill(iPMT, histograms1[histoName]->GetMean());  
    ratioHistograms["chargeMean_2"]->Fill(iPMT, histograms2[histoName]->GetMean());  
    ratioHistograms["chargeMean_1"]->SetBinError(iPMT, histograms1[histoName]->GetRMS());   
    ratioHistograms["chargeMean_2"]->SetBinError(iPMT, histograms2[histoName]->GetRMS());

    ratioHistograms["chargeRMS_1"]->Fill(iPMT, histograms1[histoName]->GetRMS());  
    ratioHistograms["chargeRMS_2"]->Fill(iPMT, histograms2[histoName]->GetRMS());

    histoName = "time_pmt";
    histoName += iPMT;
    ratioHistograms["timeRMS_1"]->Fill(iPMT, histograms1[histoName]->GetRMS());  
    ratioHistograms["timeRMS_2"]->Fill(iPMT, histograms2[histoName]->GetRMS());    



    
  }
  
  ratioHistograms["chargeMean_ratio"] = (TH1F*) ratioHistograms["chargeMean_2"]->Clone("chargeMean_ratio");
  ratioHistograms["chargeMean_ratio"]->SetYTitle("ratio"); 
  ratioHistograms["chargeMean_ratio"]->Divide(ratioHistograms["chargeMean_1"]);

  ratioHistograms["chargeRMS_ratio"] = (TH1F*) ratioHistograms["chargeMean_2"]->Clone("chargeRMS_ratio");
  ratioHistograms["chargeRMS_ratio"]->SetYTitle("ratio"); 
  ratioHistograms["chargeRMS_ratio"]->Divide(ratioHistograms["chargeMean_1"]);

  ratioHistograms["timeRMS_ratio"] = (TH1F*) ratioHistograms["timeRMS_2"]->Clone("timeRMS_ratio");
  ratioHistograms["timeRMS_ratio"]->SetYTitle("ratio"); 
  ratioHistograms["timeRMS_ratio"]->Divide(ratioHistograms["timeRMS_1"]);

} //end of fillRatiohistograms


void setRatioHistogramTitles(std::map<TString, TH1F*> &ratioHistograms, std::pair<Int_t, Int_t> &runRange1, std::pair<Int_t, Int_t> &runRange2) {

  TString runTitle1     = runRangeForTitle(runRange1);
  TString runTitle2     = runRangeForTitle(runRange2);
  TString runTitleRatio = combinedRunRangeForTitle(runRange1, runRange2);

  ratioHistograms["chargeMean_1"]->SetTitle("Means of charge distributions " + runTitle1);
  ratioHistograms["chargeRMS_1"]->SetTitle("RMS of charge distributions " + runTitle1);
  ratioHistograms["timeRMS_1"]->SetTitle("Means of charge distributions " + runTitle1);
  ratioHistograms["chargeMean_2"]->SetTitle("Means of charge distributions " + runTitle2);
  ratioHistograms["chargeRMS_2"]->SetTitle("RMS of charge distributions " + runTitle2);
  ratioHistograms["timeRMS_2"]->SetTitle("Means of charge distributions " + runTitle2);
  ratioHistograms["chargeMean_ratio"]->SetTitle("Ratio of means of charge distributions " + runTitleRatio);
  ratioHistograms["chargeRMS_ratio"]->SetTitle("Ratio of RMS of charge distributions " + runTitleRatio);
  ratioHistograms["timeRMS_ratio"]->SetTitle("Ratio of means of charge distributions " + runTitleRatio);

} //end of setRatioHistogramTitles


TString runRangeForTitle(std::pair<Int_t, Int_t> &runRange) {

  TString runTitle("");
  if (runRange.first == runRange.second) {
    runTitle = "for Run ";
    runTitle += runRange.first;
  } else {
    runTitle = "for Runs "; 
    runTitle += runRange.first;
    runTitle += " to "; 
    runTitle += runRange.second;
  }
  return runTitle;
} //end of runRangeForTitle


TString combinedRunRangeForTitle(std::pair<Int_t, Int_t> &runRange1, std::pair<Int_t, Int_t> &runRange2) {

  TString runTitle("between ");
  if (runRange1.first == runRange1.second) {
    runTitle += "Run ";
    runTitle += runRange1.first;
  } else {
    runTitle += "Runs ("; 
    runTitle += runRange1.first;
    runTitle += " to "; 
    runTitle += runRange1.second;
    runTitle += ")";
  }
  if (runRange2.first == runRange2.second) {
    runTitle += " and Run ";
    runTitle += runRange2.first;
  } else {
    runTitle += " and Runs ("; 
    runTitle += runRange2.first;
    runTitle += " to "; 
    runTitle += runRange2.second;
    runTitle += ")";
  }
  return runTitle;

} //end of combinedRunRangeForTitle


void createOccupancyHistograms(std::map<TString, TH2F*> &histograms, TString identifier) {

  //identifier is a string to ensure that each TH2F has a unique name for ROOT's internal memory management, and when written to a ROOT file.

  TString key  = "pmtOccupancy";
  TString name = key + identifier;
  histograms[key] = new TH2F(name, key, 16, 1, 17, 2, 1, 3);
  histograms[key]->SetOption("colz");
  histograms[key]->GetXaxis()->SetNdivisions(16, kFALSE);
  histograms[key]->GetYaxis()->SetNdivisions(2, kFALSE);
  histograms[key]->SetXTitle("PMT column");
  histograms[key]->SetYTitle("PMT row");  

  key  = "pixelOccupancy";
  name = key + identifier;
  histograms[key] = new TH2F(name, key, 64, 1, 65, 8, 1, 9);  
  histograms[key]->SetOption("colz");
  histograms[key]->GetXaxis()->SetNdivisions(16, kFALSE);
  histograms[key]->GetYaxis()->SetNdivisions(402, kFALSE);  
  histograms[key]->SetXTitle("pixel column");
  histograms[key]->SetYTitle("pixel row");

   
}  //end of createOccupancyHistograms

void fillOccupancyHistograms(TChain* chain, topEvent &event, std::map<TString, TH2F*> &histograms) {

  Long64_t nEvents(chain->GetEntries());
  for (Long64_t iEvent(0); iEvent < nEvents; ++iEvent) {
    chain->GetEntry(iEvent);
    
    Int_t nHit(event.nHit);
    for (Int_t iHit(0); iHit < nHit; ++iHit) {

       Int_t pmtRow((event.pmt[iHit] - 1) / 16 + 1);
       Int_t pmtCol((event.pmt[iHit] - 1) % 16 + 1);
       histograms["pmtOccupancy"]->Fill(pmtCol, pmtRow); 

       Int_t pixelRow((event.pixel[iHit] - 1) / 64 + 1);
       Int_t pixelCol((event.pixel[iHit] - 1) % 64 + 1);
       histograms["pixelOccupancy"]->Fill(pixelCol, pixelRow);

    }
  }    
} // end of fillOccupancyHsitograms 

void createOccupancyRatioHistograms(std::map<TString, TH2F*> &histograms1, std::map<TString, TH2F*> &histograms2) {

  histograms1["pmtOccupancy"]->Scale(1.0 / histograms1["pmtOccupancy"]->GetEntries());
  histograms2["pmtOccupancy"]->Scale(1.0 / histograms2["pmtOccupancy"]->GetEntries());
  histograms1["pixelOccupancy"]->Scale(1.0 / histograms1["pixelOccupancy"]->GetEntries());
  histograms2["pixelOccupancy"]->Scale(1.0 / histograms2["pixelOccupancy"]->GetEntries());
  
  
  histograms1["pmtOccupancyRatio"]   = (TH2F*) histograms2["pmtOccupancy"]->Clone("pmtOccupancyRatio");
  histograms1["pixelOccupancyRatio"] = (TH2F*) histograms2["pixelOccupancy"]->Clone("pixelOccupancyRatio");  
 
  histograms1["pmtOccupancyRatio"]->Divide(histograms1["pmtOccupancy"]);
  histograms1["pixelOccupancyRatio"]->Divide(histograms1["pixelOccupancy"]);

  histograms1["pmtOccupancyRatio"]->SetMinimum(0.0);
  histograms1["pmtOccupancyRatio"]->SetMaximum(2.0);
  histograms1["pixelOccupancyRatio"]->SetMinimum(0.0);
  histograms1["pixelOccupancyRatio"]->SetMaximum(2.0);


} // end of createOccupancyRatioHistograms

void setOccupancyRatioHistogramTitles(std::map<TString, TH2F*> &histograms1, std::map<TString, TH2F*> &histograms2, std::pair<Int_t, Int_t> &runRange1, std::pair<Int_t, Int_t> &runRange2) {

  TString runTitle1     = runRangeForTitle(runRange1);
  TString runTitle2     = runRangeForTitle(runRange2);
  TString runTitleRatio = combinedRunRangeForTitle(runRange1, runRange2);

  histograms1["pmtOccupancy"]->SetTitle("PMT occupancy " + runTitle1);
  histograms2["pmtOccupancy"]->SetTitle("PMT occupancy " + runTitle2);
  histograms1["pixelOccupancy"]->SetTitle("Pixel occupancy " + runTitle1);
  histograms2["pixelOccupancy"]->SetTitle("Pixel occupancy " + runTitle1);
  histograms1["pmtOccupancyRatio"]->SetTitle("PMT occupancy ratio " + runTitleRatio);
  histograms1["pixelOccupancyRatio"]->SetTitle("Pixel occupancy ratio " + runTitleRatio);

} //end of setOccupancyRatioHistogramTitles
