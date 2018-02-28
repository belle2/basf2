/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:Bjoern Spruck / Klemens Lautenbach                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDRAWHIT_H
#define PXDRAWHIT_H


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
     * @param row Pixel row coordinate.
     * @param column Pixel column coordinate.
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

    /** Get v coordinate of hit position.
     * @return v coordinate of hit position.
     */
    short getRow() const
    {
      return m_row;
    }

    /** Get u coordinate of hit position.
     * @return u coordinate of hit position.
     */
    short getColumn() const
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
    short m_row;         /**< Absolute pixel position in v. */
    short m_column;         /**< Absolute pixel position in v. */
    short m_charge;           /**< Deposited charge in pixel. */
    unsigned short m_frameNr;    /**< Number of the Frames */

    ClassDef(PXDRawHit, 6)
  };


} //Belle2 namespace
#endif
