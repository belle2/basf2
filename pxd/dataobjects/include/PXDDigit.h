/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Zbynek Drasal                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDDIGIT_H
#define PXDDIGIT_H

#include <TObject.h>

#define DEFAULT_PXDDIGITS "PXDDigits"
#define DEFAULT_PXDDIGITSREL "PXDDigitsToMCParticles"


namespace Belle2 {

  /*
   ** The PXD digit class.
   *
   * This is a development implementation which is intentionally kept
   * somewhat bulky. The coordinates probably won't be kept in future.
   */

  class PXDDigit : public TObject {

  public:

    /** Default constructor for the ROOT IO. */
    PXDDigit():
        m_sensorID(-1),
        m_uCellID(-1), m_vCellID(-1),
        m_uCellPosition(0), m_vCellPosition(0),
        m_charge(0)
    {;}

    /** Useful Constructor.
     * @param sensorID Sensor compact ID.
     * @param uCellID Cell ID in "z".
     * @param vCellID Cell ID in "r-phi".
     * @param uCell Cell center u coordinate.
     * @param vCell Cell center v coordinate.
     * @param charge The charge collected in the cell.
     */
    PXDDigit(int sensorID, short uCellID, short vCellID, float uCellPosition, float vCellPosition, float charge):
        m_sensorID(sensorID),
        m_uCellID(uCellID), m_vCellID(vCellID),
        m_uCellPosition(uCellPosition), m_vCellPosition(vCellPosition),
        m_charge(charge)
    {;}

    /** Set compressed layer/ladder/sensor id.
     * @param sensorID Compressed ID of the sensor.
     */
    void setSensorID(int sensorID) { m_sensorID = sensorID; }

    /** Set u ID of a cell.
     * @param uCellID Cell number in u.
     */
    void setUCellID(short uCellID) { m_uCellID = uCellID; }

    /** Set v ID of a cell.
     * @param vCellID Cell number in v.
     */
    void setVCellID(short vCellID) { m_vCellID = vCellID; }

    /** Set u cell coordinate.
     * @param uCellPosition Cell center u coordinate.
     */
    void setUCellPosition(float uCellPosition) { m_uCellPosition = uCellPosition; }

    /** Set v cell coordinate.
    * @param vCellPosition Cell center v coordinate.
    */
    void setVCellPosition(float vCellPosition) { m_vCellPosition = vCellPosition; }

    /** Set cell charge.
     * @param charge Cell charge.
     */
    void setCharge(float charge) { m_charge = charge; }

    /** Get the sensor compact ID.
     * @return compact ID of the sensor.
     */
    int getSensorID() const { return m_sensorID; }

    /** Get cell ID in u.
     * @return u ID of the cell.
     */
    float getUCellID() const { return m_uCellID; }

    /** Get cell ID in v.
     * @return v ID of the cell.
     */
    float getVCellID() const { return m_vCellID; }

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

  private:

    int m_sensorID;                 /**< Compressed sensor identifier.*/
    short m_uCellID;        /**< Cell z-coordinate in pitch units. */
    short m_vCellID;        /**< Cell r-phi coordinate in pitch units. */
    float m_uCellPosition;          /**< Absolute cell position in z. */
    float m_vCellPosition;          /**< Absolute cell position in r-phi. */
    float m_charge;         /**< Deposited charge (units depend on user selection). */

    ClassDef(PXDDigit, 1);

  }; // class PXDDigit

} // end namespace Belle2

#endif
