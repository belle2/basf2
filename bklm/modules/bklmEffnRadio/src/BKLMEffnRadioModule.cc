/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anselm Vossen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



#include <bklm/modules/bklmEffnRadio/BKLMEffnRadioModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <math.h>

#include <bklm/geometry/GeometryPar.h>
#include <bklm/geometry/Module.h>
#include <bklm/dataobjects/BKLMDigit.h>
#include <bklm/dataobjects/BKLMHit1d.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <bklm/dataobjects/BKLMStatus.h>

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Matrix/Matrix.h"

#include <bklm/geometry/GeometryPar.h>
#include <bklm/geometry/Module.h>

#include "TMath.h"
#include "TCanvas.h"
#include "TBox.h"

#include "TColor.h"
#include "TStyle.h"
#include "TLatex.h"

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Units/PhysicalConstants.h"
#include "CLHEP/Units/SystemOfUnits.h"


using namespace std;
using namespace CLHEP;
using namespace Belle2;
using namespace Belle2::bklm;

REG_MODULE(BKLMEffnRadio)


void BKLMEffnRadioModule::set_plot_style()
{
  const Int_t NRGBs = 5;
  const Int_t NCont = 255;

  Double_t stops[NRGBs] = {0.00, 0.34, 0.61, 0.84, 1.00};
  Double_t red[NRGBs] = {0.00, 0.00, 0.87, 1.00, 0.51};
  Double_t green[NRGBs] = {0.00, 0.81, 1.00, 0.20, 0.00};
  Double_t blue[NRGBs] = {0.51, 1.00, 0.12, 0.00, 0.00};
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  gStyle->SetNumberContours(NCont);

}

BKLMEffnRadioModule::BKLMEffnRadioModule() : Module(), m_minNumPointsOnTrack(7), m_maxEffDistance(10), m_eventCounter(0),
  m_cModule(nullptr), m_cModuleEff(nullptr), m_cModuleEff2D(nullptr), m_hTrackPhi(nullptr), m_hTrackTheta(nullptr),
  m_hHitsPerLayer(nullptr), m_hClusterSize(nullptr), m_hTracksPerEvent(nullptr), m_hHitsPerEvent1D(nullptr),
  m_hHitsPerEvent2D(nullptr), m_hHitsPerEventPerLayer1D(nullptr), m_hOccupancy1D(nullptr), m_eff2DFound(nullptr),
  m_eff2DExpected(nullptr), m_strips(nullptr), m_stripsEff(nullptr), m_file(nullptr), m_stripHits(nullptr), m_stripHitsEff(nullptr),
  m_stripNonHitsEff(nullptr), m_GeoPar(NULL)
{
  setDescription("Get efficiency and generate radio plots for bklm");
  addParam("filename", m_filename, "Output root filename", string("eff_output.root"));
}

BKLMEffnRadioModule::~BKLMEffnRadioModule()
{

}

