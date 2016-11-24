/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-20XX  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponent3d.h>
#include <geometry/bfieldmap/BFieldComponentBeamline.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <cmath>

#include <TMath.h>
#include <TFile.h>
#include <TH3.h>
#include <TH2.h>

using namespace std;
using namespace Belle2;
namespace io = boost::iostreams;

void BFieldComponent3d::initialize()
{

  // Input field map
  if (m_mapFilename.empty()) {
    B2ERROR("The filename for the 3d magnetic field component is empty !");
    return;
  }
  string fullPath = FileSystem::findFile("/data/" + m_mapFilename);
  if (!FileSystem::fileExists(fullPath)) {
    B2ERROR("The 3d magnetic field map file '" << m_mapFilename << "' could not be found !");
    return;
  }

  // Options to reduce 3D to 2D
  if (m_mapEnable != "rphiz" && m_mapEnable != "rphi" && m_mapEnable != "phiz" && m_mapEnable != "rz") {
    B2ERROR("BField3d:: enabled coordinates must be \"rphiz\", \"rphi\", \"phiz\" or \"rz\"");
    return;
  }

  // Excluded region
  m_exRegion = true;
  if ((m_exRegionR[0] == m_exRegionR[1]) || (m_exRegionZ[0] == m_exRegionZ[1])) m_exRegion = false;

  //convert back phi units to degrees for easier indexing
  m_gridPitch[1] /= Unit::deg;

  // Print initial input parameters
  B2INFO("BField3d:: initial input parameters");
  B2INFO(Form("   map filename:          %s",                   m_mapFilename.c_str()));
  B2INFO(Form("   map dimension:         %s",                   m_mapEnable.c_str()));
  if (m_interpolate) { B2INFO(Form("   map interpolation:     on")); }
  else               { B2INFO(Form("   map interpolation:     off")); }
  B2INFO(Form("   map r pitch & range:   %.2e [%.2e, %.2e] cm", m_gridPitch[0], m_mapRegionR[0], m_mapRegionR[1]));
  B2INFO(Form("   map phi pitch:         %.2e deg",             m_gridPitch[1]));
  B2INFO(Form("   map z pitch & range:   %.2e [%.2e, %.2e] cm", m_gridPitch[2], m_mapRegionZ[0], m_mapRegionZ[1]));
  if (m_exRegion) {
    B2INFO(Form("   map r excluded region: [%.2e, %.2e] cm",    m_exRegionR[0], m_exRegionR[1]));
    B2INFO(Form("   map z excluded region: [%.2e, %.2e] cm",    m_exRegionZ[0], m_exRegionZ[1]));
  }

  // Load B-field map file
  io::filtering_istream fieldMapFile;
  fieldMapFile.push(io::gzip_decompressor());
  fieldMapFile.push(io::file_source(fullPath));

  // B-field map is defined only for phi=0-180
  int phimapsize = m_mapSize[1];
  if (m_mirrorPhi) m_mapSize[1] = 2 * m_mapSize[1] - 1;

  // Store B-field map in TH3D -> convert (Br, Bphi, Bz) to (Bx, By, Bz)
  // N.B. ROOT assumes grid points defined at bin centres
  TH3D* m_mapBuffer3D[3];
  TString mapname = Form("BFieldMap_R%d-%d_Z%d-%d",
                         int(10 * m_mapRegionR[0]), int(10 * m_mapRegionR[1]), int(10 * m_mapRegionZ[0]), int(10 * m_mapRegionZ[1]));
  m_mapBuffer3D[0] = new TH3D(mapname + "_Bx", "; r [cm]; #phi [deg]; z [cm]",
                              m_mapSize[0], m_mapRegionR[0] - m_gridPitch[0] / 2., m_mapRegionR[1] + m_gridPitch[0] / 2,
                              m_mapSize[1], 0 - m_gridPitch[1] / 2., 360 + m_gridPitch[1] / 2.,
                              m_mapSize[2], m_mapRegionZ[0] - m_gridPitch[2] / 2., m_mapRegionZ[1] + m_gridPitch[2] / 2.);
  m_mapBuffer3D[1] = new TH3D(mapname + "_By", "; r [cm]; #phi [deg]; z [cm]",
                              m_mapSize[0], m_mapRegionR[0] - m_gridPitch[0] / 2., m_mapRegionR[1] + m_gridPitch[0] / 2,
                              m_mapSize[1], 0 - m_gridPitch[1] / 2., 360 + m_gridPitch[1] / 2.,
                              m_mapSize[2], m_mapRegionZ[0] - m_gridPitch[2] / 2., m_mapRegionZ[1] + m_gridPitch[2] / 2.);
  m_mapBuffer3D[2] = new TH3D(mapname + "_Bz", "; r [cm]; #phi [deg]; z [cm]",
                              m_mapSize[0], m_mapRegionR[0] - m_gridPitch[0] / 2., m_mapRegionR[1] + m_gridPitch[0] / 2,
                              m_mapSize[1], 0 - m_gridPitch[1] / 2., 360 + m_gridPitch[1] / 2.,
                              m_mapSize[2], m_mapRegionZ[0] - m_gridPitch[2] / 2., m_mapRegionZ[1] + m_gridPitch[2] / 2.);

  double r, z, phi, phi_rad, Br, Bz, Bphi, Bx, By;
  int binx, biny, binz;
  for (int k = m_mapSize[2] - 1; k >= 0; k--) { // z --> axis flipped (not important when using TH3)
    for (int i = 0; i < m_mapSize[0]; i++) { // r
      for (int j = 0;  j < phimapsize; j++) { // phi
        //r[m]     phi[deg]   z[m]   Br[T]   Bphi[T]   Bz[T]
        fieldMapFile >> r >> phi >> z >> Br >> Bphi >> Bz;
        // Convert units r[m]->r[cm], phi[deg]->phi[rad], z[m]->z[cm]
        r *= Unit::m;
        z *= Unit::m;
        //phi*=Unit::deg; // keep degrees for indexing
        phi_rad = phi * Unit::deg;

        // Possible shift of coordinate system
        z += m_mapOffset;

        // B-field calculation done in different coordiate orientation
        // flip z & y coordinate
        z *= -1.;
        Bz *= -1.;
        // No need to change phi coordinate because it is anyway mirrored
        Bphi *= -1.;

        // Introduce error on B field
        if ((m_errRegionR[0] != m_errRegionR[1]) && (r >= m_errRegionR[0]) && (r < m_errRegionR[1])) {
          Br   *= m_errB[0];
          Bphi *= m_errB[1];
          Bz   *= m_errB[2];
        }

        // Store the values
        binx = m_mapBuffer3D[0]->GetXaxis()->FindBin(r);
        biny = m_mapBuffer3D[0]->GetYaxis()->FindBin(phi);
        binz = m_mapBuffer3D[0]->GetZaxis()->FindBin(z);
        Bx   = Br * cos(phi_rad) - Bphi * sin(phi_rad);
        By   = Br * sin(phi_rad) + Bphi * cos(phi_rad);
        m_mapBuffer3D[0]->SetBinContent(binx, biny, binz, Bx);
        m_mapBuffer3D[1]->SetBinContent(binx, biny, binz, By);
        m_mapBuffer3D[2]->SetBinContent(binx, biny, binz, Bz);
        // Mirror (missing half of the input in phi)
        if (m_mirrorPhi) {
          phi = 360 - phi;
          biny = m_mapBuffer3D[0]->GetYaxis()->FindBin(phi);
          Bx   = Br * cos(phi_rad) + Bphi * sin(phi_rad);
          By   = Br * sin(phi_rad) - Bphi * cos(phi_rad);
          m_mapBuffer3D[0]->SetBinContent(binx, biny, binz, Br);
          m_mapBuffer3D[1]->SetBinContent(binx, biny, binz, -1.*Bphi);
          m_mapBuffer3D[2]->SetBinContent(binx, biny, binz, Bz);
        }
      }
    }
  }

  // Remove disabled dimension to make 2D
  // NOTE: disabling phi dimension inevitably removes Bphi component
  //       -> currently done automatically because of mirroring in x-z plane (but not exactly zero...),
  //          should be done manually if it is not mirrored
  if (m_mapEnable != "rphiz") {
    // Rebin in the disabled dimension and rescale
    if (m_mapEnable == "phiz") {
      for (int i = 0; i < 3; ++i) {
        m_mapBuffer3D[i]->RebinX(m_mapSize[0]);
        m_mapBuffer3D[i]->Scale(1. / double(m_mapSize[0]));
      }
      m_gridPitch[0] = m_mapBuffer3D[0]->GetXaxis()->GetBinWidth(1);
      m_mapSize[0] = m_mapBuffer3D[0]->GetNbinsX() + 1; // need extra bin for interpolation
    } else if (m_mapEnable == "rz") {
      for (int i = 0; i < 3; ++i) {
        m_mapBuffer3D[i]->RebinY(m_mapSize[1]);
        m_mapBuffer3D[i]->Scale(1. / double(m_mapSize[1]));
      }
      m_gridPitch[1] = m_mapBuffer3D[0]->GetYaxis()->GetBinWidth(1);
      m_mapSize[1] = m_mapBuffer3D[0]->GetNbinsY() + 1;
    } else if (m_mapEnable == "rphi") {
      for (int i = 0; i < 3; ++i) {
        m_mapBuffer3D[i]->RebinZ(m_mapSize[2]);
        m_mapBuffer3D[i]->Scale(1. / double(m_mapSize[2]));
      }
      m_gridPitch[2] = m_mapBuffer3D[0]->GetZaxis()->GetBinWidth(1);
      m_mapSize[2] = m_mapBuffer3D[0]->GetNbinsZ() + 1; // need extra bin for interpolation
    }
  }

  // Use vectors instead of TH3 to reduce CPU/memory(?) usage
  for (int imap = 0; imap < 3; ++imap) {
    m_mapBuffer[imap].resize(m_mapSize[0]);
    for (int ix = 0; ix < m_mapSize[0]; ++ix) {
      m_mapBuffer[imap][ix].resize(m_mapSize[1]);
      for (int iy = 0; iy < m_mapSize[1]; ++iy) {
        m_mapBuffer[imap][ix][iy].resize(m_mapSize[2]);
        for (int iz = 0; iz < m_mapSize[2]; ++iz) {
          int jx = (m_mapEnable != "phiz" || ix != m_mapSize[0] - 1) ? ix + 1 : ix;
          int jy = (m_mapEnable != "rz"   || iy != m_mapSize[1] - 1) ? iy + 1 : iy;
          int jz = (m_mapEnable != "rphi" || iz != m_mapSize[2] - 1) ? iz + 1 : iz;
          m_mapBuffer[imap][ix][iy][iz] = m_mapBuffer3D[imap]->GetBinContent(jx, jy, jz);
        }
      }
    }
  }
  for (int i = 0; i < 3; ++i) delete m_mapBuffer3D[i];

  B2INFO(Form("BField3d:: final map region & pitch: r [%.2e,%.2e] %.2e, phi %.2e, z [%.2e,%.2e] %.2e",
              m_mapRegionR[0], m_mapRegionR[1], m_gridPitch[0], m_gridPitch[1],
              m_mapRegionZ[0], m_mapRegionZ[1], m_gridPitch[2]));

  /*
  TFile *f = new TFile("FieldMap3D.root", "RECREATE");
  f->cd();
  m_mapBuffer3D[0]->Write("Br_3D");
  m_mapBuffer3D[1]->Write("Bphi_3D");
  m_mapBuffer3D[2]->Write("Bz_3D");
  f->Close();
  delete f;
  */
}


