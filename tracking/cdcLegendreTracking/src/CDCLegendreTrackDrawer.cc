/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreTrackDrawer.h>
#include "tracking/cdcLegendreTracking/CDCLegendreNiceDrawing.h"
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

#include <framework/core/Module.h>
#include "framework/datastore/StoreArray.h"
#include "cdc/dataobjects/CDCHit.h"

#include <iomanip>

#include "TF1.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TEllipse.h"


using namespace Belle2;


CDCLegendreTrackDrawer::CDCLegendreTrackDrawer(bool drawCandInfo, bool drawCandidates) :
  m_drawCandInfo(drawCandInfo), m_drawCandidates(drawCandidates), m_rMin(-0.15), m_rMax(0.15)
{

}

void CDCLegendreTrackDrawer::initialize()
{
  m_StoreDirectory = std::string("tmp/visualization");

  if (not m_drawCandInfo) return;

  m_zReference = 25.852;

  m_eventCounter = 0;

  m_max = 800;

  m_realMax = 250;

  m_rCDC = 113;

  m_scale = m_max / float(m_realMax);

  initColorVec();

  initWireString();

  StoreArray<CDCHit>::required("CDCHits");

  initRootColorVec();

}

void CDCLegendreTrackDrawer::event()
{
  if (not m_drawCandInfo) return;

  m_eventCounter += 1;

  m_trackCounter = 1;

  initFig();

  drawWires();

  drawCDCHits();

  m_iTrack = 0;


//  m_fig << "</svg>\n";
//  m_fig.close();

//  std::stringstream ss;
//  ss << m_StoreDirectory << std::setfill('0') << std::setw(4) << m_eventCounter << "_cdc.svg";

//  pipeout_fp = popen(Form("display %s", ss.str().c_str()), "w");

//  openFileAgain();


}

void CDCLegendreTrackDrawer::drawHitsROOT(std::vector<CDCLegendreTrackHit*>& hits_vector)
{
  drawConformalHits(hits_vector, -1, false);
  drawLegendreHits(hits_vector, -1, false);
}

void CDCLegendreTrackDrawer::drawTrackCand(CDCLegendreTrackCandidate* TrackCand)
{
  if (not m_drawCandInfo) return;

  StoreArray<genfit::TrackCand> CandArray(m_TrackCandColName);
  StoreArray<CDCHit> HitArray(m_HitColName);

  std::stringstream ss;
  std::string trackColor = getColor(m_iTrack);
  TVector2 momentum(TrackCand->getMomentumEstimation().X(), TrackCand->getMomentumEstimation().Y());
  TVector2 position(TrackCand->getReferencePoint().X(), TrackCand->getReferencePoint().Y());
  int charge = TrackCand->getChargeSign() * 1.1;

  drawAnyTrack(ss, momentum, charge, trackColor, position);

  for (CDCLegendreTrackHit * hit : TrackCand->getTrackHits()) {
    CDCHit* TrackHit = HitArray[hit->getStoreIndex()];

    drawCDCHit(ss, TrackHit, trackColor);
  }

  drawConformalHits((TrackCand->getTrackHits()), m_iTrack, false);
  drawLegendreHits((TrackCand->getTrackHits()), m_iTrack, false);


  m_fig << ss.str();

  m_iTrack++;


}

void CDCLegendreTrackDrawer::finalizeFile()
{

  if (!m_drawCandInfo) return;

  m_fig << "</svg>\n";
  m_fig.close();

}


void CDCLegendreTrackDrawer::finalizeROOTFile(std::vector<CDCLegendreTrackHit*>& hits_vector)
{
  if (not m_drawCandInfo) return;

  drawConformalHits(hits_vector, -1, true);
  drawLegendreHits(hits_vector, -1, true);

  for (TEllipse * hit_conformal : m_hitsConformal) {
    delete hit_conformal;
  }
  m_hitsConformal.clear();

  for (TF1 * hit_legendre : m_hitsLegendre) {
    delete hit_legendre;
  }
  m_hitsLegendre.clear();

}


