/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/dbobjects/ARICHAeroTilesInfo.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;

float ARICHAeroTilesInfo::getRefractiveIndex(int slot, int layer) const
{
  if (slot > 124 || slot < 1) { B2WARNING("ARICHAeroTilesInfo::getTransmissionLength: Slot number " << slot << " not valid!"); return 0.;}
  if (layer > 1 || layer < 0) { B2WARNING("ARICHAeroTilesInfo::getTransmissionLength: Layer number " << layer << " not valid!"); return 0.;}

  std::vector<int> slotID{slot, layer};
  std::vector<float> opticalProperties = m_aeroPos2optical.find(slotID)->second;
  return opticalProperties[0];
}

float ARICHAeroTilesInfo::getTransmissionLength(int slot, int layer) const
{
  if (slot > 124 || slot < 1) { B2WARNING("ARICHAeroTilesInfo::getTransmissionLength: Slot number " << slot << " not valid!"); return 0.;}
  if (layer > 1 || layer < 0) { B2WARNING("ARICHAeroTilesInfo::getTransmissionLength: Layer number " << layer << " not valid!"); return 0.;}

  std::vector<int> slotID{slot, layer};
  std::vector<float> opticalProperties = m_aeroPos2optical.find(slotID)->second;
  return opticalProperties[1];
}

void ARICHAeroTilesInfo::addMapping(int slot, int layer, float refractiveIndex, float transmissionLength)
{

  if (slot > 124 || slot < 1) { B2WARNING("ARICHAeroTilesInfo::getTransmissionLength: Slot number " << slot << " not valid!"); }
  if (layer > 1 || layer < 0) { B2WARNING("ARICHAeroTilesInfo::getTransmissionLength: Layer number " << layer << " not valid!"); }

  std::vector<int> slotID{slot, layer};
  std::vector<float> opticalProperties{refractiveIndex, transmissionLength};
  m_aeroPos2optical.insert(std::pair<std::vector<int>, std::vector<float>>(slotID, opticalProperties));

}

void ARICHAeroTilesInfo::print() const
{
  B2INFO("AEROGEL SLOT MAPPING with optical properties of tiles");
  for (int tile = 1; tile < N_TILES + 1; tile++) {
    B2INFO("Slot number: " << tile << "\n" << "       downstream tile: refIndex = " << getRefractiveIndex(tile,
           0) << "; transLength = " << getTransmissionLength(tile,
                                                             0) << "\n" << "       upstream tile: refIndex = " << getRefractiveIndex(tile,
                                                                 1) << "; transLength = " << getTransmissionLength(tile, 1));
  }
}