void BKLMEffnRadioModule::initialize()
{
  m_eventCounter = 0;
  set_plot_style();
  hits1D.isRequired();
  hits2D.isRequired();

  m_file = new TFile(m_filename.c_str(), "recreate");

  //one canvas per module
  m_cModule = new TCanvas** [8];
  m_cModuleEff = new TCanvas** [8];
  m_cModuleEff2D = new TCanvas** [8];

  m_eff2DFound = new TH2D** *[8];
  m_eff2DExpected = new TH2D** *[8];

  m_strips = new TBox**** *[8];
  m_stripsEff = new TBox**** *[8];

  m_stripHits = new int**** [8];
  m_stripHitsEff = new int**** [8];
  m_stripNonHitsEff = new int**** [8];

  m_hOccupancy1D = new TH1D**** [8];


  if (!m_strips || !m_stripsEff)
  { B2DEBUG(1, "no strip"); return;}
  float stripPix = 1000 / 48;
  float stripPixScinti = 1000 / 54;

  //all modules

  for (int i = 0; i < 8; i++) {
    m_strips[i] = new TBox**** [15];
    m_stripsEff[i] = new TBox**** [15];
    if (!m_strips[i] || !m_stripsEff[i])
    {B2DEBUG(1, "no strips! i = " << i); continue;}
    m_stripHits[i] = new int** *[15];
    m_stripHitsEff[i] = new int** *[15];
    m_stripNonHitsEff[i] = new int** *[15];
    m_cModule[i] = new TCanvas*[16];
    m_cModuleEff[i] = new TCanvas*[16];
    m_cModuleEff2D[i] = new TCanvas*[16];
    m_eff2DFound[i] = new TH2D** [16];
    m_eff2DExpected[i] = new TH2D** [16];
    m_hOccupancy1D[i] = new TH1D** *[16];
    char buffer[100];

    for (int iLay = 0; iLay < 15; iLay++) {

      sprintf(buffer, "Sector%d_layer%d", i + 1, iLay + 1);
      m_cModule[i][iLay] = new TCanvas(buffer, buffer, 0, 0, 1000, 800);
      sprintf(buffer, "EffSector%d_layer%d", i + 1, iLay + 1);
      m_cModuleEff[i][iLay] = new TCanvas(buffer, buffer, 0, 0, 1000, 800);
      sprintf(buffer, "Eff2DSector%d_layer%d", i + 1, iLay + 1);
      m_cModuleEff2D[i][iLay] = new TCanvas(buffer, buffer, 0, 0, 1000, 800);
      m_hOccupancy1D[i][iLay] = new TH1D** [2];
      m_strips[i][iLay] = new TBox** *[2];
      m_stripsEff[i][iLay] = new TBox** *[2];
      m_eff2DFound[i][iLay] = new TH2D*[2];
      m_eff2DExpected[i][iLay] = new TH2D*[2];

      if (!m_strips[i][iLay] || !m_stripsEff[i][iLay])
      {B2DEBUG(1, "no strips! iLay = " << iLay); continue;}
      m_stripHits[i][iLay] = new int** [2];
      m_stripHitsEff[i][iLay] = new int** [2];
      m_stripNonHitsEff[i][iLay] = new int** [2];
      //left right, phi and z strips
      m_cModule[i][iLay]->Divide(2, 2);
      m_cModuleEff[i][iLay]->Divide(2, 2);
      //each module canvas gets one pad for phi/theta strips, forward backward
      //forward/backward


      //don't divide the 2D effs in z/phi because we look at D effs. We still want to divide in fwd/backwd
      m_cModuleEff2D[i][iLay]->Divide(2);

      for (int j = 0; j < 2; j++) {

        sprintf(buffer, "EffFound_Sector%d_layer%d_fwd%d", i + 1, iLay + 1, j);
        m_eff2DFound[i][iLay][j] = new TH2D(buffer, buffer, 50, -200.0, 200.0, 50, 0.0, 300.0);
        sprintf(buffer, "EffExpected_Sector%d_layer%d_fwd%d", i + 1, iLay + 1, j);
        m_eff2DExpected[i][iLay][j] = new TH2D(buffer, buffer, 50, -200.0, 200, 50, 0.0, 300.0);

        m_strips[i][iLay][j] = new TBox** [2];
        m_stripsEff[i][iLay][j] = new TBox** [2];
        if (!m_strips[i][iLay][j] || !m_stripsEff[i][iLay][j])
        {B2DEBUG(1, "no strips! j = " << j); continue;}
        m_stripHits[i][iLay][j] = new int* [2];
        m_stripHitsEff[i][iLay][j] = new int* [2];
        m_stripNonHitsEff[i][iLay][j] = new int* [2];
        m_hOccupancy1D[i][iLay][j] = new TH1D*[2];
        //theta/phi, use the size of the RPC modules as the range
        //z for each module is about 220cm (440 fwd+bkwd), y is 167-275 cm...
        //but this doesn't matter, since we'll be using 2D histos anyways
        //        m_cModuleEff2D[i][iLay]->cd(j+1)->Range(0,0,300,300);
        for (int k = 0; k < 2; k++) {
          (m_cModule[i][iLay]->cd(j * 2 + k + 1))->Range(0, 0, 1000, 1000);
          (m_cModuleEff[i][iLay]->cd(j * 2 + k + 1))->Range(0, 0, 1000, 1000);
          m_strips[i][iLay][j][k] = new TBox*[54];
          m_stripsEff[i][iLay][j][k] = new TBox*[54];
          if (!m_strips[i][iLay][j][k] || !m_stripsEff[i][iLay][j][k])
          {B2DEBUG(1, "no strips! k = " << k); continue;}
          m_stripHits[i][iLay][j][k] = new int[54];
          m_stripHitsEff[i][iLay][j][k] = new int[54];
          m_stripNonHitsEff[i][iLay][j][k] = new int[54];
          char fwd[10];
          char phi[10];
          if (j == 1)
            sprintf(fwd, "Forward");
          else
            sprintf(fwd, "Backward");
          if (k == 1)
            sprintf(phi, "Phi");
          else
            sprintf(phi, "Z");

          sprintf(buffer, "Module%d_Layer%d_%s_%s", i + 1, iLay + 1, fwd, phi);
          //48 channels
          if (iLay >= 2)
            m_hOccupancy1D[i][iLay][j][k] = new TH1D(buffer, buffer, 48, 0, 48);
          else//54 channels
            m_hOccupancy1D[i][iLay][j][k] = new TH1D(buffer, buffer, 54, 0, 54);
          m_hOccupancy1D[i][iLay][j][k]->GetXaxis()->SetTitle("Channel #");
          m_hOccupancy1D[i][iLay][j][k]->SetFillColor(kYellow);
          for (int l = 0; l < 54; l++) {
            if (iLay >= 2) {
              m_strips[i][iLay][j][k][l] = new TBox(l * stripPix + 4, 100, (l + 1)*stripPix - 4, 900);
              m_stripsEff[i][iLay][j][k][l] = new TBox(l * stripPix + 4, 100, (l + 1)*stripPix - 4, 900);
            }
            //scintillator layer
            else {
              m_strips[i][iLay][j][k][l] = new TBox(l * stripPixScinti + 4, 100, (l + 1)*stripPixScinti - 4, 900);
              m_stripsEff[i][iLay][j][k][l] = new TBox(l * stripPixScinti + 4, 100, (l + 1)*stripPixScinti - 4, 900);

            }
            m_stripHits[i][iLay][j][k][l] = 0;
            m_stripHitsEff[i][iLay][j][k][l] = 0;
            m_stripNonHitsEff[i][iLay][j][k][l] = 0;
          }

        }
      }
    }
  }

  //initialize other histograms...
  m_hHitsPerLayer = new TH1D("HitsPerLayer", "HitsPerLayer", 15, 0, 15);
  m_hHitsPerLayer->SetFillColor(kYellow);
  m_hHitsPerLayer->GetXaxis()->SetTitle("Hits Per Layer");
  m_hClusterSize = new TH1D("ClusterSize", "ClusterSize", 10, 0, 10);
  m_hClusterSize->SetFillColor(kYellow);
  m_hClusterSize->GetXaxis()->SetTitle("Cluster Size");

  m_hTrackPhi = new TH1D("TrackPhi", "TrackPhi", 100, 0, 2 * TMath::Pi());
  m_hTrackPhi->GetXaxis()->SetTitle("Track #phi");
  m_hTrackPhi->SetFillColor(kYellow);

  m_hTrackTheta = new TH1D("TrackTheta", "TrackTheta", 100, 0, TMath::Pi());
  m_hTrackTheta->GetXaxis()->SetTitle("Track #theta");
  m_hTrackTheta->SetFillColor(kYellow);


  m_hTracksPerEvent = new TH1D("TracksPerEvent", "TracksPerEvent", 10, 0, 10);
  m_hTracksPerEvent->SetFillColor(kYellow);
  m_hTracksPerEvent->GetXaxis()->SetTitle("Tracks Per Event");

  m_hHitsPerEvent1D = new TH1D("hitsPerEvent1D", "hitsPerEvent1D", 50, 0, 50);
  m_hHitsPerEvent1D->SetFillColor(kYellow);
  m_hHitsPerEvent1D->GetXaxis()->SetTitle("1D hits per event");
  m_hHitsPerEvent2D = new TH1D("hitsPerEvent2D", "hitsPerEvent2D", 50, 0, 50);
  m_hHitsPerEvent2D->SetFillColor(kYellow);
  m_hHitsPerEvent2D->GetXaxis()->SetTitle("2D hits per event");
  m_hHitsPerEventPerLayer1D = new TH2D("hitsPerLayer1D", "hitsPerLayer1D", 50, 0, 50, 15, 0, 15);
  m_hHitsPerEventPerLayer1D->GetXaxis()->SetTitle("#1D hits");
  m_hHitsPerEventPerLayer1D->GetYaxis()->SetTitle("layer");


}

