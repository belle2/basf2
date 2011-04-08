/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SD_CID_H
#define SD_CID_H

#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * A simple utility class to manage layer/ladder/sensor IDs.
   *
   * The class compresses integer (ladderID, layerID, sensorID) to
   * a short compressed ID (CID). It provides the corresponding
   * getters and setters to make the use of CIDs in lean SD hits
   * convenient.
   * <p>
   * Bit layout data of the class are static and are initialized to reasonable
   * defaults for current PXD/SVD design, but in principle can be changed with
   * the constraint of total number of bits being at most 15.
   */
  class SensorUIDManager {

  public:

    /**
     * Constructor.
     *
     * @param aCID An existing CID to decode. If omitted or zero, IDs are expected
     * to be set by the user.
     */
    SensorUIDManager(short int aCID = 0);

    /** Set CID.*/
    void setCID(short int aCID) { m_SensorUID = aCID; }

    /** Set layer ID.*/
    void setLayerID(short int layerID);

    /** Set ladder ID.*/
    void setLadderID(short int ladderID);

    /** Set sensor ID.*/
    void setSensorID(short int sensorID);

    /** Set number of layer bits.*/
    static void setBitLayer(int nbits);

    /** Set number of ladder bits.*/
    static void setBitsLadder(int nbits);

    /** Set number of sensor bits.*/
    static void setBitsSensor(int nbits);

    /** Get encoded ID.*/
    short int getSensorUID() const { return m_SensorUID; }

    /** Get decoded layer ID.*/
    short int getLayerID() const;

    /** Get decoded ladder ID.*/
    short int getLadderID() const;

    /** Get decoded sensor ID.*/
    short int getSensorID() const;

  private:

    short int m_maskLayer;           /**< Mask to reset Layer ID.*/
    short int m_maskLadder;          /**< Mask to reset Ladder ID.*/
    short int m_maskSensor;          /**< Mask to reset Sensor ID.*/

    short int m_shiftLayer;          /**< Bit shift of layer bit-field.*/
    short int m_shiftLadder;         /**< Bit shift of ladder bit-field.*/
    short int m_shiftSensor;         /**< Bit shift of sensor bit-field.*/

    short int m_SensorUID;                /**< The compressed ID.*/

    static int m_nbitsLayer;       /**< Number of bits occupied by Layer ID.*/
    static int m_nbitsLadder;      /**< Number of bits occupied by Ladder ID.*/
    static int m_nbitsSensor;      /**< Number of bits occupied by Sensor ID.*/

  }; // class Sensor UID Manager

  /**
   * CellUID manager class: encode and decode pixel/strip IDs.
   * The implementation is the same as for the CID manager, except that the CellUID
   * is an int. Again, widths of bit fields can be set through static setters,
   * but there are reasonable defaults in place.
   */
  class CellUIDManager {

  public:

    /**
     * Constructor.
     *
     * @param aCellUID An existing CellUID to decode. If omitted or zero, IDs are expected
     * to be set by the user.
     */
    CellUIDManager(int aCellUID = 0);

    /** Set CellUID.*/
    void setCellUID(int aCellUID) { m_CellUID = aCellUID; }

    /** Set u cell ID.*/
    void setUCellID(short int uCellID);

    /** Set v cell ID.*/
    void setVCellID(short int vCellID);

    /** Set number of u cell ID bits.*/
    static void setBitsUCellID(int nbits);

    /** Set number of v cell ID bits.*/
    static void setBitsVCellID(int nbits);

    /** Get encoded ID.*/
    int getCellUID() const { return m_CellUID; }

    /** Get decoded u cell ID.*/
    short int getUCellID() const;

    /** Get decoded v cell ID.*/
    short int getVCellID() const;

  private:

    int m_maskUCellID;           /**< Mask to reset Layer ID.*/
    int m_maskVCellID;          /**< Mask to reset Ladder ID.*/

    short int m_shiftUCellID;         /**< Bit shift of uCellID bit-field.*/
    short int m_shiftVCellID;         /**< Bit shift of vCellID bit-field.*/

    int m_CellUID;                /**< The compressed ID.*/

    static int m_nbitsUCellID;      /**< Number of bits occupied by Layer ID.*/
    static int m_nbitsVCellID;      /**< Number of bits occupied by Ladder ID.*/

  }; // class Cell UID Manager





} // namespace Belle2

#endif


