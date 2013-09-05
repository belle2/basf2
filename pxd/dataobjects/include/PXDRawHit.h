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
#include <TObject.h>

namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

  /** The PXD Raw Hit class
    * This class stores information about PXD Pixel hits
    * and makes them available in a root tree
    */
  class PXDRawHit : public TObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDRawHit():
      m_sensorID(0),
      m_row(0),
      m_column(0),
      m_charge(0) ,
      m_StartRow(0),
      m_FrameNr(0) {};


    /**
     * @param sensorID Sensor compact ID.
     * @param row Pixel row coordinate.
     * @param column Pixel column coordinate.
     * @param charge The Pixel charge.
     * @param StartRow Number of the row where the reading electronic startet.
     * @param FrameNr number of frame.
     */
    PXDRawHit(VxdID sensorID, short row, short column, short charge,
              unsigned short StartRow, unsigned int FrameNr):
      m_sensorID(sensorID), m_row(row), m_column(column),
      m_charge(charge), m_StartRow(StartRow), m_FrameNr(FrameNr)
    {};

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const {
      return m_sensorID;
    }

    /** Get u coordinate of hit position.
     * @return u coordinate of hit position.
     */
    short getRow() const {
      return m_row;
    }

    /** Get v coordinate of hit position.
     * @return v coordinate of hit position.
     */
    short getColumn() const {
      return m_column;
    }

    /** Get collected charge.
     * @return charge collected in the pixel.
     */
    short getCharge() const {
      return m_charge;
    }

    /** Get cluster start pixel in u direction.
     * @return row where reading begins.
     */
    unsigned short getStartRow() const {
      return m_StartRow;
    }

    /** Get number of Frames.
     * @return Number of Frames.
     */
    unsigned short getFrameNr() const {
      return m_FrameNr;
    }

  protected:
    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    short m_row;         /**< Absolute pixel position in u. */
    short m_column;         /**< Absolute pixel position in v. */
    short m_charge;           /**< Deposited charge in pixel. */
    unsigned short m_StartRow;  /**< pixel where reading starts */
    unsigned short m_FrameNr;    /**< Number of Frames */

    // ~PXDRawHit();

    ClassDef(PXDRawHit, 1)
  };

  /** @}*/

} //Belle2 namespace
#endif
