/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//my
#include <arich/utility/ARICHAerogelHist.h>

//c, c++
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>

//root
#include <TCanvas.h>
#include <TMath.h>
#include <TPad.h>
#include <TString.h>
#include <TFile.h>
#include <TAxis.h>

using namespace std;
using namespace Belle2;

ARICHAerogelHist::ARICHAerogelHist(const char* name, const char* title) : TH2Poly(), m_histName(name), m_histTitle(title)
{
  SetName(m_histName.Data());
  SetTitle(m_histTitle.Data());
  SetInitialParametersByDefault();
  SetUpVerticesMap();

  if (m_verboseLevel > 0) {
    std::cout << setw(10) << "ring" << setw(10) << "nTiles" << setw(10) << "tileRmin" << setw(10) << "tileRmax" << std::endl;
    for (unsigned int i = 0; i < m_nTiles.size(); i++)
      std::cout << setw(10) << i + 1 << setw(10) << m_nTiles[i] << setw(10) << m_tileRmin[i] << setw(10) << m_tileRmax[i] << std::endl;
    dumpVerticesMap();
  }

  unsigned int n;
  double* x;
  double* y;

  double xold;
  double yold;
  double xnew;
  double ynew;
  double phi;

  //Add "poly bins" to the histogram
  //Loop over rings with different radiuses
  unsigned int iR = 0;
  for (auto& m : m_verticesMap) {
    //Loop aerogel tiles (bins) within one ring.
    double phi0 = m_tileDeltaPhiCenter[iR] / 2.0;
    double deltaPhi = m_tileDeltaPhiCenter[iR] + m_aerogelAriGapDeltaPhiCenter[iR];
    for (Int_t j = 0; j < m_nTiles[iR]; j++) {
      phi = phi0 + deltaPhi * j;
      n = m.second.size();
      x = new double [n];
      y = new double [n];
      //Loop over polygonal points which defines bins.
      for (unsigned int i = 0; i < n; i++) {
        xold = m.second[i].X();
        yold = m.second[i].Y();
        makeRotation(xold, yold, xnew, ynew, phi);
        x[i] = xnew;
        y[i] = ynew;
      }
      AddBin(n, x, y);
      delete []x;
      delete []y;
    }
    iR++;
  }// for (auto& m: m_verticesMap) {

}

Int_t ARICHAerogelHist::GetBinIDFromRingColumn(Int_t ring, Int_t column)
{

  Int_t binID = 0;
  unsigned int ringu = ring;
  //cout<<" ewewe "<<m_nTiles[ring-1]<<endl;
  if (ringu > m_nTiles.size() || ringu < 1)
    return -999;
  if (column > m_nTiles[ring - 1] || column < 1)
    return -999;
  for (Int_t i = 1; i < ring; i++)
    binID += m_nTiles[i - 1];
  return binID + column;

}

void ARICHAerogelHist::makeRotation(double xold, double yold, double& xnew, double& ynew, double phi)
{
  xnew = xold * std::cos(phi) - yold * std::sin(phi);
  ynew = xold * std::sin(phi) + yold * std::cos(phi);
}

void ARICHAerogelHist::DrawHisto(TString opt = "ZCOLOT text same", TString outDirName = "./")
{

  TCanvas* c1 = new TCanvas("c1", "c1", 1000, 1000);
  c1->SetTitle(m_histTitle.Data());
  c1->SetName(m_histName.Data());
  c1->SetRightMargin(0.17);
  c1->SetLeftMargin(0.12);
  c1->SetTopMargin(0.15);
  c1->SetBottomMargin(0.15);

  TH2F* frame = new TH2F("h2", "h2", 40, -1200, 1200, 40, -1200, 1200);
  //TH2F *frame = new TH2F("h2","h2",40, 400, 650, 40, -50, 200);
  frame->GetXaxis()->SetTitle("x (mm)");
  frame->GetYaxis()->SetTitle("y (mm)");
  frame->GetXaxis()->CenterTitle();
  frame->GetYaxis()->CenterTitle();
  frame->GetYaxis()->SetTitleOffset(1.5);
  frame->SetStats(kFALSE);
  frame->SetTitle("");
  frame->Draw();

  Draw(opt.Data());
  c1->Modified();
  c1->Update();
  if (outDirName.Length() > 0) {
    std::cout << "outDirName.Length() " << outDirName.Length() << std::endl;
    TString outnamePDF = outDirName; outnamePDF += m_histName; outnamePDF += ".pdf";
    TString outnameEPS = outDirName; outnameEPS += m_histName; outnameEPS += ".eps";
    c1->SaveAs(outnamePDF.Data());
    c1->SaveAs(outnameEPS.Data());
  }

}

