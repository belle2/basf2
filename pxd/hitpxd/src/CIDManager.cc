/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/hitpxd/CIDManager.h>

using namespace Belle2;

/* Statics. */

int CIDManager::m_nbitsLayer = 3;       /**< Number of bits occupied by Layer ID.*/
int CIDManager::m_nbitsLadder = 5;      /**< Number of bits occupied by Ladder ID.*/
int CIDManager::m_nbitsSensor = 3;      /**< Number of bits occupied by Sensor ID.*/

/* Set number of layer bits.*/
void CIDManager::setBitLayer(int nbits) { m_nbitsLayer = nbits; }

/* Set number of ladder bits.*/
void CIDManager::setBitsLadder(int nbits) { m_nbitsLadder = nbits; }

/* Set number of sensor bits.*/
void CIDManager::setBitsSensor(int nbits) { m_nbitsSensor = nbits; }

/* Constructor. */
CIDManager::CIDManager(short int aCID):
    m_maskLayer(0),
    m_maskLadder(0),
    m_maskSensor(0),
    m_CID(aCID)
{
  // Check input settings
  if ((m_nbitsLayer + m_nbitsLadder + m_nbitsSensor) > 15)
    B2ERROR("CIDManager: Size of bit fields exceeds short int!")

    // Set shifts
    m_shiftSensor = 0;
  m_shiftLadder = m_nbitsSensor;
  m_shiftLayer  = m_nbitsSensor + m_nbitsLadder;

  // Set masks: shift bit and set in the corresponding mask.
  short int mask = 1;
  for (short int iBit = 0; iBit < m_nbitsSensor; ++iBit) {
    m_maskSensor |= mask;
    mask <<= 1;
  }
  for (short int iBit = 0; iBit < m_nbitsLadder; ++iBit) {
    m_maskLadder |= mask;
    mask <<= 1;
  }
  for (short int iBit = 0; iBit < m_nbitsLayer; ++iBit) {
    m_maskLayer |= mask;
    mask <<= 1;
  }
}

/* Setters.*/

/* Set layer ID.*/
void CIDManager::setLayerID(short int layerID)
{
  // Clear current layer bits
  short int newCID = m_CID & (~m_maskLayer);
  // Make mask with new bits
  short int mask = (layerID << m_shiftLayer) & m_maskLayer;
  // Apply
  m_CID = newCID | mask;
}

/* Set ladder ID.*/
void CIDManager::setLadderID(short int ladderID)
{
  // Clear current ladder bits
  short int newCID = m_CID & (~m_maskLadder);
  // Make mask with new bits
  short int mask = (ladderID << m_shiftLadder) & m_maskLadder;
  // Apply
  m_CID = newCID | mask;
}

/* Set sensor ID.*/
void CIDManager::setSensorID(short int sensorID)
{
  // Clear current sensor bits
  short int newCID = m_CID & (~m_maskSensor);
  // Make mask with new bits
  short int mask = (sensorID << m_shiftSensor) & m_maskSensor;
  // Apply
  m_CID = newCID | mask;
}

/** Getters.*/

/** Get decoded layer ID.*/
short int CIDManager::getLayerID() const
{
  // Get the requested bits
  short int result = m_CID & m_maskLayer;
  // bit-shift back
  return result >> m_shiftLayer;
}

/** Get decoded ladder ID.*/
short int CIDManager::getLadderID() const
{
  short int result = m_CID & m_maskLadder;
  return result >> m_shiftLadder;
}

/** Get decoded sensor ID.*/
short int CIDManager::getSensorID() const
{
  short int result = m_CID & m_maskSensor;
  return result >> m_shiftSensor;
}