void BKLMEffnRadioModule::beginRun()
{

}

void BKLMEffnRadioModule::event()
{
  m_eventCounter++;
  if (!(m_eventCounter % 1000))
    B2DEBUG(1, "looking at event nr " << m_eventCounter);

  //   cout <<" we have " << hits1D.getEntries() << " 1D hits " << endl;

  m_hHitsPerEvent1D->Fill(hits1D.getEntries());
  int hitsPerLayer[16];
  memset(hitsPerLayer, 0, 16 * sizeof(int));
  for (int h = 0;  h < hits1D.getEntries(); h++) {
    int sector = hits1D[h]->getSector() - 1;
    if ((sector < 0) || (sector >= 8)) {
      B2DEBUG(1, "wrong sector number. sector = " << sector);
      continue;
    }
    int layer = hits1D[h]->getLayer() - 1;
    //      cout <<"layer is : "<< layer <<endl;
    if ((layer < 0) || (layer >= 15)) {
      B2DEBUG(1, "wrong layer number. layer = " << layer);
      continue;
    }
    hitsPerLayer[layer]++;
    int fwd = 0;
    int isPhi = 0;

    int channelMin = hits1D[h]->getStripMin() - 1;
    int channelMax = hits1D[h]->getStripMax() - 1;

    m_hClusterSize->Fill(channelMax - channelMin + 1);
    m_hHitsPerLayer->Fill(layer);


    //scintillator layers have 54 channels, not 48 like the rpcs
    if (channelMin < 0 || channelMax < 0 || channelMin > channelMax || channelMax >= 54 || channelMin >= 54) {
      B2DEBUG(1, "wrong channel, min = " << channelMin << " max = " << channelMax << " layer = " << layer);
      continue;
    }

    if (hits1D[h]->isForward()) {
      fwd = 1;
    }
    if (hits1D[h]->isPhiReadout()) {
      isPhi = 1;
    }
    for (int c = channelMin; c <= channelMax; c++) {
      //      cout <<"sector: " << sector <<" layer: "<< layer<<" fwd: " << fwd <<" isPhi: " << isPhi <<" channel: " << c <<endl;
      m_stripHits[sector][layer][fwd][isPhi][c]++;
      m_hOccupancy1D[sector][layer][fwd][isPhi]->Fill(c);
    }

  }
  for (int i = 0; i < 16; i++) {
    m_hHitsPerEventPerLayer1D->Fill(hitsPerLayer[i], i);
  }

  getEffs();

}

