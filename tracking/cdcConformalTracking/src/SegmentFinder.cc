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

#include "TGraph.h"
#include "TAxis.h"
#include "TF1.h"

using namespace std;
using namespace Belle2;


SegmentFinder::SegmentFinder()
{
}

SegmentFinder::~SegmentFinder()
{
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



void SegmentFinder::ClassifySegments(string CDCSegments, int minHits, int maxHits, int nWires)
{
  StoreArray<CDCSegment> cdcSegments(CDCSegments.c_str());

  for (int i = 0; i < cdcSegments.GetEntries(); i++) {
    if (IsValueWithinRange(cdcSegments[i]->getNHits(), minHits, maxHits) == false || cdcSegments[i]->getWireIdDiff() > nWires)  {
      cdcSegments[i]->setIsGood(false) ; //classify segment as bad

    } else {
      cdcSegments[i]->setIsGood(true) ; //classify segment as good
    }

  }

}

void SegmentFinder::FindSegments(string CDCTrackHits, string CDCSegments, bool root_output)
{

  StoreArray<CDCTrackHit> cdcTrackHits(CDCTrackHits.c_str());
  StoreArray<CDCSegment> cdcSegments(CDCSegments.c_str());

  int nHits = cdcTrackHits.GetEntries();
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
      Phi.push_back(cdcTrackHits[i]->getPhi());

      CDCTrackHit *aTrackHit = cdcTrackHits[i];

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
      if (cdcTrackHits[i]->getSuperlayerId() == j)//only hits in the current superlayer are considered

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
          new(cdcSegments->AddrAt(counter)) CDCSegment(j, counter);  //Achtung Achtung

          startSegment = false;
        }

        for (int i = 0; i < nHits; i++) {  //Loop over all TrackHits

          CDCTrackHit *aTrackHit = cdcTrackHits[i];

          //If a TrackHit has a correspondig superlayerId and a phi value within the bin-range, it is added to the segment
          if (aTrackHit->getSuperlayerId() == j && IsValueWithinRange(Phi.at(i), histo[j]->GetBinCenter(bin) - binWidth / 2 , histo[j]->GetBinCenter(bin)  + binWidth / 2)) {
            cdcSegments[counter]->addTrackHit(*aTrackHit) ;

          }
        }//end for-loop over TrackHits

        bin++;

      }//end while-loop over found entries

    }//end while-loop over all bins

    Phi.clear(); //clear the vector to be filled again for the next superlayer

  }//end for-loop over all superlayers

  ClassifySegments(CDCSegments.c_str(), 5, 20, 10); //classifies segments as good and bad

  //-------------------------------------------------
  //FitSegments(SegmentsCDCArray.c_str());
  //-------------------------------------------------

  if (root_output == true) {
//Creates a root file
    std::string filename = CDCTrackHits;
    filename = filename.append(".root");

    std::string pdfname = CDCTrackHits;
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


//Segment Fitting: not needed for the moment, will maybe useful to reduce background, still under developement!
//--------------------------------------------------------------------------
/*vector<int> SegmentFinder::FindOvercrowdedLayers(CDCSegment segment, int nWires) {

  //Search for Segments with too many hits (>nWires) in the same layer
  vector<int> layers;
  vector<int> overcrowdedLayers;
  //collect all layers in this segment

  for (int i = 0; i < segment.getNHits(); i++) {
    layers.push_back(segment.getTrackHits().at(i).getLayerId());
  }

  //loop through all layers and count the hits in each layer
  for (int j = 0; j < layers.size(); j++) {
    bool already = false;
    int nHitWires = count(layers.begin(), layers.end(), layers.at(j));
    for (int k = 0; k < overcrowdedLayers.size(); k++) {
      if (layers.at(j) == overcrowdedLayers.at(k))
        already = true;
    }
    //if there are too many hits and if this layer wasn't counted already, put the layer id in the return vector
    if (nHitWires > nWires && already == false) {
      overcrowdedLayers.push_back(layers.at(j));
    }
  }

  return overcrowdedLayers;

}*/

/*void SegmentFinder::FitSegments(string CDCSegments) {
 //B2INFO("------  FitSegments")
   StoreArray<CDCSegment> cdcSegments(CDCSegments.c_str());
 int nSegments = cdcSegments->GetEntries();
 TGraph * graph;
 double x[1000];
 double y[1000];
 int nHits;

 //loop over all segments
 for (int i = 0; i < nSegments; i++) {
   //B2INFO("********** Segment Nr "<<i<<"  SL "<<cdcSegments[i]->getSuperlayerId());
   nHits = cdcSegments[i]->getNHits();
   vector<int> overcrowdedLayers;

   //perform first fit
   //B2INFO("Nr of Hits "<<nHits);
   for (int j = 0; j < nHits; j++) {
     x[j] = cdcSegments[i]->getTrackHits().at(j).getConformalX();
     y[j] = cdcSegments[i]->getTrackHits().at(j).getConformalY();
     //B2INFO("LayerId "<<cdcSegments[i]->getTrackHits().at(j).getLayerId()<<"  WireId "<<cdcSegments[i]->getTrackHits().at(j).getWireId());
     //B2INFO("X: "<<x[j]<<"  Y: "<<y[j]);
   }
   graph = new TGraph(nHits, x, y);

   double min = graph->GetXaxis()->GetXmin();
   double max = graph->GetXaxis()->GetXmax();
   //TH1F *fit = new TH1F("fit","pol1", min, max);
   graph->Fit("pol1", "Q", "", min, max);
   TF1 *fit = graph->GetFunction("pol1");
   //TH1F *fithisto = new TH1F("fit","pol1", nHits, min, max);
   TH1 *fithisto = fit->GetHistogram();
   double Chi = fit->GetChisquare();
   B2INFO("Chisquare "<<Chi<<"  "<<fit->GetParameter(0)<<"  "<<fit->GetParameter(1));
   cdcSegments[i]->setChiSquare(Chi);

   if (Chi > 0.000003/cdcSegments[i]->getSuperlayerId()){
     B2INFO("Fit needed");

   //set deviation cuts
   double deviation = 0;
   double maxDeviation = 0;
       // Some test values!!!!
   double deviationCut = 0.6;
   double deviationCut_1 = 0.5;
   double deviationCut_2 = 0.4;
   double deviationCut_3 = 0.3;
   double deviationCut_4 = 0.3;
   double deviationCut_5 = 0.2;
   double deviationCut_6 = 0.2;
   double deviationCut_7 = 0.1;
   double deviationCut_8 = 0.1;
   double deviationCut_9 = 0.1;

   bool badHits = true; //if true, continue to search for bad hits
   int badHitId = 0; //ID of the bad Hit to be removed
   bool overcrowdedLayer = false; //true if the layer hat too many hits

   bool withinDeviationCut = true; //true if a hit lies within the deviation cut

   //loop over all hits as long as there are still some bad hits in the segment
   while (badHits == true) {
     badHits = false;
     deviation = 0;
     withinDeviationCut == true;
     badHitId = 0;
     //  B2INFO("Hits in the Segment: "<<nHits);

     //Find overcrowded layers for this segment
     if (cdcSegments[i]->getSuperlayerId() == 1) {
       overcrowdedLayers = FindOvercrowdedLayers(*cdcSegments[i], 3);
     } else
       overcrowdedLayers = FindOvercrowdedLayers(*cdcSegments[i], 2);

     //loop over all hits
     for (int j = 0; j < nHits; j++) {
       overcrowdedLayer = false;
       withinDeviationCut = true;
       maxDeviation = 0;
       for (int k = 0; k < overcrowdedLayers.size(); k++) {
         if (cdcSegments[i]->getTrackHits().at(j).getLayerId()
             == overcrowdedLayers.at(k)) {
           overcrowdedLayer = true;
           //B2INFO("Overcrowded Layer!");
         }
       }
       //calculate deviations for hits in overcrowded layers
       if (overcrowdedLayer == true) {
         B2INFO("x: "<<x[j]<<"  y: "<<y[j]);
         deviation = fabs((y[j] - (fit->GetParameter(1) * x[j]
             + fit->GetParameter(0))) / (fit->GetParameter(1)
             * x[j]+ fit->GetParameter(0)));
                       B2INFO("Deviation: "<<deviation);
         if (deviation > maxDeviation) {
           maxDeviation = deviation;
           badHitId = j;
         }

         if (cdcSegments[i]->getSuperlayerId() == 1 && maxDeviation
             > deviationCut_1)
           withinDeviationCut = false;
         else if (cdcSegments[i]->getSuperlayerId() == 2
             && maxDeviation > deviationCut_2)
           withinDeviationCut = false;
         else if (cdcSegments[i]->getSuperlayerId() == 3
             && maxDeviation > deviationCut_3)
           withinDeviationCut = false;
         else if (cdcSegments[i]->getSuperlayerId() == 4
             && maxDeviation > deviationCut_4)
           withinDeviationCut = false;
         else if (cdcSegments[i]->getSuperlayerId() == 5
             && maxDeviation > deviationCut_5)
           withinDeviationCut = false;
         else if (cdcSegments[i]->getSuperlayerId() == 6
             && maxDeviation > deviationCut_6)
           withinDeviationCut = false;
         else if (cdcSegments[i]->getSuperlayerId() == 7
             && maxDeviation > deviationCut_7)
           withinDeviationCut = false;
         else if (cdcSegments[i]->getSuperlayerId() == 8
             && maxDeviation > deviationCut_8)
           withinDeviationCut = false;
         else if (cdcSegments[i]->getSuperlayerId() == 9
             && maxDeviation > deviationCut_9)
           withinDeviationCut = false;


         //Remove hits with too bad deviations
         if (withinDeviationCut == false && cdcSegments[i]->getNHits() > 3) {
           B2INFO("Remove Hit "<<maxDeviation);
           cdcSegments[i]->removeTrackHit(badHitId);

           //Update overcrowded layers for this segment
             if (cdcSegments[i]->getSuperlayerId() == 1) {
               overcrowdedLayers = FindOvercrowdedLayers(*cdcSegments[i], 3);
             } else
               overcrowdedLayers = FindOvercrowdedLayers(*cdcSegments[i], 2);


           badHits = true;
           nHits = cdcSegments[i]->getNHits();
           //Refit the segment again, without the bad hit
           B2INFO("Refit");

           for (int j = 0; j < nHits; j++) {
             x[j]
                 = cdcSegments[i]->getTrackHits().at(j).getConformalX();
             y[j]
                 = cdcSegments[i]->getTrackHits().at(j).getConformalY();

           }
           graph = new TGraph(nHits, x, y);

           min = graph->GetXaxis()->GetXmin();
           max = graph->GetXaxis()->GetXmax();

           graph->Fit("pol1", "Q", "", min, max);
           fit = graph->GetFunction("pol1");
           Chi = fit->GetChisquare();
           cdcSegments[i]->setChiSquare(Chi);

         }//end if within deviation cut

       }//end if overcrowded =true
     }//end loop over hits
   }// end while

   B2INFO("Refit without overcrowded layers");
   badHits = true;
   double deviationStrictCut_1 = 17;
   double deviationStrictCut_9 = 1.0 ;
   //bool withinStrictCut = true;


   while (badHits == true) {
     badHits = false;
     deviation = 0;
     maxDeviation = 0;
     badHitId = 0;
     //  B2INFO("Hits in the Segment: "<<nHits);

     for (int j = 0; j < nHits; j++) {
               //withinStrictCut = true ;


       deviation = fabs((y[j] - (fit->GetParameter(1) * x[j]
           + fit->GetParameter(0)))
           / (fit->GetParameter(1) * x[j] + fit->GetParameter(0)));
       B2INFO("x: "<<x[j]<<"  y: "<<y[j]);
       B2INFO("Deviation: "<<deviation);
       if (deviation > maxDeviation) {
         maxDeviation = deviation;
         badHitId = j;
       }
     }


     if ((maxDeviation > deviationStrictCut_1 &&  cdcSegments[i]->getSuperlayerId() < 8)||(maxDeviation > deviationStrictCut_9 &&  cdcSegments[i]->getSuperlayerId() > 7) && cdcSegments[i]->getNHits() > 3) {
       B2INFO("Remove Hit after strict cut "<<maxDeviation);
       cdcSegments[i]->removeTrackHit(badHitId);
       badHits = true;
       nHits = cdcSegments[i]->getNHits();
       B2INFO("Refit");

       for (int j = 0; j < nHits; j++) {
         x[j]
             = cdcSegments[i]->getTrackHits().at(j).getConformalX();
         y[j]
             = cdcSegments[i]->getTrackHits().at(j).getConformalY();

       }
       graph = new TGraph(nHits, x, y);

       min = graph->GetXaxis()->GetXmin();
       max = graph->GetXaxis()->GetXmax();

       graph->Fit("pol1", "Q", "", min, max);
       fit = graph->GetFunction("pol1");
       Chi = fit->GetChisquare();
       cdcSegments[i]->setChiSquare(Chi);

     }

   }//end while strict cut

   }
 }//end loop over all segments



}*/
//--------------------------------------------------------------------------






