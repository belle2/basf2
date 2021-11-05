/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisTrackingER.h>

#include <TROOT.h>
#include <TString.h>

#include <TMath.h>
#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisTrackingER)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisTrackingERModule::DQMHistAnalysisTrackingERModule()
  : DQMHistAnalysisModule()
{

  setDescription("DQM Analysis Module of the Tracking ER Plots.");

  addParam("onTimeHalfWidth", m_onTimeHalfWidth, "a cluster is on time if within ± onTimeHalfWidth [ns]", float(m_onTimeHalfWidth));

}

void DQMHistAnalysisTrackingERModule::initialize()
{

  gROOT->cd();

  // add MonitoringObject
  m_monObj = getMonitoringObject("trackingER");

}


void DQMHistAnalysisTrackingERModule::event()
{

  //compute fraction of tracks with no PXD hits

  TH1* hNoPXDHits = findHist("TrackingERDQM/NoOfHitsInTrack_PXD");
  if (hNoPXDHits != nullptr) {

    int nTracks = hNoPXDHits->GetEntries();
    int nTracksNoPXD = hNoPXDHits->GetBinContent(1);

    if (nTracks > 0)
      m_monObj->setVariable("tracksNoPXDHit", (double)nTracksNoPXD / nTracks);
  }

  //fraction of offtime SVD hits
  //considering L3V and L456V clusters (separately)

  TH1* hSVDL3VTime = findHist("SVDClsTrk/SVDTRK_ClusterTimeV3");
  if (hSVDL3VTime != nullptr) {
    int all = hSVDL3VTime->GetEntries();
    if (all > 0) {
      int bin_min = hSVDL3VTime->GetXaxis()->FindBin(-m_onTimeHalfWidth);
      int bin_max = hSVDL3VTime->GetXaxis()->FindBin(+m_onTimeHalfWidth);
      int offtime = all - hSVDL3VTime->Integral(bin_min, bin_max);
      double offtimeL3Hits = (double)offtime / all;
      m_monObj->setVariable("offtimeL3Hits", offtimeL3Hits);
    }
  }

  TH1* hSVDL456VTime = findHist("SVDClsTrk/SVDTRK_ClusterTimeV456");
  if (hSVDL456VTime != nullptr) {
    int all = hSVDL456VTime->GetEntries();
    if (all > 0) {
      int bin_min = hSVDL456VTime->GetXaxis()->FindBin(-m_onTimeHalfWidth);
      int bin_max = hSVDL456VTime->GetXaxis()->FindBin(+m_onTimeHalfWidth);
      int offtime = all - hSVDL456VTime->Integral(bin_min, bin_max);
      double offtimeL456Hits = (double)offtime / all;
      m_monObj->setVariable("offtimeL456Hits", offtimeL456Hits);
    }
  }
}