void CDCLegendreTrackDrawer::showPicture()
{

  if (!(m_drawCandidates && m_drawCandInfo)) return;

  finalizeFile();

  std::stringstream ss;
  ss << m_StoreDirectory << std::setfill('0') << std::setw(4) << m_eventCounter << "_cdc.svg";

  std::stringstream ss_out;
  ss_out << m_StoreDirectory << std::setfill('0') << std::setw(4) << m_eventCounter << "_cdc_" << m_trackCounter << ".svg";




//  if (m_drawCandidates) system(Form("display %s", ss.str().c_str()));
  if (m_drawCandidates) system(Form("cp %s %s", ss.str().c_str(),  ss_out.str().c_str()));

  openFileAgain();

  m_trackCounter++;
  //        std::cin.ignore();

}

void CDCLegendreTrackDrawer::openFileAgain()
{
  if (!m_drawCandInfo) return;

  std::stringstream ss;
  std::stringstream ss_temp;
  ss << m_StoreDirectory << std::setfill('0') << std::setw(4) << m_eventCounter << "_cdc.svg";
  ss_temp << m_StoreDirectory << std::setfill('0') << std::setw(4) << m_eventCounter << "_cdc_temp.svg";

//  m_fig.open(ss.str().c_str());

//  m_fig << "<?xml version=\"1.0\" ?> \n<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"" << m_max << "pt\" height=\"" << m_max << "pt\" viewBox=\"0 0 " << m_max << " " << m_max << "\" version=\"1.1\">\n";

  std::string line, svg_line;
  svg_line = std::string("</svg>");
  std::ifstream file;
  std::ofstream outfile;
  file.open(ss.str().c_str());
  outfile.open(ss_temp.str().c_str());
  while (getline(file, line)) {
    if (line == svg_line) {}
    else {outfile << line << std::endl;}
  }
  outfile.close();
  file.close();
  remove(ss.str().c_str());
  rename(ss_temp.str().c_str(), ss.str().c_str());

  m_fig.open(ss.str().c_str(), fstream::in | fstream::app);

}


void CDCLegendreTrackDrawer::drawConformalHits(std::vector<CDCLegendreTrackHit*> trackHitList, int ntrack, bool do_print = false)
{
  if (not m_drawCandInfo) return;

  if (!do_print) {
    double x0, y0, R;
    double norm;
    norm = 1. / (m_rMax - m_rMin);
    Color_t trackColor;
    if (ntrack >= 0)trackColor = getRootColor(ntrack);
    else trackColor = kBlue;
    BOOST_FOREACH(CDCLegendreTrackHit * hit, trackHitList) {
      x0 = hit->getConformalX();
      y0 = hit->getConformalY();
      R = hit->getConformalDriftTime();
      TEllipse* hit_conformal = new TEllipse((x0 - m_rMin)*norm, (y0 - m_rMin)*norm, R * norm, R * norm);
      //    TEllipse* hit_conformal = new TEllipse(x0,y0,R, R);
      hit_conformal->SetFillStyle(0);
      hit_conformal->SetLineColor(trackColor);
      hit_conformal->SetLineWidth(3);
      if (ntrack == -1)hit_conformal->SetLineStyle(2);
      m_hitsConformal.push_back(hit_conformal);
    }
  } else {
    static int nevent;
    nevent++;
    TCanvas* canv = new TCanvas("canv", "conformal space", 0, 0, 600, 600);
    canv->Range(0, 0, 1, 1);

    BOOST_FOREACH(TEllipse * hit_conformal, m_hitsConformal) {
      hit_conformal->Draw();
    }

    canv->Print(Form("conformalHits_%i.root", nevent));
    canv->Print(Form("conformalHits_%i.eps", nevent));
    canv->Print(Form("conformalHits_%i.png", nevent));
  }
}