void BKLMEffnRadioModule::endRun()
{

}

void BKLMEffnRadioModule::terminate()
{
  //find max hits for normalization...
  int maxStripHits = 0;
  for (int i = 0; i < 8; i++) {
    for (int iLay = 0; iLay < 15; iLay++) {
      for (int j = 0; j < 2; j++) {
        for (int k = 0; k < 2; k++) {
          for (int l = 0; l < 54; l++) {
            if (m_stripHits[i][iLay][j][k][l] > maxStripHits) {
              maxStripHits = m_stripHits[i][iLay][j][k][l];
            }
          }
        }
      }
    }
  }

  TCanvas cLegend("colorLegend", "colorLegend", 0, 0, 1000, 800);
  cLegend.Range(0, 0, 1000, 1000);
  TLatex latex;
  latex.SetTextSize(latex.GetTextSize() / 2.0);
  for (int i = 0; i < 48; i++) {
    float lInd = i / (float)48;
    float colorIndex = lInd * 48 + 51;
    int roundInd = fabs(lInd * maxStripHits + 0.5);
    B2DEBUG(1, "lInd is " << lInd << " color index is: " << colorIndex << " max strip: " << maxStripHits << " round ind: " << roundInd);
    char buffer[200];
    sprintf(buffer, "%d", roundInd);

    latex.SetTextColor(colorIndex);
    latex.DrawLatex(100, 18 * i, buffer);
  }
  cLegend.Write();

  //    cout <<"max Strip Hits: " << maxStripHits<<endl;
  for (int i = 0; i < 8; i++) {
    for (int iLay = 0; iLay < 15; iLay++) {
      for (int j = 0; j < 2; j++) {

        m_cModuleEff2D[i][iLay]->cd(j + 1);

        for (int nx = 0; nx < m_eff2DFound[i][iLay][j]->GetNbinsX(); nx++) {
          for (int ny = 0; ny < m_eff2DFound[i][iLay][j]->GetNbinsY(); ny++) {
            if (m_eff2DExpected[i][iLay][j]->GetBinContent(nx, ny) > 0) {
              float denom = m_eff2DExpected[i][iLay][j]->GetBinContent(nx, ny);
              m_eff2DFound[i][iLay][j]->SetBinContent(nx, ny, m_eff2DFound[i][iLay][j]->GetBinContent(nx, ny) / denom);
            }
          }
        }
        m_eff2DFound[i][iLay][j]->Draw("colz");


        for (int k = 0; k < 2; k++) {
          m_cModule[i][iLay]->cd(j * 2 + k + 1);
          m_hOccupancy1D[i][iLay][j][k]->Write();
          char buffer[100];
          char fwd[10];
          char phi[10];
          if (j == 1)
            sprintf(fwd, "Forward");
          else
            sprintf(fwd, "Backward");
          if (k == 1)
            sprintf(phi, "Phi");
          else
            sprintf(phi, "Z");

          sprintf(buffer, "Sector %d, layer %d, %s, %s", i + 1, iLay + 1, fwd, phi);

          for (int l = 0; l < 54; l++) {
            //don't draw channels > 48 for RPC layers...
            if (iLay >= 2 && l >= 48)
              continue;
            //colors 51-99 seem to be fine...
            float colorIndex = m_stripHits[i][iLay][j][k][l] / (float)maxStripHits;

            colorIndex = colorIndex * 48 + 51;

            m_strips[i][iLay][j][k][l]->SetFillColor(ceil(colorIndex));
            //      strips[i][iLay][j][k][l]->SetFillColor(50+l+50*k);
            m_strips[i][iLay][j][k][l]->Draw();

          }
          //draw over strips
          TLatex latex2(10, 10, buffer);
          latex2.DrawLatex(100, 10, buffer);

        }
      }



      m_cModuleEff2D[i][iLay]->Draw();
      m_cModuleEff2D[i][iLay]->Write();

      if (i == 0 && iLay == 0) {
        m_cModuleEff2D[i][iLay]->SaveAs("effs.pdf(");
      } else {
        if (i == 7 && iLay == 14) {
          m_cModuleEff2D[i][iLay]->SaveAs("effs.pdf)");
        } else {
          m_cModuleEff2D[i][iLay]->SaveAs("effs.pdf");
        }
      }





      m_cModule[i][iLay]->Draw();
      m_cModule[i][iLay]->Write();

      if (i == 0 && iLay == 0) {
        cLegend.SaveAs("radio.pdf(");
        m_cModule[i][iLay]->SaveAs("radio.pdf");

      } else {
        if (i == 7 && iLay == 14) {
          m_cModule[i][iLay]->SaveAs("radio.pdf)");
        } else {
          m_cModule[i][iLay]->SaveAs("radio.pdf");
        }
      }
    }
  }

  m_hHitsPerLayer->Write();
  m_hClusterSize->Write();
  m_hTracksPerEvent->Write();
  m_hHitsPerEvent1D->Write();
  m_hHitsPerEvent2D->Write();
  m_hHitsPerEventPerLayer1D->Write();
  m_hTrackPhi->Write();
  m_hTrackTheta->Write();


  m_file->Write();
  m_file->Close();
}


