/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /** The PXD Raw Hit class
    * This class stores information about PXD Pixel hits
    * and makes them available in a root tree
    */
  class PXDRawHit : public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDRawHit():
      m_sensorID(0),
      m_row(0),
      m_column(0),
      m_charge(0) ,
      m_frameNr(0) {};


    /**
     * @param sensorID Sensor compact ID.
     * @param row Pixel row coordinate (V Cell ID).
     * @param column Pixel column coordinate ( U Cell ID).
     * @param charge The Pixel charge.
     * @param frameNr number of the (readout) frame.
     */
    PXDRawHit(VxdID sensorID, short row, short column, short charge,
              unsigned int frameNr):
      m_sensorID(sensorID), m_row(row), m_column(column),
      m_charge(charge), m_frameNr(frameNr)
    {};

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const
    {
      return m_sensorID;
    }

    /** Get v cell id of hit position.
     * @return v cell id of hit position.
     */
    short getRow() const
    {
      return m_row;
    }

    /** Get v cell id of hit position.
     * @return v cell id of hit position.
     */
    short getVCellID() const
    {
      return m_row;
    }

    /** Get u cell id of hit position.
     * @return u cell id of hit position.
     */
    short getColumn() const
    {
      return m_column;
    }

    /** Get u cell id of hit position.
     * @return u cell id of hit position.
     */
    short getUCellID() const
    {
      return m_column;
    }

    /** Get collected charge.
     * @return charge collected in the pixel.
     */
    short getCharge() const
    {
      return m_charge;
    }

    /** Get frame number.
     * @return Number of the Frame.
     */
    unsigned short getFrameNr() const
    {
      return m_frameNr;
    }


  private:
    unsigned short m_sensorID; /**< Compressed sensor identifier. actually a VxdID object*/
    short m_row;         /**< Absolute pixel position in v (V Cell ID). */
    short m_column;         /**< Absolute pixel position in u (U Cell ID). */
    short m_charge;           /**< Deposited charge in pixel. */
    unsigned short m_frameNr;    /**< Number of the Frames */

    ClassDef(PXDRawHit, 6)
  };


} //Belle2 namespace
