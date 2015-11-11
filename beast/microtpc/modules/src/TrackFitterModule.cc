/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/microtpc/modules/TpcDigitizerModule.h>
#include <beast/microtpc/modules/TrackFitterModule.h>
#include <beast/microtpc/dataobjects/MicrotpcSimHit.h>
#include <beast/microtpc/dataobjects/MicrotpcHit.h>
#include <beast/microtpc/dataobjects/MicrotpcRecoTrack.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/RandomNumbers.h>

//c++
#include <cmath>
#include <boost/foreach.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

// ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>


using namespace std;
using namespace Belle2;
using namespace microtpc;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackFitter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackFitterModule::TrackFitterModule() : Module()
{
  // Set module properties
  setDescription("Track fitter module");

  //Default values are set here. New values can be in MICROTPC.xml.
  addParam("GEMGain1", m_GEMGain1, "GEM1 gain", 10.0);
  addParam("GEMGain2", m_GEMGain2, "GEM1 gain", 20.0);
  addParam("GainRMS1", m_GEMGainRMS1, "GEM1 rms", 0.2);
  addParam("GainRMS2", m_GEMGainRMS2, "GEM1 rms", 0.2);
  addParam("ScaleGain1", m_ScaleGain1, "scale gain 1 by a factor", 2.0);
  addParam("ScaleGain2", m_ScaleGain2, "scale gain 2 by a factor", 4.0);
  addParam("GEMpitch", m_GEMpitch, "GEM pitch", 0.014);
  addParam("PixelThreshold", m_PixelThreshold, "Pixel threshold in [e]", 3000);
  addParam("PixelThresholdRMS", m_PixelThresholdRMS, "Pixel threshold rms in [e]", 150);
  addParam("ChipRowNb", m_ChipRowNb, "Chip number of row", 226);
  addParam("ChipColumnNb", m_ChipColumnNb, "Chip number of column", 80);
  addParam("ChipColumnX", m_ChipColumnX, "Chip x dimension in cm / 2", 1.0);
  addParam("ChipRowY", m_ChipRowY, "Chip y dimension in cm / 2", 0.86);
  addParam("PixelTimeBinNb", m_PixelTimeBinNb, "Pixel number of time bin", 256);
  addParam("PixelTimeBin", m_PixelTimeBin, "Pixel time bin in ns", 25.0);
  addParam("TOTA1", m_TOTA1, "TOT factor A 1", 24.4678);
  addParam("TOTB1", m_TOTB1, "TOT factor B 1", -34.7008);
  addParam("TOTC1", m_TOTC1, "TOT factor C 1", 264.282);
  addParam("TOTQ1", m_TOTQ1, "TOT factor Q 1", 57.);
  addParam("TOTA2", m_TOTA2, "TOT factor A 2", 24.4678);
  addParam("TOTB2", m_TOTB2, "TOT factor B 2", -34.7008);
  addParam("TOTC2", m_TOTC2, "TOT factor C 2", 264.282);
  addParam("TOTQ2", m_TOTQ2, "TOT factor Q 2", 57.);
  addParam("z_DG", m_z_DG, "Drift gap distance [cm]", 12.0);
  addParam("z_TG", m_z_TG, "Transfer gap distance [cm]", 0.28);
  addParam("z_CG", m_z_CG, "Collection gap distance [cm]", 0.34);
  addParam("Dt_DG", m_Dt_DG, "Transverse diffusion in drift gap [cm^-1]", 0.0129286);
  addParam("Dt_TG", m_Dt_TG, "Transverse diffusion in transfer gap [cm^-1]", 0.0153198);
  addParam("Dt_CG", m_Dt_CG, "Transverse diffusion in collection gap [cm^1]", 0.014713);
  addParam("Dl_DG", m_Dl_DG, "Longitudinal diffusion in drift gap [cm^-1]", 0.0124361);
  addParam("Dl_TG", m_Dl_TG, "Longitudinal diffusion in transfer gap distance [cm^-1]", 0.0131141);
  addParam("Dl_CG", m_Dl_CG, "Longitudinal diffusion in collection gap [cm^-1]", 0.0134958);
  addParam("v_DG", m_v_DG, "Drift velocity in gap distance [cm/ns]", 0.00100675);
  addParam("v_TG", m_v_TG, "Drift velocity in transfer gap [cm/ns]", 0.0004079);
  addParam("v_CG", m_v_CG, "Drift velocity in collection gap [cm/ns]", 0.00038828);
  //addParam("P_vessel", m_P_vessel,"Pressure in vessel [atm]",1.0);
  addParam("Workfct", m_Workfct, "Work function", 35.075);
  addParam("Fanofac", m_Fanofac, "Fano factor", 0.19);
  addParam("GasAbs", m_GasAbs, "Gas absorption", 0.05);

}