//let's look into only one module where all planes are coplanar, so we don't have to do any
//math to get the intersections

void BKLMEffnRadioModule::getEffs()
{
  //  cout <<"get Effs " <<endl;


  m_GeoPar = GeometryPar::instance();
  //  cout <<"we have " <<hits2D.getEntries() <<" 2d hits " <<endl;
  m_hHitsPerEvent2D->Fill(hits2D.getEntries());

  //we should
  // -get the track candidate
  // -extrapolate to the layer in question
  // -check if there is any hit close enough within it size (number of strips)
  //  float r1=m_GeoPar->getLayerInnerRadius(1)+2;
  float r1 = m_GeoPar->getActiveMiddleRadius(BKLM_FORWARD, 1, 1);
  //there is no layer 16, so counting the tracks found with no layer excluded should give all 'regular' tracks
  for (int effLayer = 1; effLayer <= 16; effLayer++) {
    int numTracks = 0;
    //for each efficiency layer, memorize the points used, so we don't find the same track twice
    m_pointIndices.clear();
    float effX = 0;

    if (effLayer < 16) {
      effX = m_GeoPar->getActiveMiddleRadius(BKLM_FORWARD, 1, effLayer) - r1;
    }
    //exclude layer for which we want to measure the efficiency

    for (int h = 0; h < hits2D.getEntries() - 1; ++h) {
      int layer1 = hits2D[h]->getLayer();
      if (effLayer == layer1)
        continue;
      if (m_pointIndices.find(h) != m_pointIndices.end()) {

        continue;
      }
      for (int h2 = h + 1; h2 < hits2D.getEntries(); h2++) {
        int layer2 = hits2D[h2]->getLayer();
        if (effLayer == layer1)
          continue;
        if (abs(layer1 - layer2) < minSeedLayerDistance)
          continue;

        //for good extrapolation, we don't want to be too far away...

        // this only makes sense if we only use these two points for the extrapolation...
        ///since we use all points on the track now with a certain minimum, get rid of this requirement
        //        if(abs(effLayer-layer1) > 2 &&abs(effLayer-layer2) > 2)
        //    continue;


        //check if these hits have already been used for tracks
        if (m_pointIndices.find(h2) != m_pointIndices.end()) {
          continue;
        }

        int sector1 = hits2D[h]->getSector();
        int fwd1 = 0;
        if (hits2D[h]->isForward())
          fwd1 = 1;
        int sector2 = hits2D[h2]->getSector();
        int fwd2 = 0;
        if (hits2D[h]->isForward())
          fwd2 = 1;

        //let's stay in the same module...(that is what we use for the simple tracking as well...)
        if (sector1 != sector2  || fwd1 != fwd2)
          continue;
        //module that defines the coordiante system used... use first layer, other layers should then be along the x axis
        const Belle2::bklm::Module* refMod = m_GeoPar->findModule(fwd1, sector1, 1);
        //require lever arm

        vector<SimplePoint*> points;
        //check if there is another point on this track (so not spurious... ->could also interpolate...)
        //the points vector contains then all points on that track (also the potential eff point in the eff layer..)
        if (!validTrackCandidate(h, h2, hits2D, points, refMod, effLayer))
          continue;
        numTracks++;
        TrackParams trackParams;
        trackFit(points, trackParams);
        //just did this to collect tracks
        try {
          if (effLayer >= 16) {
            //      cout <<endl<<"looking for all tracks"<<endl;
            throw string("all layers");
          }

          //the stuff below is only needed if we only extrapolate using the two seed points
          //    const Belle2::bklm::Module* m1 = m_GeoPar->findModule(fwd1, sector1, layer1);
          TVector3 gHitPos1 = hits2D[h]->getGlobalPosition();

          //for some bizarre reason, some parts of the BKLM software use tvector3, others the clhep classes...
          Hep3Vector gHit1_cl(gHitPos1.x(), gHitPos1.y(), gHitPos1.z());

          Hep3Vector lHitPos1_cl = refMod->globalToLocal(gHit1_cl);
          TVector3 lHitPos1(lHitPos1_cl.x(), lHitPos1_cl.y(), lHitPos1_cl.z());
          //the x of this layer

          //        float x1=0.5*(m_GeoPar->getLayerInnerRadius(layer1)+m_GeoPar->getLayerOuterRadius(layer1));
          float x1 = lHitPos1_cl.x();
          //float z1=gHitPos1.z();

          TVector3 gHitPos2 = hits2D[h2]->getGlobalPosition();
          Hep3Vector gHit2_cl(gHitPos2.x(), gHitPos2.y(), gHitPos2.z());
          const CLHEP::Hep3Vector lHitPos2_cl = refMod->globalToLocal(gHit2_cl);
          TVector3 lHitPos2(lHitPos2_cl.x(), lHitPos2_cl.y(), lHitPos2_cl.z());
          //        float x2=0.5*(m_GeoPar->getLayerInnerRadius(layer2)+m_GeoPar->getLayerOuterRadius(layer2));
          float  x2 = lHitPos2_cl.x();

          // UNUSED float z2=gHitPos2.z();
          //compute point where we expect hit, should be in frame of mod1
          TVector3 expHit = lHitPos1 + (lHitPos2 - lHitPos1) * ((effX - x1) / (x2 - x1));
          TVector3 expHitExtrapol(effX, trackParams.my * effX + trackParams.by, trackParams.mz * effX + trackParams.bz);

          //        TVector3 expHit(expHit_cl.x(),expHit_cl.y(),expHit_cl.z());
          //check if we find a point in effLayer which is also in this sector and close to the track
          bool found = false;
          for (int e = 0; e < hits2D.getEntries(); e++) {
            if ((hits2D[e]->getLayer() != effLayer) || (hits2D[e]->getSector() != sector1) || (hits2D[e]->isForward() != fwd1))
              continue;
            //looking at hit in this layer..., see how far away we are from the projected hit...
            TVector3 candGlPos = hits2D[e]->getGlobalPosition();
            Hep3Vector candGlPos_cl(candGlPos.x(), candGlPos.y(), candGlPos.z());
            const Hep3Vector candLocPos_cl = refMod->globalToLocal(candGlPos_cl);
            TVector3 candLocPos(candLocPos_cl.x(), candLocPos_cl.y(), candLocPos_cl.z());

            // UNUSED Double_t distance=(candLocPos-expHit).Mag();
            Double_t distanceExtrapol = (candLocPos - expHitExtrapol).Mag();

            //            cout <<" expHit: "; expHit.Print();cout <<"distance to hit: "; candLocPos.Print();
            //            cout <<"extrapolatedHit: "; expHitExtrapol.Print();
            //      cout <<" is " << distance << " or (extrapolated) " << distanceExtrapol<<endl;
            //            cout <<"subtract " << effX-r1 <<"?  effX: " << effX <<",r1: " << r1 <<" (this radius minus layer1 should bring you back)" <<endl;
            //      cout <<"angle is: " << (lHitPos2-lHitPos1).Angle(lHitPos1-candLocPos);
            //        cout <<" or " <<  (lHitPos2-lHitPos1).Angle(lHitPos2-candLocPos);
            if (distanceExtrapol < m_maxEffDistance)
              found = true;

          }
          if (found)
            m_eff2DFound[sector1 - 1][effLayer - 1][fwd1]->Fill(expHitExtrapol.Y(), expHitExtrapol.Z());
          m_eff2DExpected[sector1 - 1][effLayer - 1][fwd1]->Fill(expHitExtrapol.Y(), expHitExtrapol.Z());



          //now that we checked the efficieny for this track, lets delete the points for the track
          //however, we memorize the ids used for that track, so we don't use the same points for a different track
          //for the same eff plane
        } catch (string& str) {
          //we land here if the eff.layer is outside the range of existing layeres
          //let's compute theta, phi of the track in the global reference frame
          //grab two points in the local system, transform to global and take the difference vector
          //if we assume that the track comes from (0,0,0) we'd only need one...
          Hep3Vector v1(0, trackParams.by, trackParams.bz);
          Hep3Vector v2(1, trackParams.by + trackParams.my, trackParams.bz + trackParams.mz);
          Hep3Vector gv1 = refMod->localToGlobal(v1);
          Hep3Vector gv2 = refMod->localToGlobal(v2);
          Hep3Vector vDiff = gv2 - gv1;
          float theta = vDiff.theta();
          if (theta < 0)
            theta *= (-1);
          float phi = vDiff.phi();
          if (phi < 0)
            phi += (2 * TMath::Pi());
          m_hTrackTheta->Fill(theta);
          m_hTrackPhi->Fill(phi);
        }
        if (effLayer == 16) {
          // UNUSED for(auto pi : m_pointIndices)
          {
            //          cout <<"using index : " <<pi<<endl;
          }
        }
        for (unsigned int i = 0; i < points.size(); i++) {
          if (effLayer == 16) {
            //      cout<<"x: "<< points[i]->x <<" y: " << points[i]->y << " z: "<< points[i]->z<<endl;
          }
          delete points[i];
        }
        points.clear();
      }

    }
    if (effLayer == 16) {
      m_hTracksPerEvent->Fill(numTracks);
    }
  }



}



