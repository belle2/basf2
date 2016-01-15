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
#include "TMath.h"
#include "TROOT.h"
#include "TApplication.h"
#include "TString.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TCanvas.h"
#include <map>
#include <vector>

#include "topEvent.hh"
//Set only one of the following three lines to "true" based on the input ROOT format being used:
//Please also modify the equivalent declaration in event.hh
bool useFall2015ROOTFormat()  {return false;}
bool useTopcafROOTFormat()    {return false;}
bool usePocketDAQROOTFormat() {return true;}

//Basic histogram bin and range settings:
Float_t getTimeHistogramNBins() {return 100;}
Float_t getTimeRangeLow()       {return 0.0;}
Float_t getTimeRangeHigh()      {return 1100.0;}
Float_t timeUnit()              {return 1.0;} // 1.0 = ns
Float_t minReflectedDelay()     {return 20.0;} // ns - minimum time after main peak to start looking for reflected peak;
Float_t timeWidthForPeaks()     {return 8.0;} // ns - width of region to integrate around peaks


Float_t getChargeHistogramNBins() {return 100;}
Float_t getChargeRangeLow()       {return 0.0;}
Float_t getChargeRangeHigh()      {return 5000.0;}
Float_t chargeUnit()              {return 1.0;}


bool verbose() {return false;}
//bool verbose() {return true;}

void setUpChain(TString fileName, TChain* chain);
void setupBranches(TChain* tree, topEvent &event);
std::pair<Int_t, Int_t> getRunRange(TChain* chain, topEvent &event);
void plotHistograms(TString type, std::map<TString, TH1F*> &histograms, TCanvas* canvas);
void writeHistograms(std::map<TString, TH1F*> &histograms);
void write2DHistograms(std::map<TString, TH2F*> &histograms);
void createIntermediateHistograms(std::map<TString, TH1F*> &histograms, TString type, TString identifier);
void fillIntermediateHistograms(TChain* chain, topEvent &event, std::map<TString, TH1F*> &histograms, TString type);
void createRatioHistograms(TString type, std::map<TString, TH1F*> &histograms);
void fillRatioHistograms(TString type, std::map<TString, TH1F*> &ratioHistograms, std::map<TString, TH1F*> &histograms1, std::map<TString, TH1F*> &histograms2, Int_t nEvents1, Int_t nEvents2);
void setRatioHistogramTitles(TString type, std::map<TString, TH1F*> &ratioHistograms, std::pair<Int_t, Int_t> &runRange1, std::pair<Int_t, Int_t> &runRange2);
TString runRangeForTitle(std::pair<Int_t, Int_t> &runRange);
TString combinedRunRangeForTitle(std::pair<Int_t, Int_t> &runRange1, std::pair<Int_t, Int_t> &runRange2);
void createOccupancyHistograms(std::map<TString, TH2F*> &histograms, TString identifier);
void fillOccupancyHistograms(TChain* chain, topEvent &event, std::map<TString, TH2F*> &histograms);
void createOccupancyRatioHistograms(std::map<TString, TH2F*> &histograms1, std::map<TString, TH2F*> &histograms2);
void setOccupancyRatioHistogramTitles(std::map<TString, TH2F*> &histograms1, std::map<TString, TH2F*> &histograms2, std::pair<Int_t, Int_t> &runRange1, std::pair<Int_t, Int_t> &runRange2);
std::pair<Int_t, Int_t> findTimingPeaks(TH1F* histogram);
Float_t getIntegralAroundPeak(Int_t iBin, TH1F* histogram);
Float_t getRMSAroundPeak(Int_t iBin, TH1F* histogram);




