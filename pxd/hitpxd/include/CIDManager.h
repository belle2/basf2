/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SD_CID_H
#define SD_CID_H

#include <framework/logging/Logger.h>

#include <TObject.h>

namespace Belle2 {

  /*
   ** A simple utility class to manage layer/ladder/sensor IDs.
   *
   * The class compresses integer (ladderID, layerID, sensorID) to
   * a short compressed ID (CID). It provides the corresponding
   * getters and setters to make the use of CIDs in lean SD hits
   * convenient.
   */
  class CIDManager {

  public:

    /*
     ** Constructor.
     *
     * @param aCID - an existing CID to decode. If omitted, IDs are expected
     * to be set by the user.
     * or an existing cid to decode.
     */
    CIDManager(short int aCID = 0);

    /**Setters.*/

    /** Set CID.*/
    void setCID(short int aCID) { m_CID = aCID; }

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

    /** Getters.*/

    /** Get encoded ID.*/
    short int getCID() const { return m_CID; }

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

    short int m_CID;                /**< The compressed ID.*/

    /*
     ** Static layout data.
     *
     * These are initialized to reasonable defaults for current PXD/SVD
     * design, but in principle can be changed for the class with the constraint
     * of total number of bits being at most 15.
     */
    static int m_nbitsLayer;       /**< Number of bits occupied by Layer ID.*/
    static int m_nbitsLadder;      /**< Number of bits occupied by Ladder ID.*/
    static int m_nbitsSensor;      /**< Number of bits occupied by Sensor ID.*/

  }; // class CID Manager

} // namespace Belle2

#endif