void CDCLegendreTrackDrawer::drawLegendreHits(std::vector<CDCLegendreTrackHit*> trackHitList, int ntrack, bool do_print = false)
{
  if (not m_drawCandInfo) return;

  if (!do_print) {

    int nhits = 0;
    double x0, y0, R;
    Color_t trackColor;
    if (ntrack >= 0)trackColor = getRootColor(ntrack);
    else trackColor = kBlue;
    BOOST_FOREACH(CDCLegendreTrackHit * hit, trackHitList) {
      TF1* funct1 = new TF1("funct", "[0]*cos(x)+[1]*sin(x)+[2]", 0, m_PI);
      TF1* funct2 = new TF1("funct", "[0]*cos(x)+[1]*sin(x)-[2]", 0, m_PI);
      funct1->SetLineWidth(0.25);
      funct2->SetLineWidth(0.25);
      x0 = hit->getConformalX();
      y0 = hit->getConformalY();
      R = hit->getConformalDriftTime();
      funct1->SetParameters(x0, y0, R);
      funct2->SetParameters(x0, y0, R);
      funct1->SetLineColor(trackColor);
      funct2->SetLineColor(trackColor);
//      if(ntrack == -1)funct1->SetLineStyle(2);
//      if(ntrack == -1)funct2->SetLineStyle(2);
      m_hitsLegendre.push_back(funct1);
      m_hitsLegendre.push_back(funct2);
    }
  } else {
    static int nevent;
    nevent++;

    TCanvas* canv = new TCanvas("canv", "legendre transform", 0, 0, 1200, 600);
    canv->cd(1);
    TGraph* dummyGraph = new TGraph();
    dummyGraph->SetPoint(1, 0, 0);
    dummyGraph->SetPoint(2, m_PI, 0);
    dummyGraph->Draw("AP");
    dummyGraph->GetXaxis()->SetTitle("#theta");
    dummyGraph->GetYaxis()->SetTitle("#rho");
    dummyGraph->GetXaxis()->SetRangeUser(0, m_PI);
    dummyGraph->GetYaxis()->SetRangeUser(m_rMin, m_rMax);


    BOOST_FOREACH(TF1 * hit_legendre, m_hitsLegendre) {
      hit_legendre->Draw("CSAME");
    }

    canv->Print(Form("legendreHits_%i.root", nevent));
    canv->Print(Form("legendreHits_%i.eps", nevent));
    canv->Print(Form("legendreHits_%i.png", nevent));
  }
}


void CDCLegendreTrackDrawer::initRootColorVec()
{
//  m_colorVec.push_back("#0343df"); //blue
  m_colorRootVec.push_back(kGreen); //green
  m_colorRootVec.push_back(kRed); //red
  m_colorRootVec.push_back(kOrange); //orange
  m_colorRootVec.push_back(kOrange + 1); //orange
  m_colorRootVec.push_back(kOrange + 2); //orange
  m_colorRootVec.push_back(kRed + 1); //dark red
  m_colorRootVec.push_back(kRed + 2); //dark red
  m_colorRootVec.push_back(kRed - 1); //dark red
  m_colorRootVec.push_back(kCyan); //cyan
  m_colorRootVec.push_back(kCyan + 1); //cyan
  m_colorRootVec.push_back(kCyan + 3); //cyan
  m_colorRootVec.push_back(kTeal + 2); //dark teal
  m_colorRootVec.push_back(kTeal + 4); //dark teal
  m_colorRootVec.push_back(kGreen + 1); //puke green
  m_colorRootVec.push_back(kGreen + 3); //puke green
  m_colorRootVec.push_back(kGreen - 1); //puke green
  m_colorRootVec.push_back(kYellow + 1); //yellow green
  m_colorRootVec.push_back(kPink); //cobalt
  m_colorRootVec.push_back(kPink + 1); //cobalt
  m_colorRootVec.push_back(kPink + 2); //cobalt
}


Color_t CDCLegendreTrackDrawer::getRootColor(int i)
{
  int iColor = i % m_colorVec.size();

  return m_colorRootVec[iColor];
}