void ARICHAerogelHist::dumpVerticesMap()
{

  unsigned int i;
  for (auto& m : m_verticesMap) {
    std::cout << " --> Aerogel ring : " << m.first << '\n';
    for (i = 0; i < m.second.size(); i++) {
      std::cout << "                    " << setw(15) << m.second[i].X() << setw(15) << m.second[i].Y() << std::endl;
    }
  }

}

void ARICHAerogelHist::SetUpVerticesMap()
{

  for (unsigned int i = 0; i < m_nTiles.size(); i++) {
    std::vector<ROOT::Math::XYVector> vecTvec;
    int nTiles = m_nTiles[i];
    double rmin = m_tileRmin[i];
    double rmax = m_tileRmax[i];
    double lmin = 2 * TMath::Pi() * rmin / nTiles - m_aerogelTileGap;
    double lmax = 2 * TMath::Pi() * rmax / nTiles - m_aerogelTileGap;
    double phimin = lmin / rmin;
    double phimax = lmax / rmax;
    double x1 = rmin * TMath::Cos(phimin / 2.0);
    double y1 = rmin * TMath::Sin(phimin / 2.0);
    ROOT::Math::XYVector v1(x1, y1);
    vecTvec.push_back(v1);
    double x2 = rmax * TMath::Cos(phimax / 2.0);
    double y2 = rmax * TMath::Sin(phimax / 2.0);
    ROOT::Math::XYVector v2(x2, y2);
    vecTvec.push_back(v2);
    //Add circular points from outer radious (clockwise added)
    if (m_nCircularPoints > 0) {
      double dPhi = phimax / (m_nCircularPoints + 1);
      for (int j = 0; j < m_nCircularPoints; j++) {
        const double phi = phimax / 2.0 - dPhi * (j + 1);
        ROOT::Math::XYVector v(rmax * std::cos(phi), rmax * std::sin(phi));
        vecTvec.push_back(v);
      }
    }

    double x3 =  x2;
    double y3 = -y2;
    ROOT::Math::XYVector v3(x3, y3);
    vecTvec.push_back(v3);
    double x4 =  x1;
    double y4 = -y1;
    ROOT::Math::XYVector v4(x4, y4);
    vecTvec.push_back(v4);
    //Add circular points inner radious
    if (m_nCircularPoints > 0) {
      double dPhi = phimin / (m_nCircularPoints + 1);
      for (int j = 0; j < m_nCircularPoints; j++) {
        const double phi = -phimax / 2.0 + dPhi * (j + 1);
        ROOT::Math::XYVector v(rmax * std::cos(phi), rmax * std::sin(phi));
        vecTvec.push_back(v);
      }
    }

    vecTvec.push_back(v1);
    m_verticesMap[i] = vecTvec;
  }

}

void ARICHAerogelHist::SetInitialParametersByDefault()
{

  //Verbose level
  m_verboseLevel = 0;
  //Distance between aerogel tiles
  m_aerogelTileGap = 1;
  //Number of circular points
  m_nCircularPoints = 10;
  //Number of tiles per ring.
  m_nTiles.push_back(22);
  m_nTiles.push_back(28);
  m_nTiles.push_back(34);
  m_nTiles.push_back(40);
  //Minimum radius of aerogel ring
  m_tileRmin.push_back(441.0);
  m_tileRmin.push_back(614.0);
  m_tileRmin.push_back(787.0);
  m_tileRmin.push_back(960.0);
  //Maximum radius of aerogel ring
  m_tileRmax.push_back(613.0);
  m_tileRmax.push_back(786.0);
  m_tileRmax.push_back(959.0);
  m_tileRmax.push_back(1117.0);
  for (unsigned int i = 0; i < m_nTiles.size(); i++) {
    m_tileRcenter.push_back((m_tileRmax[i] + m_tileRmin[i]) / 2.0);
    double r = m_tileRcenter[i];
    double l = 2 * TMath::Pi() * r / m_nTiles[i] - m_aerogelTileGap;
    double phi = l / r;
    m_tileDeltaPhiCenter.push_back(phi);
    m_aerogelAriGapDeltaPhiCenter.push_back(m_aerogelTileGap / r);
  }

}