//check if we find another hit in this module that is close to the track
bool BKLMEffnRadioModule::validTrackCandidate(int firstHit, int secondHit,  StoreArray<BKLMHit2d>& bklmHits2D,
                                              vector<SimplePoint*>& points, const Belle2::bklm::Module* refMod, int effLayer)
{

  set<int> locIndices;
  if (points.size() > 0) {
    B2DEBUG(1, "point vector not empty...");
  }

  int layer1 = bklmHits2D[firstHit]->getLayer();
  int sector1 = bklmHits2D[firstHit]->getSector();
  int fwd1 = 0;

  if (bklmHits2D[firstHit]->isForward())
    fwd1 = 1;

  int layer2 = bklmHits2D[secondHit]->getLayer();
  int sector2 = bklmHits2D[secondHit]->getSector();
  int fwd2 = 0;

  if (bklmHits2D[secondHit]->isForward())
    fwd2 = 1;


  for (int h = 0; h < bklmHits2D.getEntries(); ++h) {
    int layer = bklmHits2D[h]->getLayer();
    int sector = bklmHits2D[h]->getSector();
    if (layer == effLayer)
      continue;
    //differnt layer, same sector
    if ((layer == layer1) || (layer == layer2))
      continue;
    //same sector necessary? probably not, the test is pretty simple...
    if ((sector != sector1) || (sector != sector2) || (fwd1 != bklmHits2D[h]->isForward()) || (fwd2 != fwd1))
      continue;
    //don't use points that are already part of other tracks... So don't allow sharing
    //tried only to disallow the same seeds but that doesn't seem to be enough...
    if (m_pointIndices.find(h) != m_pointIndices.end())
      continue;

    TVector3 gl1 = bklmHits2D[firstHit]->getGlobalPosition();
    TVector3 gl2 = bklmHits2D[secondHit]->getGlobalPosition();
    TVector3 gl = bklmHits2D[h]->getGlobalPosition();
    TVector3 diff1 = gl1 - gl;
    TVector3 diff2 = gl2 - gl;
    //if gl, gl1 and gl2 lie on the same line (track), the angle between the respective difference vectors should be small..
    Double_t angle = diff1.Angle(diff2);
    if (angle > TMath::Pi()) {
      angle -= 2 * TMath::Pi();
    }
    if (angle < ((-1)*TMath::Pi())) {
      angle += 2 * TMath::Pi();
    }
    if (fabs(angle) < TMath::Pi() * 0.1) {
      //    cout <<"found point on track, angle: " << angle <<endl;
      //    cout <<"gl1: ("<<gl1.x()<<","<<gl1.y()<<","<< gl1.z()<<")"<<endl;
      //    cout <<"gl2: ("<<gl2.x()<<","<<gl2.y()<<","<<gl2.z()<<")"<<endl;
      //    cout <<"gl: ("<<gl.x()<<","<<gl.y()<<","<< gl.z()<<")"<<endl;
      //    cout <<"------"<<endl;
      //we found a point on the track, so let's add it to the points of the track...
      //but first translate it into the reference coordinate system

      TVector3 candGlPos = bklmHits2D[h]->getGlobalPosition();
      Hep3Vector candGlPos_cl(candGlPos.x(), candGlPos.y(), candGlPos.z());
      const Hep3Vector candLocPos_cl = refMod->globalToLocal(candGlPos_cl);
      SimplePoint* p = new SimplePoint;
      p->x = candLocPos_cl.x();
      p->y = candLocPos_cl.y();
      p->z = candLocPos_cl.z();
      points.push_back(p);
      locIndices.insert(h);
    }


  }
  if ((int)points.size() >= m_minNumPointsOnTrack - 2) {
    //add seed points

    TVector3 candGlPos = bklmHits2D[firstHit]->getGlobalPosition();
    Hep3Vector candGlPos_cl(candGlPos.x(), candGlPos.y(), candGlPos.z());
    const Hep3Vector candLocPos_cl = refMod->globalToLocal(candGlPos_cl);
    SimplePoint* p = new SimplePoint;
    p->x = candLocPos_cl.x();
    p->y = candLocPos_cl.y();
    p->z = candLocPos_cl.z();
    points.push_back(p);

    candGlPos = bklmHits2D[secondHit]->getGlobalPosition();
    Hep3Vector candGlPos_cl2(candGlPos.x(), candGlPos.y(), candGlPos.z());
    const Hep3Vector candLocPos_cl2 = refMod->globalToLocal(candGlPos_cl2);
    SimplePoint* p2 = new SimplePoint;
    p2->x = candLocPos_cl2.x();
    p2->y = candLocPos_cl2.y();
    p2->z = candLocPos_cl2.z();
    points.push_back(p2);

    m_pointIndices.insert(firstHit);
    m_pointIndices.insert(secondHit);
    for (auto pointIndex : locIndices) {
      m_pointIndices.insert(pointIndex);
    }
    return true;

  } else {
    //delete what we collected and return false
    for (unsigned int i = 0; i < points.size(); i++) {
      delete points[i];
    }
    points.clear();
    return false;
  }

  return false;

}