TrackFitterModule::~TrackFitterModule()
{
}

void TrackFitterModule::initialize()
{
  B2INFO("TrackFitter: Initializing");
  StoreArray<MicrotpcRecoTrack>::registerPersistent();

  //get the garfield drift data, gas, and TPC paramters
  getXMLData();

  //fctToT_Calib1 = new TF1("fctToT_Calib1", "[0]*(x/[3]+[1])/(x/[3]+[2])", 0., 100000.);
  //fctToT_Calib1->SetParameters(m_TOTA1, m_TOTB1, m_TOTC1, m_TOTQ1);

  //fctToT_Calib2 = new TF1("fctToT_Calib2", "[0]*(x/[3]+[1])/(x/[3]+[2])", 0., 100000.);
  //fctToT_Calib2->SetParameters(m_TOTA2, m_TOTB2, m_TOTC2, m_TOTQ2);

  fctQ_Calib1 = new TF1("fctQ_Calib1", "[0]*([1]*x-[2])/([3]-x)", 0., 15.);
  fctQ_Calib1->SetParameters(m_TOTQ1, m_TOTC1, m_TOTA1 * m_TOTB1, m_TOTA1);

  fctQ_Calib2 = new TF1("fctQ_Calib2", "[0]*([1]*x-[2])/([3]-x)", 0., 15.);
  fctQ_Calib2->SetParameters(m_TOTQ2, m_TOTC2, m_TOTA2 * m_TOTB2, m_TOTA2);

}

void TrackFitterModule::beginRun()
{
}