void TOPModulePlots(TString file1, TString file2, TString outputFileName) {

  bool makePngs(true);  //make png files in plots directory
  bool makeAllPngs(false); // make lots of png files in plots directory
  bool writeAllHistograms(true); //Save many histograms to ROOT file



  TString treeName;
  if (useFall2015ROOTFormat())  {treeName = "top";}
  if (useTopcafROOTFormat())    {treeName = "Event";}
  if (usePocketDAQROOTFormat()) {treeName = "???";}


  TChain* tree1 = new TChain(treeName);
  TChain* tree2 = new TChain(treeName);

  std::cout << "Setting up first input data:" << std::endl;
  setUpChain(file1, tree1);
  std::cout << "Setting up second input data:" << std::endl;
  setUpChain(file2, tree2);

  Int_t nEvents1(tree1->GetEntries());
  Int_t nEvents2(tree2->GetEntries());

  topEvent event1;
  topEvent event2;  
  
  setupBranches(tree1, event1);
  setupBranches(tree2, event2);

  std::pair<Int_t, Int_t> runRange1 = getRunRange(tree1, event1);
  std::pair<Int_t, Int_t> runRange2 = getRunRange(tree2, event2);

  TFile* outputFile = new TFile(outputFileName, "recreate");

  std::map<TString, TH1F*> pmtHistograms1;
  std::map<TString, TH1F*> pmtHistograms2;
  std::map<TString, TH1F*> pixelHistograms1;
  std::map<TString, TH1F*> pixelHistograms2;


  std::map<TString, TH1F*> pmtRatioHistograms;
  createRatioHistograms("pmt", pmtRatioHistograms);

  std::map<TString, TH1F*> pixelRatioHistograms;
  createRatioHistograms("pixel", pixelRatioHistograms);


  if (verbose()) {std::cout << "Creating pmt histograms" << std::endl;}
  createIntermediateHistograms(pmtHistograms1, "pmt", "1");
  createIntermediateHistograms(pmtHistograms2, "pmt", "2");

  if (verbose()) {std::cout << "Filling pmt histograms" << std::endl;}
  fillIntermediateHistograms(tree1, event1, pmtHistograms1, "pmt");  
  fillIntermediateHistograms(tree2, event2, pmtHistograms2, "pmt");

  if (verbose()) {std::cout << "Creating pixel histograms" << std::endl;}
  createIntermediateHistograms(pixelHistograms1, "pixel", "1");
  createIntermediateHistograms(pixelHistograms2, "pixel", "2");

  if (verbose()) {std::cout << "Filling pixel histograms" << std::endl;}
  fillIntermediateHistograms(tree1, event1, pixelHistograms1, "pixel");  
  fillIntermediateHistograms(tree2, event2, pixelHistograms2, "pixel");

  if (verbose()) {std::cout << "Creating PMT ratio histograms" << std::endl;}
  fillRatioHistograms("pmt", pmtRatioHistograms, pmtHistograms1, pmtHistograms2, nEvents1, nEvents2);
  setRatioHistogramTitles("pmt", pmtRatioHistograms, runRange1, runRange2);

  if (verbose()) {std::cout << "Creating pixel ratio histograms" << std::endl;}
  fillRatioHistograms("pixel", pixelRatioHistograms, pixelHistograms1, pixelHistograms2, nEvents1, nEvents2);
  setRatioHistogramTitles("pixel", pixelRatioHistograms, runRange1, runRange2);



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
    plotHistograms("pmt", pmtRatioHistograms, c1);
    plotHistograms("pixel", pixelRatioHistograms, c1);
    if (makeAllPngs) {
      plotHistograms("pmt", pmtHistograms1, c1);    
      plotHistograms("pmt", pmtHistograms2, c1);
      plotHistograms("pixel", pixelHistograms1, c1);    
      plotHistograms("pixel", pixelHistograms2, c1);
    }
  }

  outputFile->mkdir("HistogramsByPMT");
  outputFile->cd("HistogramsByPMT"); 
  writeHistograms(pmtRatioHistograms);
  outputFile->cd();   

  outputFile->mkdir("HistogramsByPixel");
  outputFile->cd("HistogramsByPixel"); 
  writeHistograms(pixelRatioHistograms);
  outputFile->cd();




  write2DHistograms(occupancyHistograms1); 
  write2DHistograms(occupancyHistograms2); 

  if (writeAllHistograms) {
    outputFile->mkdir("ExpertByPMT");
    outputFile->cd("ExpertByPMT");
    writeHistograms(pmtHistograms1);
    writeHistograms(pmtHistograms2);
    outputFile->cd();

    outputFile->mkdir("ExpertByPixel");
    outputFile->cd("ExpertByPixel");
    writeHistograms(pixelHistograms1);
    writeHistograms(pixelHistograms2);
    outputFile->cd();
  }


  delete tree1;
  delete tree2;
  
  for (std::map<TString, TH1F*>::iterator iter(pmtHistograms1.begin()); iter != pmtHistograms1.end(); ++iter) {delete iter->second;}
  for (std::map<TString, TH1F*>::iterator iter(pmtHistograms2.begin()); iter != pmtHistograms2.end(); ++iter) {delete iter->second;}
  for (std::map<TString, TH1F*>::iterator iter(pixelHistograms1.begin()); iter != pixelHistograms1.end(); ++iter) {delete iter->second;}
  for (std::map<TString, TH1F*>::iterator iter(pixelHistograms2.begin()); iter != pixelHistograms2.end(); ++iter) {delete iter->second;}

  for (std::map<TString, TH1F*>::iterator iter(pmtRatioHistograms.begin()); iter != pmtRatioHistograms.end(); ++iter) {delete iter->second;}
  for (std::map<TString, TH1F*>::iterator iter(pixelRatioHistograms.begin()); iter != pixelRatioHistograms.end(); ++iter) {delete iter->second;}

  delete outputFile;


} //end of makePlots()


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
    tree->SetBranchAddress("tdc", &event.time);
    tree->SetBranchAddress("tdc", &event.tdc);
    tree->SetBranchAddress("adc", &event.charge);
    tree->SetBranchAddress("pmt", &event.pmt);
  }

  if (useTopcafROOTFormat()) {
    tree->SetBranchAddress("nhit", &event.nHit);
    tree->SetBranchAddress("runNum", &event.runNumber);    
    tree->SetBranchAddress("eventNum", &event.eventNumber);
    tree->SetBranchAddress("G4channel_id", &event.pixel);
    tree->SetBranchAddress("fine_time", &event.time);
    tree->SetBranchAddress("coarse_time", &event.tdc);
    tree->SetBranchAddress("amplitude", &event.charge);
    tree->SetBranchAddress("pmt", &event.pmt);    
  }
  if (usePocketDAQROOTFormat()) {
    tree->SetBranchAddress("nhit", &event.nHit);
    tree->SetBranchAddress("runNum", &event.runNumber);    
    tree->SetBranchAddress("eventNum", &event.eventNumber);
    tree->SetBranchAddress("pixel", &event.pixel);
    tree->SetBranchAddress("t", &event.time); 
    tree->SetBranchAddress("q", &event.charge);
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


void plotHistograms(TString type, std::map<TString, TH1F*> &histograms, TCanvas* canvas) {

  for (std::map<TString, TH1F*>::iterator iter(histograms.begin()); iter != histograms.end(); ++iter) {
    TString saveName("plots/");
    saveName += type;
    saveName += "/";
    saveName += iter->first;
    saveName += ".png";
    
    if (verbose()) {std::cout << "Plotting " << saveName << std::endl;}
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


void createIntermediateHistograms(std::map<TString, TH1F*> &histograms, TString type, TString identifier) {

  //identifier is a string to ensure that each TH1F has a unique name for ROOT's internal memory management, and when written to a ROOT file.

  if (type != "pmt" && type != "pixel") {
    std::cerr << "parameter \"type\" for function createIntermediateHistograms must be either \"pmt\" or \"pixel\"." << std::endl;
  }

  TString key = "eventNumber";
  TString name = key + identifier;
  histograms[key] = new TH1F(name, key, 100, 0, 10000);

  Int_t nEntities(0);
  if ("pmt"   == type) {nEntities = 32;}
  if ("pixel" == type) {nEntities = 512;}


  for (Int_t iEntity(1); iEntity <= nEntities; ++iEntity) {
    TString entityString("_");
    entityString += type;
    entityString += iEntity;

    key = "time" + entityString;
    name = key + "_" + identifier;
    if (verbose()) {std::cout << "Creating histogram " << key << std::endl;}
    histograms[key] = new TH1F(name, key, getTimeHistogramNBins(), getTimeRangeLow(), getTimeRangeHigh());

    key = "charge" + entityString;
    name = key + "_" + identifier;
    histograms[key] = new TH1F(name, key, getChargeHistogramNBins(), getChargeRangeLow(), getChargeRangeHigh());

  } //loop over PMT numbers

} //end of createIntermediateHistograms


void fillIntermediateHistograms(TChain* chain, topEvent &event, std::map<TString, TH1F*> &histograms, TString type) {
  if (type != "pmt" && type != "pixel") {
    std::cerr << "parameter \"type\" for function fillIntermediateHistograms must be either \"pmt\" or \"pixel\"." << std::endl;
  }

  Long64_t nEvents(chain->GetEntries());
  
  for (Long64_t iEvent(0); iEvent < nEvents; ++iEvent) {
    chain->GetEntry(iEvent);
    histograms["eventNumber"]->Fill(event.eventNumber);
        
    Int_t nHit(event.nHit);
    for (Int_t iHit(0); iHit < nHit; ++iHit) {


        TString fillName("time_");
	fillName += type;
        if ("pmt" == type)   {fillName += event.pmt[iHit];}
        if ("pixel" == type) {fillName += event.pixel[iHit];}	 
        histograms[fillName]->Fill(event.time[iHit]);

        fillName = "charge_";
	fillName += type;
        if ("pmt" == type)   {fillName += event.pmt[iHit];}
        if ("pixel" == type) {fillName += event.pixel[iHit];}
        histograms[fillName]->Fill(event.charge[iHit]);

    }//nHit loop
    
  } // nEvent loop

} //end of fillIntermediateHistograms


void createRatioHistograms(TString type, std::map<TString, TH1F*> &histograms) {

  if (type != "pmt" && type != "pixel") {
    std::cerr << "parameter \"type\" for function createRatioHistograms must be either \"pmt\" or \"pixel\"." << std::endl;
  }

  Int_t nEntities(0);
  if ("pmt"   == type) {nEntities = 32;}
  if ("pixel" == type) {nEntities = 512;}

  // time RMS histograms
  histograms["timeRMS_1"] = new TH1F("timeRMS_1", "timeRMS_1", nEntities, 1, nEntities + 1);
  histograms["timeRMS_2"] = new TH1F("timeRMS_2", "timeRMS_2", nEntities, 1, nEntities + 1);
  histograms["timeRMS_1"]->SetXTitle(type + " number");
  histograms["timeRMS_1"]->SetYTitle("time RMS / ns");
  histograms["timeRMS_1"]->SetOption("e");  
  histograms["timeRMS_2"]->SetXTitle(type + " number");
  histograms["timeRMS_2"]->SetYTitle("time RMS / ns");
  histograms["timeRMS_2"]->SetOption("e");

  // charge mean histograms
  histograms["chargeMean_1"] = new TH1F("chargeMean_1", "chargeMean_1", nEntities, 1, nEntities + 1);
  histograms["chargeMean_2"] = new TH1F("chargeMean_2", "chargeMean_2", nEntities, 1, nEntities + 1);
  histograms["chargeMean_1"]->SetXTitle(type + " number");
  histograms["chargeMean_1"]->SetYTitle("mean charge");
  histograms["chargeMean_1"]->SetOption("e");
  histograms["chargeMean_2"]->SetXTitle(type + " number");
  histograms["chargeMean_2"]->SetYTitle("mean charge");
  histograms["chargeMean_2"]->SetOption("e");

  //charge RMS histograms
  histograms["chargeRMS_1"] = new TH1F("chargeRMS_1", "chargeRMS_1", nEntities, 1, nEntities + 1);
  histograms["chargeRMS_2"] = new TH1F("chargeRMS_2", "chargeRMS_2", nEntities, 1, nEntities + 1);
  histograms["chargeRMS_1"]->SetXTitle(type + " number");
  histograms["chargeRMS_1"]->SetYTitle("RMS charge");
  histograms["chargeRMS_1"]->SetOption("e");  
  histograms["chargeRMS_2"]->SetXTitle(type + " number");
  histograms["chargeRMS_2"]->SetYTitle("RMS charge");
  histograms["chargeRMS_2"]->SetOption("e");

  //Timing main peak histograms
  histograms["mainPeak_1"] = new TH1F("mainPeak_1", "mainPeak_1", nEntities, 1, nEntities + 1);
  histograms["mainPeak_2"] = new TH1F("mainPeak_2", "mainPeak_2", nEntities, 1, nEntities + 1);
  histograms["mainPeak_1"]->SetXTitle(type + " number");
  histograms["mainPeak_1"]->SetYTitle("integral of main peak");
  histograms["mainPeak_1"]->SetOption("e");  
  histograms["mainPeak_2"]->SetXTitle(type + " number");
  histograms["mainPeak_2"]->SetYTitle("integral of main peak");
  histograms["mainPeak_2"]->SetOption("e");

  //Timing main peak histograms
  histograms["reflectedPeak_1"] = new TH1F("reflectedPeak_1", "reflectedPeak_1", nEntities, 1, nEntities + 1);
  histograms["reflectedPeak_2"] = new TH1F("reflectedPeak_2", "reflectedPeak_2", nEntities, 1, nEntities + 1);
  histograms["reflectedPeak_1"]->SetXTitle(type + " number");
  histograms["reflectedPeak_1"]->SetYTitle("integral of reflected peak");
  histograms["reflectedPeak_1"]->SetOption("e");  
  histograms["reflectedPeak_2"]->SetXTitle(type + " number");
  histograms["reflectedPeak_2"]->SetYTitle("integral of reflected peak");
  histograms["reflectedPeak_2"]->SetOption("e");

  //Timing main peak histograms
  histograms["peakRatio_1"] = new TH1F("peakRatio_1", "peakRatio_1", nEntities, 1, nEntities + 1);
  histograms["peakRatio_2"] = new TH1F("peakRatio_2", "peakRatio_2", nEntities, 1, nEntities + 1);
  histograms["peakRatio_1"]->SetXTitle(type + " number");
  histograms["peakRatio_1"]->SetYTitle("ratio of integrals of reflected/main peaks");
  histograms["peakRatio_1"]->SetOption("e");  
  histograms["peakRatio_2"]->SetXTitle(type + " number");
  histograms["peakRatio_2"]->SetYTitle("ratio of integrals of reflected/main peaks");
  histograms["peakRatio_2"]->SetOption("e");


} // end of createRatioHistograms


void fillRatioHistograms(TString type, std::map<TString, TH1F*> &ratioHistograms, std::map<TString, TH1F*> &histograms1, std::map<TString, TH1F*> &histograms2, Int_t nEvents1, Int_t nEvents2) {

  if (type != "pmt" && type != "pixel") {
    std::cerr << "parameter \"type\" for function fillRatioHistograms must be either \"pmt\" or \"pixel\"." << std::endl;
  }

  Int_t nEntity(-1);
  if ("pmt"   == type) {nEntity = 32;}
  if ("pixel" == type) {nEntity = 512;}

  
  for (Int_t iEntity(1); iEntity <= nEntity; ++iEntity) {
    TString histoName("charge_");
    histoName += type;
    histoName += iEntity;

    ratioHistograms["chargeMean_1"]->Fill(iEntity, histograms1[histoName]->GetMean());  
    ratioHistograms["chargeMean_2"]->Fill(iEntity, histograms2[histoName]->GetMean());  
    ratioHistograms["chargeMean_1"]->SetBinError(iEntity, histograms1[histoName]->GetRMS());   
    ratioHistograms["chargeMean_2"]->SetBinError(iEntity, histograms2[histoName]->GetRMS());

    ratioHistograms["chargeRMS_1"]->Fill(iEntity, histograms1[histoName]->GetRMS());  
    ratioHistograms["chargeRMS_2"]->Fill(iEntity, histograms2[histoName]->GetRMS());

    histoName = "time_";
    histoName += type;
    histoName += iEntity;
    
    std::pair<Int_t, Int_t> peaks1 = findTimingPeaks(histograms1[histoName]);
    std::pair<Int_t, Int_t> peaks2 = findTimingPeaks(histograms2[histoName]);

    Float_t mainPeakIntegral1      = getIntegralAroundPeak(peaks1.first,  histograms1[histoName]); 
    Float_t reflectedPeakIntegral1 = getIntegralAroundPeak(peaks1.second, histograms1[histoName]);
    Float_t mainPeakIntegral2      = getIntegralAroundPeak(peaks2.first,  histograms2[histoName]); 
    Float_t reflectedPeakIntegral2 = getIntegralAroundPeak(peaks2.second, histograms2[histoName]);

    ratioHistograms["timeRMS_1"]->Fill(iEntity, histograms1[histoName]->GetRMS());  
    ratioHistograms["timeRMS_2"]->Fill(iEntity, histograms2[histoName]->GetRMS());
//    ratioHistograms["timeRMS_1"]->Fill(iEntity, getRMSAroundPeak(peaks1.first,  histograms1[histoName]));  
//    ratioHistograms["timeRMS_2"]->Fill(iEntity, getRMSAroundPeak(peaks2.first,  histograms2[histoName]));


    ratioHistograms["mainPeak_1"]->Fill(iEntity, mainPeakIntegral1 / Float_t(nEvents1));    
    ratioHistograms["reflectedPeak_1"]->Fill(iEntity, reflectedPeakIntegral1 / Float_t(nEvents1));
    ratioHistograms["mainPeak_2"]->Fill(iEntity, mainPeakIntegral2 / Float_t(nEvents2));    
    ratioHistograms["reflectedPeak_2"]->Fill(iEntity, reflectedPeakIntegral2 / Float_t(nEvents2));
    if (mainPeakIntegral1 > 0) {ratioHistograms["peakRatio_1"]->Fill(iEntity, reflectedPeakIntegral1 / mainPeakIntegral1);}
    if (mainPeakIntegral2 > 0) {ratioHistograms["peakRatio_2"]->Fill(iEntity, reflectedPeakIntegral2 / mainPeakIntegral2);}

    if (mainPeakIntegral1>0) {ratioHistograms["mainPeak_1"]->SetBinError(iEntity, ratioHistograms["mainPeak_1"]->GetBinContent(iEntity) / TMath::Sqrt(mainPeakIntegral1));}
    if (reflectedPeakIntegral1>0) {ratioHistograms["reflectedPeak_1"]->SetBinError(iEntity, ratioHistograms["reflectedPeak_1"]->GetBinContent(iEntity) / TMath::Sqrt(reflectedPeakIntegral1));}
    if (reflectedPeakIntegral1>0) {ratioHistograms["peakRatio_1"]->SetBinError(iEntity, ratioHistograms["peakRatio_1"]->GetBinContent(iEntity) / TMath::Sqrt(reflectedPeakIntegral1));}
    if (mainPeakIntegral2>0) {ratioHistograms["mainPeak_2"]->SetBinError(iEntity, ratioHistograms["mainPeak_2"]->GetBinContent(iEntity) / TMath::Sqrt(mainPeakIntegral2));}
    if (reflectedPeakIntegral2>0) {ratioHistograms["reflectedPeak_2"]->SetBinError(iEntity, ratioHistograms["reflectedPeak_2"]->GetBinContent(iEntity) / TMath::Sqrt(reflectedPeakIntegral2));}
    if (reflectedPeakIntegral2>0) {ratioHistograms["peakRatio_2"]->SetBinError(iEntity, ratioHistograms["peakRatio_2"]->GetBinContent(iEntity) / TMath::Sqrt(reflectedPeakIntegral2));}

   
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

  ratioHistograms["mainPeak_ratio"] = (TH1F*) ratioHistograms["mainPeak_2"]->Clone("mainPeak_ratio");
  ratioHistograms["mainPeak_ratio"]->SetYTitle("ratio"); 
  ratioHistograms["mainPeak_ratio"]->Divide(ratioHistograms["mainPeak_1"]);

  ratioHistograms["reflectedPeak_ratio"] = (TH1F*) ratioHistograms["reflectedPeak_2"]->Clone("reflectedPeak_ratio");
  ratioHistograms["reflectedPeak_ratio"]->SetYTitle("ratio"); 
  ratioHistograms["reflectedPeak_ratio"]->Divide(ratioHistograms["reflectedPeak_1"]);

  ratioHistograms["peakRatio_ratio"] = (TH1F*) ratioHistograms["peakRatio_2"]->Clone("peakRatio_ratio");
  ratioHistograms["peakRatio_ratio"]->SetYTitle("double ratio"); 
  ratioHistograms["peakRatio_ratio"]->Divide(ratioHistograms["peakRatio_1"]);







} //end of fillRatiohistograms


void setRatioHistogramTitles(TString type, std::map<TString, TH1F*> &ratioHistograms, std::pair<Int_t, Int_t> &runRange1, std::pair<Int_t, Int_t> &runRange2) {
  if (type != "pmt" && type != "pixel") {
    std::cerr << "parameter \"type\" for function setRatioHistogramTitles must be either \"pmt\" or \"pixel\"." << std::endl;
  }


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

  ratioHistograms["mainPeak_1"]->SetTitle("Integral around main peak " + runTitle1);
  ratioHistograms["mainPeak_2"]->SetTitle("Integral around main peak " + runTitle2);
  ratioHistograms["mainPeak_ratio"]->SetTitle("Ratio of Integrals around main peaks " + runTitleRatio);
  ratioHistograms["reflectedPeak_1"]->SetTitle("Integral around reflected peak " + runTitle1);
  ratioHistograms["reflectedPeak_2"]->SetTitle("Integral around reflected peak " + runTitle2);
  ratioHistograms["reflectedPeak_ratio"]->SetTitle("Ratio of Integrals around reflected peaks " + runTitleRatio);
  ratioHistograms["peakRatio_1"]->SetTitle("Ratio of Integrals around reflected/main peak " + runTitle1);
  ratioHistograms["peakRatio_2"]->SetTitle("Ratio of Integrals around reflected/main peak " + runTitle2);
  ratioHistograms["peakRatio_ratio"]->SetTitle("Double Ratio of Integrals around reflected/main peaks " + runTitleRatio);

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



std::pair<Int_t, Int_t> findTimingPeaks(TH1F* histogram) {

  bool timingVerbose(false);

  Int_t nBins = getTimeHistogramNBins();
  Float_t timeStep = (getTimeRangeHigh() - getTimeRangeLow()) / nBins; 


  Float_t max5Bins(0);
  Int_t maxBinD(-1); // Direct peak

  for (Int_t iBin(1); iBin <= nBins; ++iBin) {
    Float_t sum5Bins(0);
    if (iBin > 2) {sum5Bins += histogram->GetBinContent(iBin - 2);}
    if (iBin > 1) {sum5Bins += histogram->GetBinContent(iBin - 1);}
    sum5Bins += histogram->GetBinContent(iBin - 1);
    if (iBin < nBins - 1) {sum5Bins += histogram->GetBinContent(iBin + 1);}
    if (iBin < nBins - 2) {sum5Bins += histogram->GetBinContent(iBin + 2);}

    if (sum5Bins > max5Bins) {
      max5Bins = sum5Bins;
      maxBinD = iBin;
    }
  } // loop over bins

  // find largest reflected peak (at least (default) 20 ns later than main peak).
  
  Int_t RPeakSearchStartBin(-1);
  
  RPeakSearchStartBin = maxBinD + (minReflectedDelay() / timeUnit()) / timeStep;
  
  if (timingVerbose) {std::cout << "Start looking for reflected peak in bin " << RPeakSearchStartBin << " at time " << histogram->GetBinCenter(RPeakSearchStartBin) << std::endl;}

  if (RPeakSearchStartBin < 1 || RPeakSearchStartBin > nBins - 2) {
    if (timingVerbose) {std::cerr << "Could not search for reflected peak within nominal range." << std::endl;}
  }  

  max5Bins = 0;
  Int_t maxBinR(-1);
  
  for (Int_t iBin(RPeakSearchStartBin); iBin <= nBins; ++iBin) {
    Float_t sum5Bins(0);
    if (iBin > 2) {sum5Bins += histogram->GetBinContent(iBin - 2);}
    if (iBin > 1) {sum5Bins += histogram->GetBinContent(iBin - 1);}
    sum5Bins += histogram->GetBinContent(iBin - 1);
    if (iBin < nBins - 1) {sum5Bins += histogram->GetBinContent(iBin + 1);}
    if (iBin < nBins - 2) {sum5Bins += histogram->GetBinContent(iBin + 2);}

    if (sum5Bins > max5Bins) {
      max5Bins = sum5Bins;
      maxBinR = iBin;
    } 
  } // loop over bins
  
  return std::pair<Int_t, Int_t>(maxBinD, maxBinR);
   
} //end of findTimingPeaks  

Float_t getIntegralAroundPeak(Int_t iBin, TH1F* histogram) {

  Int_t nBins = getTimeHistogramNBins();
  Float_t timeStep = (getTimeRangeHigh() - getTimeRangeLow()) / nBins;
  Float_t nBinsToSample = (timeWidthForPeaks() / timeUnit()) / timeStep; 

  Int_t minBin(iBin - nBinsToSample / 2);
  Int_t maxBin(iBin + nBinsToSample / 2);
 
  if (minBin < 1)     {minBin = 1;};
  if (maxBin > nBins) {maxBin = nBins;}
  
  Float_t integral(0);
  for (Int_t jBin(minBin); jBin <= maxBin; ++jBin) {
    integral += histogram->GetBinContent(jBin);
  }  
  
  return integral;

} //end of getIntegralAroundPeak

Float_t getRMSAroundPeak(Int_t iBin, TH1F* histogram) {

  Int_t nBins = getTimeHistogramNBins();
  Float_t timeStep = (getTimeRangeHigh() - getTimeRangeLow()) / nBins;
  Float_t nBinsToSample = (timeWidthForPeaks() / timeUnit()) / timeStep; 

  Int_t minBin(iBin - nBinsToSample / 2);
  Int_t maxBin(iBin + nBinsToSample / 2);
 
  if (minBin < 1)     {minBin = 1;};
  if (maxBin > nBins) {maxBin = nBins;}
  
  Float_t sumSq(0);
  Float_t sum(0);
  Float_t entries(0);
  
  for (Int_t jBin(minBin); jBin <= maxBin; ++jBin) {
    entries += histogram->GetBinContent(jBin);
    sum     += histogram->GetBinCenter(jBin) * histogram->GetBinContent(jBin);
    sumSq   += histogram->GetBinCenter(jBin) * histogram->GetBinCenter(jBin) * histogram->GetBinContent(jBin);
  }  
  Float_t mean(sum/entries);
  Float_t meanSumSq(sumSq/entries);

  return TMath::Sqrt(meanSumSq - mean * mean);

  
} //end of getRMSofPeak




