/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMLayerArea.h>

/* Framework headers. */
#include <framework/gearbox/GearDir.h>

/* C++ headers. */
#include <string>


using namespace Belle2;

KLMLayerArea::KLMLayerArea() {}
KLMLayerArea::~KLMLayerArea() {}

void KLMLayerArea::calculateLayerAreas()
{
  const std::string component = "KLM";
  GearDir bklmDir("/Detector/DetectorComponent[@name='" + component + "']/Content");
  GearDir eklmDir("/Detector/DetectorComponent[@name='" + component + "']/Content/EKLM");

  Belle2::BKLMGeometryPar bklmGeometry(bklmDir);
  // EKLMGeometry doesn't have a GearDir constructor, so we read directly from GearDir

  // Flattened indexing: section 0 (0-119) (backward), section 1 (120-239) (forward)
  int nSections = 2;
  int nSectors = bklmGeometry.getNSector();    // 8
  int nLayers = bklmGeometry.getNLayer();     // 15

  for (int section = 0; section < nSections; ++section) {
    for (int sector = 0; sector < nSectors; ++sector) {
      for (int layer = 0; layer < nLayers; ++layer) {
        double area = 0.0;
        if (!bklmGeometry.hasRPCs(layer + 1)) {
          // Scintillator layers (1-2)
          int nPhiScints = bklmGeometry.getNPhiScints(layer + 1);
          int nZScints = (section == 0 && sector == 2) ? bklmGeometry.getNZScintsChimney() : bklmGeometry.getNZScints();
          double scintWidth = bklmGeometry.getScintWidth();
          area = nPhiScints * scintWidth * nZScints * scintWidth;
        } else {
          // RPC layers (3-15)
          int nPhiStrips = bklmGeometry.getNPhiStrips(layer + 1);
          double phiStripWidth = bklmGeometry.getPhiStripWidth(layer + 1);
          double zStripWidth = bklmGeometry.getZStripWidth(layer + 1);
          int nZStrips = (section == 0 && sector == 2) ? bklmGeometry.getNZStripsChimney() : bklmGeometry.getNZStrips();
          area = nPhiStrips * phiStripWidth * nZStrips * zStripWidth;
        }
        // Calculate flattened index: section*120 + sector*15 + layer
        int flattenedIndex = section * 120 + sector * 15 + layer;
        m_BKLMLayerAreas[flattenedIndex] = area;
      }
    }
  }

  // EKLM: all layers have the same area per sector
  int nStrips = eklmDir.getInt("NStrips");
  double stripWidth = eklmDir.getDouble("Strip/Width") / 10.0; // Convert mm to cm
  // Calculate total area per sector by summing strip areas
  m_EKLMLayerArea = 0.0;
  for (int stripId = 1; stripId <= nStrips; ++stripId) {
    std::string stripPath = "Strip/Strip[@id='" + std::to_string(stripId) + "']/Length";
    double stripLength = eklmDir.getDouble(stripPath) / 10.0; // Convert mm to cm
    m_EKLMLayerArea += stripLength * stripWidth;
  }
}

double KLMLayerArea::getBKLMLayerArea(int layer) const
{
  int layer0 = layer - 1; // Convert to 0-based (0-239)

  if (layer0 < 0 || layer0 >= 240) {
    return 0.0;
  }

  // Direct access to flat array
  return m_BKLMLayerAreas[layer0];
}

double KLMLayerArea::getEKLMLayerArea() const
{
  // EKLM: all layers have the same area
  return m_EKLMLayerArea;
}