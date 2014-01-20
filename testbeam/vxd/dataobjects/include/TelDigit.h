/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Zbynek Drasal, Martin Ritter            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TELDIGIT_H
#define TELDIGIT_H

#include <vxd/dataobjects/VxdID.h>

#include <TObject.h>

namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

  /**
   * The EUDET telescope digit class.
   */

  class TelDigit : public TObject {
  public:

    /** Full Constructor.
     * @param sensorID Sensor compact ID.
     * @param uCellID Cell ID in u.
     * @param vCellID Cell ID in v.
     */
    TelDigit(VxdID sensorID, short uCellID, short vCellID, float charge = 1):
      m_sensorID(sensorID), m_uCellID(uCellID), m_vCellID(vCellID),
      m_charge(charge)
    {;}

    /** Default constructor for the ROOT IO. */
    TelDigit(): TelDigit(0, -1, -1, 1)
    {;}

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get cell ID in u.
     * @return u ID of the cell.
     */
    short getUCellID() const { return m_uCellID; }

    /** Get cell ID in v.
     * @return v ID of the cell.
     */
    short getVCellID() const { return m_vCellID; }

    /** Get digit charge */
    float getCharge() const { return m_charge; }

  private:

    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    short m_uCellID;           /**< Cell r-phi coordinate in pitch units. */
    short m_vCellID;           /**< Cell z coordinate in pitch units. */
    float m_charge;             /**< Dummy charge member for historical reasons. */

    ClassDef(TelDigit, 2)

  }; // class TelDigit

  /** @}*/

} // end namespace Belle2

#endif
