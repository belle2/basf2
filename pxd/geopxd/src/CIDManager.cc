/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/geopxd/CIDManager.h>

using namespace Belle2;

/* Statics. */

int SensorUIDManager::m_nbitsLayer = 3;       /**< Number of bits occupied by Layer ID.*/
int SensorUIDManager::m_nbitsLadder = 5;      /**< Number of bits occupied by Ladder ID.*/
int SensorUIDManager::m_nbitsSensor = 3;      /**< Number of bits occupied by Sensor ID.*/

/* Set number of layer bits.*/
void SensorUIDManager::setBitLayer(int nbits) { m_nbitsLayer = nbits; }

/* Set number of ladder bits.*/
void SensorUIDManager::setBitsLadder(int nbits) { m_nbitsLadder = nbits; }

/* Set number of sensor bits.*/
void SensorUIDManager::setBitsSensor(int nbits) { m_nbitsSensor = nbits; }

/* Constructor. */
SensorUIDManager::SensorUIDManager(short int aCID):
    m_maskLayer(0),
    m_maskLadder(0),
    m_maskSensor(0),
    m_SensorUID(aCID)
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
void SensorUIDManager::setLayerID(short int layerID)
{
  // Clear current layer bits
  short int newCID = m_SensorUID & (~m_maskLayer);
  // Make mask with new bits
  short int mask = (layerID << m_shiftLayer) & m_maskLayer;
  // Apply
  m_SensorUID = newCID | mask;
}

/* Set ladder ID.*/
void SensorUIDManager::setLadderID(short int ladderID)
{
  // Clear current ladder bits
  short int newCID = m_SensorUID & (~m_maskLadder);
  // Make mask with new bits
  short int mask = (ladderID << m_shiftLadder) & m_maskLadder;
  // Apply
  m_SensorUID = newCID | mask;
}

/* Set sensor ID.*/
void SensorUIDManager::setSensorID(short int sensorID)
{
  // Clear current sensor bits
  short int newCID = m_SensorUID & (~m_maskSensor);
  // Make mask with new bits
  short int mask = (sensorID << m_shiftSensor) & m_maskSensor;
  // Apply
  m_SensorUID = newCID | mask;
}

/** Getters.*/

/** Get decoded layer ID.*/
short int SensorUIDManager::getLayerID() const
{
  // Get the requested bits
  short int result = m_SensorUID & m_maskLayer;
  // bit-shift back
  return result >> m_shiftLayer;
}

/** Get decoded ladder ID.*/
short int SensorUIDManager::getLadderID() const
{
  short int result = m_SensorUID & m_maskLadder;
  return result >> m_shiftLadder;
}

/** Get decoded sensor ID.*/
short int SensorUIDManager::getSensorID() const
{
  short int result = m_SensorUID & m_maskSensor;
  return result >> m_shiftSensor;
}
//*****************************************************************************
// CellUID Manager
//****************************************************************************

/* Statics. */

int CellUIDManager::m_nbitsUCellID = 11;       /**< Number of bits occupied by UCellID.*/
int CellUIDManager::m_nbitsVCellID = 11;       /**< Number of bits occupied by VCellID.*/

/* Set number of u cellID bits.*/
void CellUIDManager::setBitsUCellID(int nbits) { m_nbitsUCellID = nbits; }

/* Set number of ladder bits.*/
void CellUIDManager::setBitsVCellID(int nbits) { m_nbitsVCellID = nbits; }

/* Constructor. */
CellUIDManager::CellUIDManager(int aCellUID):
    m_maskUCellID(0),
    m_maskVCellID(0),
    m_CellUID(aCellUID)
{
  // Check input settings
  if ((m_nbitsUCellID + m_nbitsVCellID) > 31)
    B2ERROR("CellUIDManager: Size of bit fields exceeds int!")

    // Set shifts
    m_shiftVCellID = 0;
  m_shiftUCellID = m_nbitsVCellID;

  // Set masks: shift bit and set in the corresponding mask.
  int mask = 1;
  for (short int iBit = 0; iBit < m_nbitsVCellID; ++iBit) {
    m_maskVCellID |= mask;
    mask <<= 1;
  }
  for (short int iBit = 0; iBit < m_nbitsUCellID; ++iBit) {
    m_maskUCellID |= mask;
    mask <<= 1;
  }
}

/* Setters.*/

/* Set u cell ID.*/
void CellUIDManager::setUCellID(short int uCellID)
{
  // Clear current uCellID bits
  int newCellUID = m_CellUID & (~m_maskUCellID);
  // Make mask with new bits
  int mask = (uCellID << m_shiftUCellID) & m_maskUCellID;
  // Apply
  m_CellUID = newCellUID | mask;
}

/* Set v cell ID.*/
void CellUIDManager::setVCellID(short int vCellID)
{
  // Clear current ladder bits
  int newCellUID = m_CellUID & (~m_maskVCellID);
  // Make mask with new bits
  short int mask = (vCellID << m_shiftVCellID) & m_maskVCellID;
  // Apply
  m_CellUID = newCellUID | mask;
}

/** Getters.*/

/** Get decoded u cell ID.*/
short int CellUIDManager::getUCellID() const
{
  // Get the requested bits
  int result = m_CellUID & m_maskUCellID;
  // bit-shift back
  return static_cast<short>(result >> m_shiftUCellID);
}

/** Get decoded v cell ID.*/
short int CellUIDManager::getVCellID() const
{
  int result = m_CellUID & m_maskVCellID;
  return static_cast<short>(result >> m_shiftVCellID);
}