TVector3 BFieldComponent3d::calculate(const TVector3& point) const
{
  // If both '3d' and 'Beamline' components are defined in xml file,
  // '3d' component returns zero field where 'Beamline' component is defined.
  // If no 'Beamline' component is defined in xml file, the following function will never be called.
  if (BFieldComponentBeamline::isInRange(point)) {
    B2DEBUG(100, "'3d' magnetic field component returns zero value, because we use 'Beamline' magnetic field instead.");
    return TVector3(0.0, 0.0, 0.0);
  }

  // Get the r, phi and z component
  double r = point.Perp();
  double phi_rad = point.Phi();
  double phi = phi_rad * 180. / M_PI;
  if (phi < 0.) phi = 360. + phi;
  if (!(phi < 360.)) phi = 0.;
  double z = point.Z();

  // Check if the point lies inside the magnetic field boundaries
  if ((r < m_mapRegionR[0]) || (r >= m_mapRegionR[1]) ||
      (z < m_mapRegionZ[0]) || (z >= m_mapRegionZ[1])) {
    B2DEBUG(100, Form("BField3d:: point not in map range: %.2e [%.2e, %.2e] %.2e [%.2e, %.2e])\n",
                      r, m_mapRegionR[0], m_mapRegionR[1], z, m_mapRegionZ[0], m_mapRegionZ[1]));
    return TVector3(0.0, 0.0, 0.0);
  }

  // Check if the point lies in the exclude region
  if (m_exRegion &&
      (r >= m_exRegionR[0]) && (r < m_exRegionR[1]) &&
      (z >= m_exRegionZ[0]) && (z < m_exRegionZ[1])) {
    B2DEBUG(100, Form("BField3d:: point in the excluded region: %.2e [%.2e, %.2e] %.2e [%.2e, %.2e])\n",
                      r, m_exRegionR[0], m_exRegionR[1], z, m_exRegionZ[0], m_exRegionZ[1]));
    return TVector3(0.0, 0.0, 0.0);
  }

  // Calculate the lower index of the point in the grid
  int ir = static_cast<int>(floor((r - m_mapRegionR[0]) / m_gridPitch[0]));
  int iz = static_cast<int>(floor((z - m_mapRegionZ[0]) / m_gridPitch[2]));
  int iphi = static_cast<int>(floor(phi / m_gridPitch[1]));

  // Get B-field values from map
  double Bx(0.), By(0.), Bz(0.);
  if (m_interpolate) { // interpolate
    Bx   = interpolate(ir, iphi, iz, r, phi, z, m_mapBuffer[0]);
    By   = interpolate(ir, iphi, iz, r, phi, z, m_mapBuffer[1]);
    Bz   = interpolate(ir, iphi, iz, r, phi, z, m_mapBuffer[2]);
  } else {             // don't interpolate
    Bx   = m_mapBuffer[0][ir][iphi][iz];
    By   = m_mapBuffer[1][ir][iphi][iz];
    Bz   = m_mapBuffer[2][ir][iphi][iz];
  }

  return TVector3(Bx, By, Bz);

}


