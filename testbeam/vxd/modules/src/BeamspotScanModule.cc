/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/vxd/modules/BeamspotScanModule.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <vxd/dataobjects/VxdID.h>

#include <TVector3.h>
#include <TH1F.h>
#include <TAxis.h>
#include <TMath.h>
#include <fstream>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BeamspotScan)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BeamspotScanModule::BeamspotScanModule() : HistoModule(),
  m_hits(0),
  m_sensorID("7.2.1"), m_dataFileName("beamscan.dat"), m_fieldFileName("field.txt"),
  m_nBinX(6), m_nBinY(10), m_minX(-0.6), m_minY(-1.0), m_maxX(0.6), m_maxY(1.0)
{
  // Set module properties
  setDescription("Scans beam parameters and produces data file for particle gun array");

  // Parameter definitions
  addParam("sensorID", m_sensorID, "VxdID of PXD sensor to be scanned", m_sensorID);
  addParam("dataFileName", m_dataFileName, "Name of output file for particle gun array", m_dataFileName);
  addParam("fieldFileName", m_fieldFileName, "Name of output file for vector field drawing", m_fieldFileName);

  addParam("nBinX", m_nBinX, "Histograms: number of bins in x", m_nBinX);
  addParam("nBinY", m_nBinY, "Histograms: number of bins in y", m_nBinY);
  addParam("minX", m_minX, "Histograms: minimum value for x", m_minX);
  addParam("minY", m_minY, "Histograms: minimum value for y", m_minY);
  addParam("maxX", m_maxX, "Histograms: maximum value for x", m_maxX);
  addParam("maxY", m_maxY, "Histograms: maximum value for y", m_maxY);

}

void BeamspotScanModule::initialize()
{
  StoreArray<PXDTrueHit> PXDTrueHit;  PXDTrueHit.isRequired();
  RbTupleManager::Instance().register_module(this);
}

void BeamspotScanModule::event()
{
  StoreArray<PXDTrueHit> hits("PXDTrueHits");
  for (int i = 0; i < hits.getEntries(); i++) {
    PXDTrueHit* hit = hits[i];
    if (hit->getSensorID() != VxdID(m_sensorID))
      continue;

    VXD::GeoCache& cache = VXD::GeoCache::getInstance();
    TVector3 mom = cache.getSensorInfo(hit->getSensorID()).vectorToGlobal(hit->getEntryMomentum());
    TVector3 pos = cache.getSensorInfo(hit->getSensorID()).pointToGlobal(TVector3(hit->getU(), hit->getV(), 0.));
    double z = pos[2];
    double y = pos[1];

    m_prof[0]->Fill(z, y, mom[0]);
    m_prof[1]->Fill(z, y, mom[1]);
    m_prof[2]->Fill(z, y, mom[2]);
    double px = mom[0];
    double py = mom[1];
    double pz = mom[2];
    double phi = atan2(py, px) * TMath::RadToDeg();
    if (phi < 0.)
      phi += 360.;

    double pxy = sqrt(px * px + py * py);
    double theta = atan2(pxy, pz) * TMath::RadToDeg();
    if (theta < 0.)
      theta += 360.;

    m_phi->Fill(z, y, phi);
    m_theta->Fill(z, y, theta);
    m_mom->Fill(z, y, sqrt(pxy * pxy + pz * pz));
    m_hitmap->Fill(z, y);
    m_hits++;
  }
}

