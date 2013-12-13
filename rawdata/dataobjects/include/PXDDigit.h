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

#include <TObject.h>

namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

  /**
   * The PXD digit class.
   *
   * This is a development implementation which is intentionally kept
   * somewhat bulky. The coordinates probably won't be kept in future.
   */
  class PXDDigit : public TObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDigit():
      m_sensorID(0),
      m_uCellID(-1), m_vCellID(-1),
      m_uCellPosition(0), m_vCellPosition(0),
      m_charge(0)
    {;}

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

  private:

    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    short m_uCellID;           /**< Cell r-phi coordinate in pitch units. */
    short m_vCellID;           /**< Cell z coordinate in pitch units. */
    float m_uCellPosition;     /**< Absolute cell position in r-phi. */
    float m_vCellPosition;     /**< Absolute cell position in z. */
    float m_charge;            /**< Deposited charge (units depend on user selection). */

    ClassDef(PXDDigit, 1)

  }; // class PXDDigit

  /** @}*/

} // end namespace Belle2

#endif