void TrackFitterModule::event()
{

  StoreArray<MCParticle> particles;
  StoreArray<MicrotpcSimHit> TpcSimHits;
  StoreArray<MicrotpcHit> TpcHits;

  //Skip events with no TpcSimHits, but continue the event counter
  if (TpcHits.getEntries() == 0) {
    Event++;
    return;
  }
  int nentries = TpcHits.getEntries();
  //auto columnArray = new vector<int>[nTPC](); //column
  //auto rowArray = new vector<int>[nTPC](); //row
  //auto bcidArray = new vector<int>[nTPC](); //BCID
  //auto totArray = new vector<int>[nTPC](); //TOT
  int i_tpc[8];
  //int i_tpc1[8];
  //int i_tpc2[8];
  if (nentries > 0) {

    for (int i = 0; i < nTPC; i++) {

      i_tpc[i] = 0;

      for (int j = 0; j < MAXSIZE; j++) {
        x[j] = 0;
        y[j] = 0;
        z[j] = 0;
        e[j] = 0;
      }

      Track = new TGraph2DErrors();

      for (int j = 0; j < 16; j++) m_side[j] = 0;
      /*
      for (int k = 0; k < 4; k++)  {
      m_side1[k] = 0;
      m_side2[k] = 0;
      m_side3[k] = 0;
      m_side4[k] = 0;
      m_side5[k] = 0;
      }
      */

      int xnpts[4];
      int ynpts[4];
      for (int j = 0; j < 4; j++) {
        m_impact_x[j] = 0;
        m_impact_y[j] = 0;
        xnpts[j] = 0;
        ynpts[j] = 0;
      }

      //Determine T0 for each TPC
      //i_tpc1[i]=0;
      i_tpc[i] = 0;
      for (int j = 0; j < nentries; j++) {
        MicrotpcHit* aHit = TpcHits[j];
        int detNb = aHit->getdetNb();
        if (detNb == i) {
          //i_tpc1[i]++;
          i_tpc[i]++;
        }
      }
      if (i_tpc[i] == 0)continue;
      //int fpxhits = i_tpc1[i];
      int fpxhits = i_tpc[i];
      int time[fpxhits];
      //i_tpc2[i]=0;
      i_tpc[i] = 0;
      for (int j = 0; j < nentries; j++) {

        MicrotpcHit* aHit = TpcHits[j];
        int detNb = aHit->getdetNb();
        int bcid = aHit->getBCID();

        if (detNb == i) {
          time[i_tpc[i]] = bcid;
          //i_tpc2[i]++;
          i_tpc[i]++;
        }
      }
      i_tpc[i] = 0;
      int itime[fpxhits];
      TMath::Sort(fpxhits, time, itime, false);
      m_time_range = fabs(time[itime[0]] - time[itime[fpxhits - 1]]);

      int m_totsum = 0;
      float m_esum = 0;
      //loop on all entries to store in 3D the ionization for each TPC
      for (int j = 0; j < nentries; j++) {

        MicrotpcHit* aHit = TpcHits[j];
        int detNb = aHit->getdetNb();
        int col = aHit->getcolumn();
        int row = aHit->getrow();
        int tot = aHit->getTOT();
        int bcid = aHit->getBCID();

        if (detNb == i) {

          x[i_tpc[i]] = col * (2. * m_ChipColumnX / (float)m_ChipColumnNb) - m_ChipColumnX;
          y[i_tpc[i]] = row * (2. * m_ChipRowY / (float)m_ChipRowNb) - m_ChipRowY;
          z[i_tpc[i]] = (m_PixelTimeBin / 2. + m_PixelTimeBin * (bcid - time[itime[0]])) * m_v_DG;
          m_totsum += tot;
          if (tot < 3) e[i_tpc[i]] = fctQ_Calib1->Eval(tot) / (m_GEMGain1 * m_GEMGain2) * m_Workfct * 1e-3;
          else e[i_tpc[i]] = fctQ_Calib2->Eval(tot) / (m_GEMGain1 * m_GEMGain2) * m_Workfct * 1e-3;
          m_esum += e[i_tpc[i]];

          Track->SetPoint(i_tpc[i], x[i_tpc[i]], y[i_tpc[i]], z[i_tpc[i]]);
          //Track->SetPointError(i_tpc[i],0,0,e[i_tpc[i]]);
          Track->SetPointError(i_tpc[i], 0, 0, 1.);

          for (int k = 0; k < 4; k++) {
            if (0 <= col && col <= k)m_side[4 * k + 0] = k + 1;
            if (80 - k <= col && col <= 80)m_side[4 * k + 1] = k + 1;
            if (0 <= row && row <= 5 * k)m_side[4 * k + 2] = k + 1;
            if (336 - 5 * k <= row && row <= 336)m_side[4 * k + 3] = k + 1;
          }

          /*
          if(col == 1)m_side1[0]=1;
          if(1 <= col && col <= 2)m_side2[0]=1;
          if(1 <= col && col <= 3)m_side3[0]=1;
          if(1 <= col && col <= 4)m_side4[0]=1;
          if(1 <= col && col <= 5)m_side5[0]=1;
          if(col == 80)m_side1[0]=1;
          if(79 <= col && col <= 80)m_side2[1]=1;
          if(78 <= col && col <= 80)m_side3[1]=1;
          if(77 <= col && col <= 80)m_side4[1]=1;
          if(76 <= col && col <= 80)m_side5[1]=1;
          if(1 <= row && row <= 5)m_side1[2]=1;
          if(1 <= row && row <= 10)m_side2[2]=1;
          if(1 <= row && row <= 15)m_side3[2]=1;
          if(1 <= row && row <= 25)m_side4[2]=1;
          if(1 <= row && row <= 30)m_side5[2]=1;
          if(332 <= row && row <= 336)m_side1[3]=1;
          if(327 <= row && row <= 336)m_side2[3]=1;
          if(324 <= row && row <= 336)m_side3[3]=1;
          if(319 <= row && row <= 336)m_side4[3]=1;
          if(314 <= row && row <= 336)m_side5[3]=1;
          */
          if (col == 0) {
            m_impact_y[0] += y[i_tpc[i]];
            ynpts[0]++;
          }
          if (col == 1) {
            m_impact_y[1] += y[i_tpc[i]];
            ynpts[1]++;
          }
          if (col == 80) {
            m_impact_y[2] += y[i_tpc[i]];
            ynpts[2]++;
          }
          if (col == 79) {
            m_impact_y[3] += y[i_tpc[i]];
            ynpts[3]++;
          }
          if (row == 0) {
            m_impact_x[0] += x[i_tpc[i]];
            xnpts[0]++;
          }
          if (row == 1) {
            m_impact_x[1] += x[i_tpc[i]];
            xnpts[1]++;
          }
          if (row == 2) {
            m_impact_x[2] += x[i_tpc[i]];
            xnpts[2]++;
          }
          if (row == 3) {
            m_impact_x[3] += x[i_tpc[i]];
            xnpts[3]++;
          }
          i_tpc[i]++;
        }
      }
      fpxhits = i_tpc[i];

      if (fpxhits > 0) {

        for (int j = 0; j < 4; j++) {
          if (xnpts[j] > 0)m_impact_x[j] = m_impact_x[j] / ((double)xnpts[j]);
          if (ynpts[j] > 0)m_impact_y[j] = m_impact_y[j] / ((double)ynpts[j]);
        }

        L = new float[fpxhits];
        ix = new int[fpxhits];
        iy = new int[fpxhits];
        iz = new int[fpxhits];
        TMath::Sort(fpxhits, x, ix, false);
        TMath::Sort(fpxhits, y, iy, false);
        TMath::Sort(fpxhits, z, iz, false);

        TVirtualFitter::SetDefaultFitter("Minuit");
        TVirtualFitter* min = TVirtualFitter::Fitter(0, 5);
        min->SetDefaultFitter("Minuit");
        min->SetObjectFit(Track);
        min->SetFCN(SumDistance2_angles);

        Double_t arglist[6] = { -1, 0, 0, 0, 0, 0};
        min->ExecuteCommand("SET PRINT", arglist, 1);
        min->ExecuteCommand("SET NOWARNINGS", arglist, 0);

        XYZVector temp_vector3(0, 0, 0);
        float pStart[5] = {0, 0, 0, 0, 0};
        temp_vector3  = XYZVector(x[ix[fpxhits - 1]] - x[ix[0]] , y[iy[fpxhits - 1]] - y[iy[0]], z[iz[fpxhits - 1]] - z[iz[0]]);
        float init_theta = temp_vector3.Theta();
        float init_phi = temp_vector3.Phi();
        pStart[0] = x[ix[0]];
        pStart[1] = y[iy[0]];
        pStart[2] = z[iz[0]];
        pStart[3] = init_theta;
        pStart[4] = init_phi;

        min->SetParameter(0, "x0",    pStart[0], 0.01, 0, 0);
        min->SetParameter(1, "y0",    pStart[1], 0.01, 0, 0);
        min->SetParameter(2, "z0",    pStart[2], 0.01, 0, 0);
        min->SetParameter(3, "theta", pStart[3], 0.0001, 0, 0);
        min->SetParameter(4, "phi",   pStart[4], 0.0001, 0, 0);

        arglist[0] = 10000; // number of fucntion calls
        arglist[1] = 0.001; // tolerance
        min->ExecuteCommand("MIGRAD", arglist, 2);

        int nvpar, nparx;
        double amin, edm, errdef;
        min->GetStats(amin, edm, errdef, nvpar, nparx);
        m_chi2 = amin;

        for (int j = 0; j < 5; j++) {
          m_parFit[j] = 0;
          m_parFit_err[j] = 0;
          m_parFit[j] = min->GetParameter(j);
          m_parFit_err[i] = min->GetParError(j);
          for (int k = 0; k < 5; k++) {
            //m_cov[j][k] = 0;
            //m_cov[j][k] = min->GetCovarianceMatrixElement(j, k);
            m_cov[j * 5 + k] = 0;
            m_cov[j * 5 + k] = min->GetCovarianceMatrixElement(j, k);
          }
        }

        TVector3 TrackDir(1, 0, 0);
        TrackDir.SetTheta(m_parFit[3]);
        TrackDir.SetPhi(m_parFit[4]);

        m_theta = m_parFit[3] * TMath::RadToDeg();
        m_phi   = m_parFit[4] * TMath::RadToDeg();

        for (int j = 0; j < fpxhits; j++) {
          TVector3 Point(x[j], y[j], z[j]);
          L[j] = Point * TrackDir.Unit();
        }

        iL = new int [fpxhits];
        TMath::Sort(fpxhits, L, iL, false);
        m_trl = fabs(L[iL[fpxhits - 1]] - L[iL[0]]);

        StoreArray<MicrotpcRecoTrack> RecoTracks;
        RecoTracks.appendNew(i, m_chi2, m_theta, m_phi, m_esum, m_totsum, m_trl, m_time_range, m_parFit, m_parFit_err, m_cov, m_impact_x,
                             m_impact_y, m_side);

        delete [] iL;
        delete min;
        delete [] L;
        delete [] ix;
        delete [] iy;
        delete [] iz;

      }
      Track->Delete();
    }
  }

  Event++;

}