void BFieldComponent3d::terminate()
{
  B2DEBUG(10, "De-allocating the memory for the 3d magnetic field map ");
  // De-Allocate memory
  for (int i = 0; i < 3; ++i)
    std::vector< std::vector< std::vector<double> > >(m_mapBuffer[i]).swap(m_mapBuffer[i]);

}

double BFieldComponent3d::interpolate(int& ir, int& iphi, int& iz, double& r, double& phi, double& z,
                                      const std::vector< std::vector< std::vector<double> > >& bmap) const

{
  // Linear interpolation as implemented in ROOT TH3

  double xd = ((r - m_mapRegionR[0]) / m_gridPitch[0]) - ir;
  double yd = (phi / m_gridPitch[1]) - iphi;
  double zd = ((z - m_mapRegionZ[0]) / m_gridPitch[2]) - iz;

  double v[8] = { bmap[ir][iphi][iz],     bmap[ir][iphi][iz + 1],
                  bmap[ir][iphi + 1][iz],   bmap[ir][iphi + 1][iz + 1],
                  bmap[ir + 1][iphi][iz],   bmap[ir + 1][iphi][iz + 1],
                  bmap[ir + 1][iphi + 1][iz], bmap[ir + 1][iphi + 1][iz + 1]
                };

  double i1 = v[0] * (1 - zd) + v[1] * zd;
  double i2 = v[2] * (1 - zd) + v[3] * zd;
  double j1 = v[4] * (1 - zd) + v[5] * zd;
  double j2 = v[6] * (1 - zd) + v[7] * zd;

  double w1 = i1 * (1 - yd) + i2 * yd;
  double w2 = j1 * (1 - yd) + j2 * yd;
  double B  = w1 * (1 - xd) + w2 * xd;

  return B;
}