//at the moment we are not doing a refit without potential outliers
void BKLMEffnRadioModule::trackFit(vector<SimplePoint*>& points, TrackParams& params)
{
  Double_t A = 0, B = 0, Cx = 0, Cy = 0, D = 0, Ex = 0, Ey = 0;
  for (auto iter : points) {

    //The tracks in the barrel go out along the x axis.
    //So we flip x and z here, since the below is written for tracks along z
    Double_t x = iter->z;
    Double_t y = iter->y;
    Double_t z = iter->x;
    //    cout <<"x: " << x << " y: " << y << " z: " << z <<endl;
    A += z * z;
    B += z;
    Cx += x * z;
    Cy += y * z;
    Ex += x;
    Ey += y;

  }
  D = points.size();

  Double_t denom = D * A - B * B;
  if (denom) {
    Double_t bx = (-B * Cx + A * Ex) / denom;
    Double_t by = (-B * Cy + A * Ey) / denom;
    Double_t mx = (D * Cx - B * Ex) / denom;
    Double_t my = (D * Cy - B * Ey) / denom;
    params.bz = bx;
    params.by = by;
    params.mz = mx;
    params.my = my;

  } else {
    params.bz = -1;
    params.by = -1;
    params.mz = -1;
    params.my = -1;
  }

  //and here we flip x-z back..


}
