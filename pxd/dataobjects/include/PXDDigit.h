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
   * somewhat bulky. The coordinates probably won't be kept in future.
   */
  class PXDDigit : public DigitBase {
  public:

    /** Useful Constructor.
     * @param sensorID Sensor compact ID.
     * @param uCellID Cell ID in "r-phi".
     * @param vCellID Cell ID in "z".
     * @param uCell Cell center u coordinate.
     * @param vCell Cell center v coordinate.
     * @param charge The charge collected in the cell.
     */
    PXDDigit(VxdID sensorID, short uCellID, short vCellID, float uCellPosition, float vCellPosition, float charge):
      m_sensorID(sensorID),
      m_uCellID(uCellID), m_vCellID(vCellID),
      m_uCellPosition(uCellPosition), m_vCellPosition(vCellPosition),
      m_charge(charge)
    {;}

    /** Default constructor for the ROOT IO. */
    PXDDigit(): PXDDigit(0, -1, -1, 0, 0, 0) {}

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
    short int getUCellID() const { return m_uCellID; }

    /** Get cell ID in v.
     * @return v ID of the cell.
     */
    short int getVCellID() const { return m_vCellID; }

    /** Get u coordinate of cell center.
     * @return u coordinate of cell center.
     */
    float getUCellPosition() const { return m_uCellPosition; }

    /** Get v coordinate of cell center.
     * @return v coordinate of cell center.
     */
    float getVCellPosition() const { return m_vCellPosition; }

    /** Get collected charge.
     * @return charge collected in the cell.
     */
    float getCharge() const { return m_charge; }

    /** Set digit charge.
     * @param charge The charge to be set to the digit.
     */
    void setCharge(float charge) { m_charge = charge; }

    /**
    * Implementation of the base class function.
    * Enables BG overlay module to identify uniquely the physical channel of this Digit.
    * @return unique channel ID, composed of VxdID (1 - 16), u pixel number (17-23), and v pixel number (24-32).
    */
    unsigned int getUniqueChannelID() const
    { return m_vCellID + (m_uCellID << 9) + (m_sensorID << 16); }

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
    short m_uCellID;           /**< Cell r-phi coordinate in pitch units. */
    short m_vCellID;           /**< Cell z coordinate in pitch units. */
    float m_uCellPosition;     /**< Absolute cell position in r-phi. */
    float m_vCellPosition;     /**< Absolute cell position in z. */
    float m_charge;            /**< Deposited charge (units depend on user selection). */

    ClassDef(PXDDigit, 4)

  }; // class PXDDigit


} // end namespace Belle2

#endif
