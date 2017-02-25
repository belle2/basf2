/*************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

#include <TFile.h>
#include <TH1.h>
#include <TGraph.h>

using namespace std;

void analysisROC(const char* filenameIn="teacherHistos.root",const char* filenameOutPdf = "analysisROC.pdf", const char* filenameOutList = "analysisROC.data")
{
    cout << "Start ..." << endl;

    // load input file
    TFile *file = new TFile(filenameIn);

    // load signal and background histos
    char histoNameSignal[] = "h9202";
    char histoNameBackground[] = "h9102";

    cout << "Signal histo name: " << histoNameSignal << endl;
    cout << "Background histo name: " << histoNameBackground << endl;
    cout << "Check out 'TBrowser <filenameIn>' for histo names." << endl;

    TH1F* histoSignal = (TH1F*)file->Get(histoNameSignal);
    TH1F* histoBackground = (TH1F*)file->Get(histoNameBackground);

    // get number of bins per histo
    float numBinsSignal = histoSignal->GetEntries();
    float numBinsBackground = histoBackground->GetEntries();

    // get histo samples number
    float numSamplesSignal = 0;
    float numSamplesBackground = 0;
    int globalBin;

    for(int k=1; k<(int)numBinsSignal; k++)
    {
        globalBin = histoSignal->GetBin(k);
        numSamplesSignal += histoSignal->GetBinContent(globalBin);
    }

    for(int k=1; k<(int)numBinsBackground; k++)
    {
        globalBin = histoBackground->GetBin(k);
        numSamplesBackground += histoBackground->GetBinContent(globalBin);
    }

    // output
    cout << "Bins histos [signal background]: " << numBinsSignal << " " << numBinsBackground << endl;
    cout << "Samples histos [signal background]: " << numSamplesSignal << " " << numSamplesBackground << endl;

    // error handling: signal and background need same number of bins
    if(numBinsSignal!=numBinsBackground)
    {
        cout << "ERROR: Signal and background do not have same number of bins." << endl;
        return;
    }

    // get ROC graph and write data points to file and arrays
    ofstream fileList;
    fileList.open(filenameOutList);
    fileList << "bin thresholdNetOutput clusterRescue backgroundReduction" << endl;

    float sumSamplesSignal = numSamplesSignal;
    float sumSamplesBackground = 0;
    const int maxNumBins = 1000;
    Double_t percentageSignal[maxNumBins-2];
    Double_t percentageBackground[maxNumBins-2];

    cout << "Number of histo bins has to be less than " << maxNumBins << "." << endl;
    cout << "Histogram range (network output) assumed from -1 to 1." << endl;

    for(int k=1; k<numBinsSignal-1; k++)
    {
        float threshold = -1.0+2.0*float(k)/numBinsSignal;

        globalBin = histoSignal->GetBin(k);
        sumSamplesSignal -= histoSignal->GetBinContent(globalBin);
        globalBin = histoBackground->GetBin(k);
        sumSamplesBackground += histoBackground->GetBinContent(globalBin);

        percentageSignal[k-1] = sumSamplesSignal/numSamplesSignal;
        percentageBackground[k-1] = sumSamplesBackground/numSamplesBackground;

        // save data to file
        fileList << k << " " << threshold << " " << percentageSignal[k-1] << " " << percentageBackground[k-1] << endl;
    }

    // write ROC graph to TGraph
    TCanvas *c1 = new TCanvas("c1","analysisROC");
    //c1->SetFillColor(42);

    TGraph *gr = new TGraph(numBinsSignal-2,percentageSignal,percentageBackground);
    gr->SetLineColor(2);
    gr->SetLineWidth(2);
    gr->SetMarkerColor(4);
    gr->SetMarkerStyle(21);
    gr->SetMarkerSize(0.5);
    gr->SetTitle("ROC graph NeuroBayes neural network");
    gr->GetXaxis()->SetTitle("Cluster rescue");
    gr->GetYaxis()->SetTitle("Background reduction");
    gr->Draw("ACP");

    c1->Update();
    //c1->GetFrame()->SetFillColor(21);
    //c1->GetFrame()->SetBorderSize(12);
    //c1->Modified();

    c1->Print(filenameOutPdf);

    cout << "... finished." << endl;
    fileList.close();
    return;
}
