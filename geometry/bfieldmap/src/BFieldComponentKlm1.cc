/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponentKlm1.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>

#include <cmath>

using namespace std;
using namespace Belle2;
namespace io = boost::iostreams;

void BFieldComponentKlm1::initialize()
{
  if (m_mapFilename.empty()) {
    B2ERROR("The filename for the magnetic field Component is empty !");
    return;
  }

  string fullPath = FileSystem::findFile("/data/" + m_mapFilename);

  if (!FileSystem::fileExists(fullPath)) {
    B2ERROR("The magnetic field map file '" << m_mapFilename << "' could not be found !");
    return;
  }

  //Load the map file
  io::filtering_istream fieldMapFile;
  fieldMapFile.push(io::file_source(fullPath));

  //read the data from the file
  B2DEBUG(10, "Loading the magnetic field from file '" << m_mapFilename << "' in to the memory...");

  char dummy[100];

  //store parameters for Barrel
  fieldMapFile.getline(dummy, sizeof(dummy));
  for (int layer = 0; layer < m_nBarrelLayers; ++layer) {
    int l;
    fieldMapFile >> l >> m_barrelZBreakpoint[layer] >> m_barrelRBreakpoint[layer]
                 >> m_barrelFieldZSlope1[layer]
                 >> m_barrelFieldZIntercept1[layer]
                 >> m_barrelFieldZSlope2[layer]
                 >> m_barrelFieldZIntercept2[layer]
                 >> m_barrelFieldRSlope1[layer]
                 >> m_barrelFieldRIntercept1[layer]
                 >> m_barrelFieldRSlope2[layer]
                 >> m_barrelFieldRIntercept2[layer];

    if (layer != l)
      B2ERROR("Barrel Parameter file is something wrong ! " << layer << " " << l);
  }

  //store parameters for Endcap
  fieldMapFile.getline(dummy, sizeof(dummy));
  fieldMapFile.getline(dummy, sizeof(dummy));
  for (int GapIron = 0; GapIron < 2; GapIron++) {
    for (int layer = 0; layer < m_nEndcapLayers + 1; layer++) {
      for (int j = 0; j < 5; j++) {
        int g, l, jj;
        fieldMapFile >> g >> l >> jj
                     >> m_endcapZBreakpoint[GapIron][layer][j] >> m_endcapRBreakpoint[GapIron][layer][j]
                     >> m_endcapFieldZSlope[GapIron][layer][j]
                     >> m_endcapFieldZIntercept[GapIron][layer][j]
                     >> m_endcapFieldRSlope[GapIron][layer][j]
                     >> m_endcapFieldRIntercept[GapIron][layer][j];
        if (GapIron != g || layer != l || jj != j)
          B2ERROR("Endcap Parameter file is something wrong ! " << g << " " << l << " " << jj);
      }
    }
  }

  B2DEBUG(10, "... loaded ");
}


B2Vector3D BFieldComponentKlm1::calculate(const B2Vector3D& point) const
{
  //Get the r and z Component
  double r = point.Perp();
  double x = point.X();
  double y = point.Y();
  double z = point.Z() - m_mapOffset;
  double absZ =  std::abs(z);

  B2Vector3D bField(0., 0., 0.);

  // Barrel
  if (absZ < m_barrelZMax) {

    // This code assumes that we are in the barrel KLM.  Use the field map
    // inside an iron plate; use zero in the gap between the iron plates.

    double cosphi = std::abs(x) / r;
    double d = ((cosphi < m_cospi8) ? ((cosphi < m_cos3pi8) ? std::abs(y)
                                       : (std::abs(x) + std::abs(y)) * m_cospi4)
                : std::abs(x)) - m_barrelRMin - m_barrelGapHeightLayer0;

    if (d >= 0.0) {
      int layer = static_cast<int>(floor(d / m_dLayer));

      // make sure layer is in a valid range
      if (layer < 0 || layer > m_nBarrelLayers) return bField;
      if (layer == m_nBarrelLayers) {
        layer = m_nBarrelLayers - 1;
        d = layer * m_dLayer; // for extra-thick outermost iron plate
      }
      if ((d - layer * m_dLayer) < m_barrelIronThickness) {
        double br = ((absZ > m_barrelRBreakpoint[layer]) ?
                     m_barrelFieldRIntercept2[layer] + m_barrelFieldRSlope2[layer] * absZ :
                     m_barrelFieldRIntercept1[layer] + m_barrelFieldRSlope1[layer] * absZ);
        if (z < 0.0) { br = -br; }
        bField.SetXYZ(br * x / r, br * y / r,
                      ((absZ > m_barrelZBreakpoint[layer]) ?
                       m_barrelFieldZIntercept2[layer] + m_barrelFieldZSlope2[layer] * absZ :
                       m_barrelFieldZIntercept1[layer] + m_barrelFieldZSlope1[layer] * absZ));
      }
    }
  } else if (absZ > m_endcapZMin) {
    //Endcap
    if (r <= m_endcapRMin) return bField;

    // This code assumes that we are in the endcap KLM.
    // The field in an inter-plate gap in the endcap is not zero.

    double dz = absZ - (m_endcapZMin - m_endcapGapHeight);
    int layer = static_cast<int>(floor(dz / m_dLayer));
    // make sure layer is in a valid range
    if (layer < 0 || layer >= m_nEndcapLayers) return bField;

    int GapIron = 0;      // in gap
    if ((dz - m_dLayer * layer) > m_endcapGapHeight) GapIron = 1;      // in Iron plate?
    for (int j = 0; j < 5; j++) {
      if (r < m_endcapRBreakpoint[GapIron][layer][j]) {
        double br = m_endcapFieldRIntercept[GapIron][layer][j] + m_endcapFieldRSlope[GapIron][layer][j] * r;
        if (z < 0.0) { br = -br; }
        bField.SetX(br * x / r);
        bField.SetY(br * y / r);
        break;
      }
    }
    for (int j = 0; j < 5; j++) {
      if (r < m_endcapZBreakpoint[GapIron][layer][j]) {
        bField.SetZ(m_endcapFieldZIntercept[GapIron][layer][j] + m_endcapFieldZSlope[GapIron][layer][j] * r);
        break;
      }
    }
  }

  return bField;
}


void BFieldComponentKlm1::terminate()
{
}
