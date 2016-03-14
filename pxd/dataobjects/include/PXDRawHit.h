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
      m_startRow(0),
      m_frameNr(0),
      m_commonMode(0) {};


    /**
     * @param sensorID Sensor compact ID.
     * @param row Pixel row coordinate.
     * @param column Pixel column coordinate.
     * @param charge The Pixel charge.
     * @param startRow Number of the row where the reading electronic startet.
     * @param frameNr number of the (readout) frame.
     * @param commonMode Common Mode correction for this pixel.
     */
    PXDRawHit(VxdID sensorID, short row, short column, short charge,
              unsigned short startRow, unsigned int frameNr, unsigned int commonMode = 0):
      m_sensorID(sensorID), m_row(row), m_column(column),
      m_charge(charge), m_startRow(startRow), m_frameNr(frameNr), m_commonMode(commonMode)
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
      return m_startRow;
    }

    /** Get number of the Frames.
     * @return Number of the Frames.
     */
    unsigned short getFrameNr() const {
      return m_frameNr;
    }

    /** Get Common Mode correction.
     * @return Common Mode correction.
     */
    unsigned short getCommonMode() const {
      return m_commonMode;
    }

  protected:
    unsigned short m_sensorID; /**< Compressed sensor identifier. actually a VxdID object*/
    short m_row;         /**< Absolute pixel position in u. */
    short m_column;         /**< Absolute pixel position in v. */
    short m_charge;           /**< Deposited charge in pixel. */
    unsigned short m_startRow;  /**< pixel row where reading starts */
    unsigned short m_frameNr;    /**< Number of the Frames */
    short m_commonMode;    /**< Common Mode correction */

    // ~PXDRawHit();

    ClassDef(PXDRawHit, 2)
  };

  /** @}*/

} //Belle2 namespace
#endif
