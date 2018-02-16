/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/dbobjects/PXDMaskedPixelPar.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace std;


void PXDMaskedPixelPar::maskSinglePixel(VxdID id, unsigned int uid, unsigned int vid)
{
  auto mapIterSingles = m_MapSingles.find(id.getID());
  if (mapIterSingles != m_MapSingles.end()) {
    // Already some masked single pixels on sensor
    auto singles = mapIterSingles->second;
    auto vCells = VXD::GeoCache::getInstance().get(id).getVCells();
    unsigned int pixelID = uid * vCells + vid;
    // Only add pixel, if it is not already in
    if (singles.find(pixelID) == singles.end())
      singles.insert(pixelID);
  } else {
    // Create an empty set of masked single pixels
    PXDMaskedPixelPar::MaskedSinglePixelsSet singles;
    // We store pixels by unique id in hash table
    auto vCells = VXD::GeoCache::getInstance().get(id).getVCells();
    unsigned int pixelID = uid * vCells + vid;
    // pixelID will be used to generate hash in unordered_set for quick access
    singles.insert(pixelID);
    m_MapSingles[id.getID()] = singles;
  }
}

bool PXDMaskedPixelPar::pixelOK(VxdID id, unsigned int uid, unsigned int vid) const
{
  auto mapIterSingles = m_MapSingles.find(id.getID());
  if (mapIterSingles != m_MapSingles.end()) {
    // Found some masked single pixels on sensor
    auto singles = mapIterSingles->second;
    auto vCells = VXD::GeoCache::getInstance().get(id).getVCells();

    // Look if this is a single masked pixel
    if (singles.find(uid * vCells + vid) != singles.end())
      return false;
  }
  // Pixel not found in the mask => pixel OK
  return true;
}

