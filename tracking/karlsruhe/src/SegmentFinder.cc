/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/SegmentFinder.h"

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include "TMath.h"
#include "TCanvas.h"
#include "TFile.h"

using namespace std;
using namespace Belle2;


SegmentFinder::SegmentFinder()
{
}

SegmentFinder::~SegmentFinder()
{
}

void SegmentFinder::ConformalTransformation(string CDCTrackHitArray)
{

  StoreArray<CDCTrackHit> cdcTrackHitArray(CDCTrackHitArray.c_str());

  int nHits = cdcTrackHitArray.GetEntries();

  for (int i = 0; i < nHits ; i++) {
    CDCTrackHit *aTrackHit = cdcTrackHitArray[i];
    double x = aTrackHit->getWirePosition().x();
    double y = aTrackHit->getWirePosition().y();
    double cx = 2 * x / (x * x + y * y); //transformation of the coordinates from normal to conformal plane
    double cy = 2 * y / (x * x + y * y);

    aTrackHit->setConformalPosition(cx, cy);

  }

}

bool SegmentFinder::IsValueWithinRange(double value, double min, double max)
{
  if (min <= max) {
    if (value >= min && value <= max) return true;
    else return false;
  } else {
    if (value >= max && value <= min) return true;
    else return false;

  }
}





void SegmentFinder::ClassifySegments(string SegmentsCDCArray, int minHits, int maxHits, int nWires)
{
  StoreArray<CDCSegment> cdcSegmentsArray(SegmentsCDCArray.c_str());

  for (int i = 0; i < cdcSegmentsArray.GetEntries(); i++) {
    if (IsValueWithinRange(cdcSegmentsArray[i]->getNHits(), minHits, maxHits) == false || cdcSegmentsArray[i]->getWireIdDiff() > nWires)  {
      cdcSegmentsArray[i]->setIsGood(false) ; //classify segment as bad

    } else {
      cdcSegmentsArray[i]->setIsGood(true) ; //classify segment as good
    }

  }

}