void BeamspotScanModule::endRun()
{
  if (m_hits == 0) return;
  double nHitsInHisto = m_hitmap->Integral();
  if (nHitsInHisto == 0.) return;
  m_hitmap->Scale(100. / nHitsInHisto);

  ofstream file(m_dataFileName);
  ofstream field(m_fieldFileName);

  // File for particle gun array
  // ===========================
  // Comments & settings for data file
  file << "@ N_BIN_X X_MIN X_MAX N_BIN_Y Y_MIN Y_MAX" << std::endl;
  file << m_nBinX << " " << m_minX << " " << m_maxX << " " << m_nBinY << " " << m_minY << " " << m_maxY << std::endl;
  file << "@ BIN_X BIN_Y DENSITY MOM ERR_MOM PHI ERR_PHI THETA ERR_THETA" << std::endl;

  for (int x = 1; x <= m_nBinX; x++) {
    for (int y = 1; y <= m_nBinY; y++) {
      file << x << " " << y << " "
           << m_hitmap->GetBinContent(x, y) / nHitsInHisto << " "
           << m_mom->GetBinContent(x, y) << " " << m_mom->GetBinError(x, y) << " "
           << m_phi->GetBinContent(x, y) << " " << m_phi->GetBinError(x, y) << " "
           << m_theta->GetBinContent(x, y) << " " << m_theta->GetBinError(x, y) << std::endl;
    }
  }

  // File for vector field drawing
  // =============================
  // This file is supposed to have no comment lines!
  double binSizeX = (m_maxX - m_minX) / double(m_nBinX);
  double binSizeY = (m_maxY - m_minY) / double(m_nBinY);
  // Settings line
  // X_MIN Y_MIN X_MAX Y_MAX
  field <<  m_minX << " " << m_minY << " "  << m_maxX << " " << m_maxY << std::endl;

  for (int x = 1; x <= m_nBinX; x++) {
    for (int y = 1; y <= m_nBinY; y++) {
      // data line
      // z y px px pz errpx errpy errpz
      field << ((double(x) - 0.5)*binSizeX + m_minX) << " " << ((double(y) - 0.5)*binSizeY + m_minY) << " "
            << m_prof[0]->GetBinContent(x, y) << " "
            << m_prof[1]->GetBinContent(x, y) << " "
            << m_prof[2]->GetBinContent(x, y) << " "
            << m_prof[0]->GetBinError(x, y) << " "
            << m_prof[1]->GetBinError(x, y) << " "
            << m_prof[2]->GetBinError(x, y) << std::endl;
    }
  }

  for (int x = 1; x <= m_nBinX; x++) {
    for (int y = 1; y <= m_nBinY; y++) {
      m_rmsY->Fill(((double(x) - 0.5)*binSizeX + m_minX), ((double(y) - 0.5)*binSizeY + m_minY), m_prof[1]->GetBinError(x,
                   y) / fabs(m_prof[0]->GetBinContent(x, y)));
      m_rmsZ->Fill(((double(x) - 0.5)*binSizeX + m_minX), ((double(y) - 0.5)*binSizeY + m_minY), m_prof[2]->GetBinError(x,
                   y) / fabs(m_prof[0]->GetBinContent(x, y)));

    }
  }

}

void BeamspotScanModule::defineHisto()
{
  m_rmsY = new TProfile2D("rmsY", "Average Divergence of Beam Slopes Y [rad]", m_nBinX, m_minX, m_maxX, m_nBinY, m_minY, m_maxY);
  m_rmsZ = new TProfile2D("rmsZ", "Average Divergence of Beam Slopes Z [rad]", m_nBinX, m_minX, m_maxX, m_nBinY, m_minY, m_maxY);

  m_prof[0] = new TProfile2D("scanX", "Average momentum x [GeV]", m_nBinX, m_minX, m_maxX, m_nBinY, m_minY, m_maxY);
  m_prof[1] = new TProfile2D("scanY", "Average momentum y [GeV]", m_nBinX, m_minX, m_maxX, m_nBinY, m_minY, m_maxY);
  m_prof[2] = new TProfile2D("scanZ", "Average momentum z [GeV]", m_nBinX, m_minX, m_maxX, m_nBinY, m_minY, m_maxY);
  for (auto& h : m_prof) {
    h->Sumw2();
    h->Approximate();
    h->SetErrorOption("s");
    h->GetXaxis()->SetTitle("Z [cm]");
    h->GetYaxis()->SetTitle("Y [cm]");
  }
  m_phi = new TProfile2D("scanPhi", "Average phi angle [deg]", m_nBinX, m_minX, m_maxX, m_nBinY, m_minY, m_maxY);
  m_phi->GetXaxis()->SetTitle("Z [cm]");
  m_phi->GetYaxis()->SetTitle("Y [cm]");
  m_phi->Sumw2();
  m_phi->Approximate();
  m_phi->SetErrorOption("s");
  m_theta = new TProfile2D("scanTheta", "Average theta angle [deg]", m_nBinX, m_minX, m_maxX, m_nBinY, m_minY, m_maxY);
  m_theta->GetXaxis()->SetTitle("Z [cm]");
  m_theta->GetYaxis()->SetTitle("Y [cm]");
  m_theta->Sumw2();
  m_theta->Approximate();
  m_theta->SetErrorOption("s");
  m_mom = new TProfile2D("scanMom", "Average momentum magnitude [GeV]", m_nBinX, m_minX, m_maxX, m_nBinY, m_minY, m_maxY);
  m_mom->GetXaxis()->SetTitle("Z [cm]");
  m_mom->GetYaxis()->SetTitle("Y [cm]");
  m_mom->Sumw2();
  m_mom->Approximate();
  m_mom->SetErrorOption("s");
  m_hitmap = new TH2F("hitmap", "Beam intensity [%]", m_nBinX, m_minX, m_maxX, m_nBinY, m_minY, m_maxY);
  m_hitmap->GetXaxis()->SetTitle("Z [cm]");
  m_hitmap->GetYaxis()->SetTitle("Y [cm]");
}
