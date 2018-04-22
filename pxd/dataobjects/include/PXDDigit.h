/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Zbynek Drasal, Martin Ritter            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDDIGIT_H
#define PXDDIGIT_H

#include <vxd/dataobjects/VxdID.h>

#include <framework/dataobjects/DigitBase.h>

namespace Belle2 {

  /**
   * The PXD digit class.
   *
   * This is a development implementation which is intentionally kept
   * somewhat bulky.
   */
  class PXDDigit : public DigitBase {
  public:

    /** Useful Constructor.
     * @param sensorID Sensor compact ID.
     * @param uCellID Cell ID in "r-phi".
     * @param vCellID Cell ID in "z".
     * @param charge The charge collected in the cell.
     */
    PXDDigit(VxdID sensorID, unsigned short uCellID, unsigned short vCellID,
             unsigned short charge):
      m_sensorID(sensorID),
      m_uCellID(uCellID), m_vCellID(vCellID),
      m_charge(charge)
    {;}

    /** Default constructor for the ROOT IO. */
    PXDDigit(): PXDDigit(0, 0, 0, 0) {}

    /** Get frame number of this digit.
     * @return frame number of the digit.
     */
    short int getFrameNumber() const { return VxdID(m_sensorID).getSegmentNumber(); }

    /** Set frame number of this digit.
     * @param frame Frame number to be set.
     */
    void setFrameNumber(unsigned short frame)
    {
      VxdID id(m_sensorID);
      id.setSegmentNumber(frame);
      m_sensorID = id;
    }

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get cell ID in u.
     * @return u ID of the cell.
     */
    unsigned short getUCellID() const { return m_uCellID; }

    /** Get cell ID in v.
     * @return v ID of the cell.
     */
    unsigned short getVCellID() const { return m_vCellID; }

    /** Get collected charge.
     * @return charge collected in the cell.
     */
    unsigned short getCharge() const { return m_charge; }

    /** Set digit charge.
     * @param charge The charge to be set to the digit.
     */
    void setCharge(unsigned short charge) { m_charge = charge; }

    /**
    * Implementation of the base class function.
    * Enables BG overlay module to identify uniquely the physical channel of this Digit.
    * @return unique channel ID, composed of VxdID (1 - 16), u pixel number (17-23), and v pixel number (24-32).
    */
    unsigned int getUniqueChannelID() const
    {
      // shift segment part of sensorID by two bits to make place for u+v
      VxdID sensorID(m_sensorID);
      sensorID.setSegmentNumber(sensorID.getSegmentNumber() << 2);
      return m_vCellID + (m_uCellID << 10) + (sensorID << 16);
    }

    /**
    * Implementation of the base class function.
    * Pile-up method.
    * @param bg BG digit
    * @return append status
    */
    DigitBase::EAppendStatus addBGDigit(const DigitBase* bg)
    {
      m_charge += dynamic_cast<const PXDDigit*>(bg)->getCharge();
      return DigitBase::c_DontAppend;
    }


  private:

    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    unsigned short m_uCellID;  /**< Cell r-phi coordinate in pitch units. */
    unsigned short m_vCellID;  /**< Cell z coordinate in pitch units. */
    unsigned short m_charge;   /**< Digitized charge in ADC units. */

    ClassDef(PXDDigit, 5)

  }; // class PXDDigit


} // end namespace Belle2

#endif