void SegmentFinder::FindSegments(string CDCTrackHitArray, string SegmentsCDCArray, bool root_output)
{

  StoreArray<CDCTrackHit> cdcTrackHitArray(CDCTrackHitArray.c_str());
  StoreArray<CDCSegment> cdcSegmentsArray(SegmentsCDCArray.c_str());

  int nHits = cdcTrackHitArray.GetEntries();
  TH1F * histo[10]; //array of histograms

  double histo_start = 0; //start value for the histograms
  double histo_end = 2 * TMath::Pi() + 0.04; //end value for the histograms

  //phi in radiant, end value chosen >2pi to be able to shift values and avoid edge effects

  //initialize histograms for each superlayer, the number of bins corresponds to the number of wires per layer (in this superlayer)

  histo[1] = new TH1F(Form("SL_1"), "Superlayer 1", 160, histo_start, histo_end);
  histo[2] = new TH1F(Form("SL_2"), "Superlayer 2", 160, histo_start, histo_end);
  histo[3] = new TH1F(Form("SL_3"), "Superlayer 3", 192, histo_start, histo_end);
  histo[4] = new TH1F(Form("SL_4"), "Superlayer 4", 224, histo_start, histo_end);
  histo[5] = new TH1F(Form("SL_5"), "Superlayer 5", 256, histo_start, histo_end);
  histo[6] = new TH1F(Form("SL_6"), "Superlayer 6", 288, histo_start, histo_end);
  histo[7] = new TH1F(Form("SL_7"), "Superlayer 7", 320, histo_start, histo_end);
  histo[8] = new TH1F(Form("SL_8"), "Superlayer 8", 352, histo_start, histo_end);
  histo[9] = new TH1F(Form("SL_9"), "Superlayer 9", 388, histo_start, histo_end);

  //Loop over the superlayers and put the phi-values of the TrackHits in root histograms

  vector <double> Phi; //vector to hold the phi values of the TrackHits
  int counter = -1; //variable to count the segments

  for (int j = 1; j <= 9; j++) {//loop over all superlayers

    int nbins = histo[j]->GetNbinsX() ; //number of bins in the histogram
    double binWidth = histo[j]->GetBinWidth(5); //bin width of the histogram, taken from arbitrary bin (depends on histogram binning)
    int start = 1; //variable to mark the first zero-entry bin

    for (int i = 0; i < nHits; i++) {  //loop over all TrackHits
      Phi.push_back(cdcTrackHitArray[i]->getPhi());

      CDCTrackHit *aTrackHit = cdcTrackHitArray[i];

      if (aTrackHit->getSuperlayerId() == j)//only hits in the current superlayer are considered

      {
//check if there are entries in the first bin (and if there are, also in the following bins)
        bool goodStart = false; //is true if the the first zero entry is found
        while (goodStart == false) { //while loop as long as there are entries in the first bins
          goodStart = true;
          if (Phi.at(i) < (histo[j]->GetBinCenter(start) + binWidth / 2) && Phi.at(i) >= (histo[j]->GetBinCenter(start) - binWidth / 2)) { //if there are entries in the bin, set goodStart to false to check if there are entries in the following bins
            goodStart = false ;
          }

          if (goodStart == false) start++; //if a non-zero entry is found, increment the mark for the first zero-entry bin

        }//end while loop

      }
    }//end loop over all TrackHits

    for (int i = 0; i < nHits; i++) { //Loop over all TrackHits
      if (cdcTrackHitArray[i]->getSuperlayerId() == j)//only hits in the current superlayer are considered

      {
//check if the first zero bin was not the first one and shift all entries so that the histogram can start at zero bin
        if (start > 1) {
          Phi.at(i) = Phi.at(i) - binWidth * (start - 1);
          if (Phi.at(i) <= 0) {
            Phi.at(i) = Phi.at(i) + 2 * TMath::Pi(); //take care that all entries lie between 0 and 2pi
          }
        }

        histo[j]->Fill(Phi.at(i)); //fill the phi values in the histogram

      }

    }


//Start cluster finding

    int bin = 1; //variable to count the bins
    bool startSegment = true; //boolean to define if a new segment should be started

    //Loop over all bins in the histogram
    while (bin < nbins) {

      //Counts the bins up as long as no entries are found, as soon a an entry is found a new Segment will be created
      while (histo[j]->GetBinContent(bin) == 0 && bin < nbins) {
        bin++;
        startSegment = true;
      }

      //As soon as an entry is found all bins are assigned to one cluster (=segment)
      while (histo[j]->GetBinContent(bin) != 0 && bin < nbins) {

        //Create new Segment
        if (startSegment == true) {
          counter ++;
          new(cdcSegmentsArray->AddrAt(counter)) CDCSegment(j, counter);  //Achtung Achtung

          startSegment = false;
        }

        for (int i = 0; i < nHits; i++) {  //Loop over all TrackHits

          CDCTrackHit *aTrackHit = cdcTrackHitArray[i];

          //If a TrackHit has a correspondig superlayerId and a phi value within the bin-range, it is added to the segment
          if (aTrackHit->getSuperlayerId() == j && IsValueWithinRange(Phi.at(i), histo[j]->GetBinCenter(bin) - binWidth / 2 , histo[j]->GetBinCenter(bin)  + binWidth / 2)) {
            cdcSegmentsArray[counter]->addTrackHit(*aTrackHit) ;

          }
        }//end for-loop over TrackHits

        bin++;

      }//end while-loop over found entries

    }//end while-loop over all bins

    Phi.clear(); //clear the vector to be filled again for the next superlayer

  }//end for-loop over all superlayers

  ClassifySegments(SegmentsCDCArray.c_str(), 5, 20, 10); //classifies segments as good and bad

  if (root_output == true) {
//Creates a root file
    std::string filename = CDCTrackHitArray;
    filename = filename.append(".root");

    std::string pdfname = CDCTrackHitArray;
    pdfname = pdfname.append(".pdf");

    TFile * output = new TFile(filename.c_str(), "RECREATE");
    for (int j = 1; j <= 9; j++) {  //loop over all superlayers and collect the histograms in the root file
      histo[j]->Write();
      histo[j]->GetXaxis()->SetTitle("#phi [rad]");
    }

//Creates a pdf with all histograms
    TCanvas *canvas = new TCanvas("canvas", "#phi distribution", 100, 10, 600, 400);
    canvas->SetFillColor(0);
    canvas->SetFillStyle(4000);
    canvas->Divide(3, 3);

    for (int j = 1; j <= 9; j++) {
      canvas->cd(j);
      histo[j]->Draw();
    }
    canvas->Update();

    canvas->Print(pdfname.c_str());

    output->Close();
  }//end if root_output

}//end FindSegments



