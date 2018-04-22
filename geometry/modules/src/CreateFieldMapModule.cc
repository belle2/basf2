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
#include <framework/geometry/BFieldManager.h>
#include <boost/algorithm/string.hpp>
#include <framework/utilities/Utils.h>
#include <framework/gearbox/Unit.h>
#include <framework/geometry/B2Vector3.h>

#include <TFile.h>
#include <TH2D.h>
#include <TTree.h>

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
  addParam("type", m_type, "type of the fieldmap (xy, zx, zy, zr)", std::string("zx"));
  addParam("nU", m_nU, "number of steps along first coordinate", 1800);
  addParam("minU", m_minU, "minimum value for the first coordinate", -400.);
  addParam("maxU", m_maxU, "maximum value for the first coordinate", 500.);
  addParam("nV", m_nV, "number of steps along second coordinate", 1600);
  addParam("minV", m_minV, "minimum value for the second coordinate", -400.);
  addParam("maxV", m_maxV, "maximum value for the second coordinate", 400.);
  addParam("phi", m_phi, "phi angle for the scan in radians", 0.);
  addParam("wOffset", m_wOffset, "value of third coordinate", 0.);
  addParam("nPhi", m_nPhi, "number of phi steps for zr averaging", m_nPhi);
  addParam("saveAllPoints", m_createTree, "save all sampled points in a TTree, "
           "WARNING: output can be huge", false);
}