//double TrackFitterModule::distance2_angles(double px,double py,double pz, double *p)
double distance2_angles(double px, double py, double pz, double* p)
{
  XYZVector xp(px, py, pz);
  XYZVector x0(p[0], p[1], p[2]);
  XYZVector u(TMath::Sin(p[3])*TMath::Cos(p[4]), TMath::Sin(p[3])*TMath::Sin(p[4]), TMath::Cos(p[3]));
  double coeff = u.Dot(xp - x0);
  XYZVector n = xp - x0 - coeff * u;

  double dx = n.x();
  double dy = n.y();
  double dz = n.z();
  double d2_x = TMath::Power(dx / 1., 2);
  double d2_y = TMath::Power(dy / 1., 2);
  double d2_z = TMath::Power(dz / 1., 2);
  double d2 = d2_x + d2_y + d2_z;

  return d2;
}

//MyClass::SumDistance2_angles(int &, double *, double & sum, double * par, int )
void SumDistance2_angles(int&, double*, double& sum, double* par, int)
{
  TGraph2DErrors* gr = dynamic_cast<TGraph2DErrors*>((TVirtualFitter::GetFitter())->GetObjectFit());
  assert(gr != 0);

  double* xp = gr->GetX();
  double* yp = gr->GetY();
  double* zp = gr->GetZ();
  double* ep = gr->GetEZ();
  int npoints = gr->GetN();

  sum = 0;

  for (int i = 0; i < npoints; i++) {
    double w = 1.;
    if (ep[i] > 0.)w = ep[i];
    double d = w * distance2_angles(xp[i], yp[i], zp[i], par);
    sum += d;
  }

}
//read tube centers, impulse response, and garfield drift data filename from MICROTPC.xml
void TrackFitterModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"MICROTPC\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    TPCCenter.push_back(TVector3(activeParams.getLength("TPCpos_x"), activeParams.getLength("TPCpos_y"),
                                 activeParams.getLength("TPCpos_z")));
    nTPC++;
  }

  m_GEMGain1 = content.getDouble("GEMGain1");
  m_GEMGain2 = content.getDouble("GEMGain2");
  m_GEMGainRMS1 = content.getDouble("GEMGainRMS1");
  m_GEMGainRMS2 = content.getDouble("GEMGainRMS2");
  m_ScaleGain1 = content.getDouble("ScaleGain1");
  m_ScaleGain2 = content.getDouble("ScaleGain2");
  m_GEMpitch = content.getDouble("GEMpitch");
  m_PixelThreshold = content.getInt("PixelThreshold");
  m_PixelThresholdRMS = content.getInt("PixelThresholdRMS");
  m_PixelTimeBinNb = content.getInt("PixelTimeBinNb");
  m_PixelTimeBin = content.getDouble("PixelTimeBin");
  m_ChipColumnNb = content.getInt("ChipColumnNb");
  m_ChipRowNb = content.getInt("ChipRowNb");
  m_ChipColumnX = content.getDouble("ChipColumnX");
  m_ChipRowY = content.getDouble("ChipRowY");
  m_TOTA1 = content.getDouble("TOTA1");
  m_TOTB1 = content.getDouble("TOTB1");
  m_TOTC1 = content.getDouble("TOTC1");
  m_TOTQ1 = content.getDouble("TOTQ1");
  m_TOTA2 = content.getDouble("TOTA2");
  m_TOTB2 = content.getDouble("TOTB2");
  m_TOTC2 = content.getDouble("TOTC2");
  m_TOTQ2 = content.getDouble("TOTQ2");
  m_z_DG = content.getDouble("z_DG");
  m_z_TG = content.getDouble("z_TG");
  m_z_CG = content.getDouble("z_CG");
  m_Dl_DG = content.getDouble("Dl_DG");
  m_Dl_TG = content.getDouble("Dl_TG");
  m_Dl_CG = content.getDouble("Dl_CG");
  m_Dt_DG = content.getDouble("Dt_DG");
  m_Dt_TG = content.getDouble("Dt_TG");
  m_Dt_CG = content.getDouble("Dt_CG");
  m_v_DG = content.getDouble("v_DG");
  m_v_TG = content.getDouble("v_TG");
  m_v_CG = content.getDouble("v_CG");
  m_Workfct = content.getDouble("Workfct");
  m_Fanofac = content.getDouble("Fanofac");
  m_GasAbs = content.getDouble("GasAbs");

  B2INFO("TrackFitter: Aquired tpc locations and gas parameters");
  B2INFO("              from MICROTPC.xml. There are " << nTPC << " TPCs implemented");

}

void TrackFitterModule::endRun()
{
}

void TrackFitterModule::terminate()
{
}


