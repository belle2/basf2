/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/modules/CreateFieldMapModule.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <boost/algorithm/string.hpp>
#include <framework/utilities/Utils.h>
#include <framework/gearbox/Unit.h>

#include <TFile.h>
#include <TH2D.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CreateFieldMap)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CreateFieldMapModule::CreateFieldMapModule() : Module()
{
  // Set module properties
  setDescription("Create Field maps of the Belle II magnetic field used in the simulation");

  // Parameter definitions
  addParam("filename", m_filename, "ROOT filename for the map", std::string("FieldMap.root"));
  addParam("type", m_type, "type of the fieldmap (xy, zx, zy)", std::string("zx"));
  addParam("nU", m_nU, "number of steps along first coordinate", 1800);
  addParam("minU", m_minU, "minimum value for the first coordinate", -400.);
  addParam("maxU", m_maxU, "maximum value for the first coordinate", 500.);
  addParam("nV", m_nV, "number of steps along second coordinate", 1600);
  addParam("minV", m_minV, "minimum value for the second coordinate", -400.);
  addParam("maxV", m_maxV, "maximum value for the second coordinate", 400.);
  addParam("phi", m_phi, "phi angle for the scan in radians", 0.);
  addParam("wOffset", m_wOffset, "value of third coordinate", 0.);
}

void CreateFieldMapModule::initialize()
{
  if (m_filename.empty()) {
    B2ERROR("No Filename given.");
  }
  boost::to_lower(m_type);
  boost::trim(m_type);
  if (m_type != "xy" && m_type != "zx" && m_type != "zy") {
    B2ERROR("CreateFieldMap type '" << m_type << "' not valid, use one of 'xy', 'zx' or 'zy'");
  }
  if (m_nU < 0 || m_nV < 0) {
    B2ERROR("Number of steps has to be positive");
  }
  if (m_maxU == m_minU) {
    B2ERROR("Range for first coordinate is zero");
  } else if (m_maxU < m_minU) {
    std::swap(m_maxU, m_minU);
  }
  if (m_maxV == m_minV) {
    B2ERROR("Range for second coordinate is zero");
  } else if (m_maxV < m_minV) {
    std::swap(m_maxV, m_minV);
  }
}

void CreateFieldMapModule::beginRun()
{
  //Create histograms
  TFile* outfile = new TFile(m_filename.c_str(), "RECREATE");
  outfile->cd();
  TH2D* h_bx = new TH2D("Bx", "Field strength along x", m_nU, m_minU, m_maxU, m_nV, m_minV, m_maxV);
  TH2D* h_by = new TH2D("By", "Field strength along y", m_nU, m_minU, m_maxU, m_nV, m_minV, m_maxV);
  TH2D* h_bz = new TH2D("Bz", "Field strength along z", m_nU, m_minU, m_maxU, m_nV, m_minV, m_maxV);
  TH2D* h_br = new TH2D("Br", "radial field strength", m_nU, m_minU, m_maxU, m_nV, m_minV, m_maxV);

  //Determine type of scan
  EFieldTypes type = c_XY;
  if (m_type == "zx")
    type = c_ZX;
  else if (m_type == "zy")
    type = c_ZY;

  //some values needed for output
  int lastPercent(-1);
  const int nSteps = m_nU * m_nV;
  const double start = Utils::getClock();

  //Loop over all bins
  for (int iU = 0; iU < m_nU; ++iU) {
    for (int iV = 0; iV < m_nV; ++iV) {
      TVector3 pos(0, 0, 0);
      //find value for first and second coordinate
      const double u = h_bx->GetXaxis()->GetBinCenter(iU + 1);
      const double v = h_bx->GetYaxis()->GetBinCenter(iV + 1);
      //Determine global coordinates
      switch (type) {
        case c_XY:
          pos.SetXYZ(u, v , m_wOffset);
          break;
        case c_ZX:
          pos.SetXYZ(v, m_wOffset, u);
          break;
        case c_ZY:
          pos.SetXYZ(m_wOffset, v, u);
          break;
      }
      pos.RotateZ(m_phi);
      //Obtain magnetic field
      TVector3 bfield = BFieldMap::Instance().getBField(pos);
      //And fill histograms
      h_bx->Fill(u, v, bfield.X());
      h_by->Fill(u, v, bfield.Y());
      h_bz->Fill(u, v, bfield.Z());
      h_br->Fill(u, v, bfield.Perp());

      //Show progress
      const int curStep = iU * iV + 1;
      const int donePercent = 100 * curStep / nSteps;
      if (donePercent > lastPercent) {
        const double perStep = (Utils::getClock() - start) / curStep;
        const double eta = perStep * (nSteps - curStep);
        B2INFO(boost::format("BField %s Scan: %3d%%, %.3f ms per sample, ETA: %.2f seconds")
               % m_type % donePercent
               % (perStep / Unit::ms) % (eta / Unit::s));
        lastPercent = donePercent;
      }
    }
  }
  //Write histograms and close file.
  h_bx->Write();
  h_by->Write();
  h_bz->Write();
  h_br->Write();
  outfile->Close();
  delete outfile;
  //histograms seem to be deleted when file is closed
}