void CreateFieldMapModule::initialize()
{
  if (m_filename.empty()) {
    B2WARNING("CreateFieldMap: No Filename given, just sampling for fun");
  }
  boost::to_lower(m_type);
  boost::trim(m_type);
  if (m_type != "xy" && m_type != "zx" && m_type != "zy" && m_type != "zr") {
    B2ERROR("CreateFieldMap type '" << m_type << "' not valid, use one of 'xy', 'zx', 'zy' or 'zr'");
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
  if (m_type == "zr") {
    if (m_minV < 0) {
      B2ERROR("R values (minV, maxV) must be positive");
    }
    if (m_nPhi <= 0) {
      B2ERROR("Number of steps in phi must be positive");
    }
  }
}

void CreateFieldMapModule::beginRun()
{
  const bool save{!m_filename.empty()};
  TFile* outfile{nullptr};
  if (save) {
    //Create histograms
    outfile = new TFile(m_filename.c_str(), "RECREATE");
    outfile->cd();
  }

  //Determine type of scan
  EFieldTypes type = c_XY;
  if (m_type == "zx")
    type = c_ZX;
  else if (m_type == "zy")
    type = c_ZY;
  else if (m_type == "zr")
    type = c_ZR;

  //some values needed for output
  int lastPercent(-1);
  uint64_t nSteps = m_nU * m_nV;
  uint64_t curStep{0};
  double startTime{0};
  auto showProgress = [&]() {
    if (curStep == 0) startTime = Utils::getClock();
    const int64_t donePercent = 100 * ++curStep / nSteps;
    if (donePercent > lastPercent) {
      const double totalTime = Utils::getClock() - startTime;
      const double perStep = totalTime / curStep;
      if (donePercent == 100) {
        B2INFO(boost::format("BField %s Scan: %d samples, %.3f us per sample, total: %.2f seconds")
               % m_type % curStep
               % (perStep / Unit::us) % (totalTime / Unit::s));
      } else {
        B2INFO(boost::format("BField %s Scan: %3d%%, %.3f us per sample")
               % m_type % donePercent
               % (perStep / Unit::us));
      }
      lastPercent = donePercent;
    }
  };

  struct { float x{0}, y{0}, z{0}, bx{0}, by{0}, bz{0}; } field_point;
  TTree* all_values{nullptr};
  if (save && m_createTree) {
    all_values = new TTree("bfield_values", "All B field values");
    all_values->Branch("x", &field_point.x, "x/F");
    all_values->Branch("y", &field_point.y, "y/F");
    all_values->Branch("z", &field_point.z, "z/F");
    all_values->Branch("bx", &field_point.bx, "bx/F");
    all_values->Branch("by", &field_point.by, "by/F");
    all_values->Branch("bz", &field_point.bz, "bz/F");
  }
  auto fillTree = [&](const B2Vector3D & p, const B2Vector3D & b) {
    if (!all_values) return;
    field_point.x = p.X();
    field_point.y = p.Y();
    field_point.z = p.Z();
    field_point.bx = b.X();
    field_point.by = b.Y();
    field_point.bz = b.Z();
    all_values->Fill();
  };

  if (type == c_ZR) {
    nSteps *= m_nPhi;
    TH2D* h_b = new TH2D("B", "$B$ average;$z$/cm;$r$/cm", m_nU, m_minU, m_maxU, m_nV, m_minV, m_maxV);
    TH2D* h_br = new TH2D("Br", "$B_r$ average;$z$/cm;$r$/cm", m_nU, m_minU, m_maxU, m_nV, m_minV, m_maxV);
    TH2D* h_bz = new TH2D("Bz", "$B_z$ average;$z$/cm;$r$/cm", m_nU, m_minU, m_maxU, m_nV, m_minV, m_maxV);
    for (int iU = 0; iU < m_nU; ++iU) {
      for (int iV = 0; iV < m_nV; ++iV) {
        //find value for first and second coordinate
        const double u = h_b->GetXaxis()->GetBinCenter(iU + 1);
        const double v = h_b->GetYaxis()->GetBinCenter(iV + 1);
        //Determine global coordinates
        for (int iPhi = 0; iPhi < m_nPhi; ++iPhi) {
          B2Vector3D pos(v, 0, u);
          pos.RotateZ(2 * M_PI * iPhi / m_nPhi);
          //Obtain magnetic field
          B2Vector3D bfield = BFieldManager::getFieldInTesla(pos);
          //And fill histograms
          if (save) {
            h_br->Fill(u, v, bfield.Perp());
            h_bz->Fill(u, v, bfield.Z());
            h_b->Fill(u, v, bfield.Mag());
            fillTree(pos, bfield);
          }
          showProgress();
        }
      }
    }
    //Write histograms and close file.
    if (save) {
      for (TH2D* h : {h_br, h_bz, h_b}) {
        h->Scale(1. / m_nPhi);
        h->Write();
      }
    }
  } else {
    const std::string nu = m_type.substr(0, 1);
    const std::string nv = m_type.substr(1, 1);
    TH2D* h_b = new TH2D("B", ("$B$;$" + nu + "$/cm;$" + nv + "$/cm").c_str(), m_nU, m_minU, m_maxU, m_nV, m_minV, m_maxV);
    TH2D* h_bx = new TH2D("Bx", ("$B_x$;$" + nu + "$/cm;$" + nv + "$/cm").c_str(), m_nU, m_minU, m_maxU, m_nV, m_minV, m_maxV);
    TH2D* h_by = new TH2D("By", ("$B_y$;$" + nu + "$/cm;$" + nv + "$/cm").c_str(), m_nU, m_minU, m_maxU, m_nV, m_minV, m_maxV);
    TH2D* h_bz = new TH2D("Bz", ("$B_z$;$" + nu + "$/cm;$" + nv + "$/cm").c_str(), m_nU, m_minU, m_maxU, m_nV, m_minV, m_maxV);

    //Loop over all bins
    for (int iU = 0; iU < m_nU; ++iU) {
      for (int iV = 0; iV < m_nV; ++iV) {
        B2Vector3D pos(0, 0, 0);
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
          default:
            break;
        }
        pos.RotateZ(m_phi);
        //Obtain magnetic field
        B2Vector3D bfield = BFieldManager::getFieldInTesla(pos);
        //And fill histograms
        if (save) {
          h_bx->Fill(u, v, bfield.X());
          h_by->Fill(u, v, bfield.Y());
          h_bz->Fill(u, v, bfield.Z());
          h_b->Fill(u, v, bfield.Mag());
          fillTree(pos, bfield);
        }
        showProgress();
      }
    }
    //Write histograms.
    if (save) {
      for (TH2D* h : {h_bx, h_by, h_bz, h_b}) {
        h->Write();
      }
    }
  }
  if (save) {
    if (all_values) all_values->Write();
    outfile->Close();
    delete outfile;
  }
  //histograms seem to be deleted when file is closed
}
