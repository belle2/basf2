/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPTBSIMHIT_H
#define TOPTBSIMHIT_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Class to store beam test simulated hits for any counter other than TOP
   */
  class TOPTBSimHit : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPTBSimHit():
      m_counterID(0),
      m_type(0),
      m_channelID(0),
      m_energyDeposit(0),
      m_time(0),
      m_x(0),
      m_y(0),
      m_z(0),
      m_driftLength(0) {
    }

    /**
     * Full constructor
     * @param counterID counter ID
     * @param type counter type
     * @param channelID software channel ID
     * @param energyDeposit energy deposit [MeV] or Cerenkov photon energy [eV]
     * @param time global time
     * @param x local x-coordinate
     * @param y local y-coordinate
     * @param z local z-coordinate
     * @param driftLength drift length (drift chambers only)
     */
    TOPTBSimHit(int counterID, int type, int channelID, double energyDeposit,
                double time, double x, double y, double z, double driftLength = 0):
      m_counterID(counterID),
      m_type(type),
      m_channelID(channelID),
      m_energyDeposit(energyDeposit),
      m_time(time),
      m_x(x),
      m_y(y),
      m_z(z),
      m_driftLength(driftLength) {
    }

    /** Get counter ID
    * @return counter ID
    */
    int getCounterID() const { return m_counterID; }

    /** Get counter type
    * @return counter type
    */
    int getType() const { return m_type;}

    /** Get software channel ID
     * @return channel ID
     */
    int getChannelID() const { return m_channelID; }

    /** Get energy deposit or Cerenkov photon energy, depending on counter type
     * @return energy deposit in MeV (or Cerenkov photon energy in eV)
     */
    double getEnergyDeposit() const { return m_energyDeposit;}

    /** Get global time
     * @return time
     */
    double getTime() const { return m_time;}

    /** Get local coordinate x
     * @return x-coordinate
     */
    double getX() const { return m_x;}

    /** Get local coordinate y
     * @return y-coordinate
     */
    double getY() const { return m_y;}

    /** Get local coordinate z
     * @return z-coordinate
     */
    double getZ() const { return m_z;}

    /** Get drift lenght
     * @return return drift length (drift chambers) or 0
     */
    double getDriftLength() const { return m_driftLength;}


  private:
    int m_counterID;          /**< counter ID */
    int m_type;               /**< counter type */
    int m_channelID;          /**< software channel ID */
    float m_energyDeposit;    /**< energy deposit [MeV] or Cerenkov photon energy [eV] */
    float m_time;             /**< global time */
    float m_x;                /**< local x-coordinate */
    float m_y;                /**< local y-coordinate */
    float m_z;                /**< local z-coordinate */
    float m_driftLength;      /**< drift length (drift chambers only) */

    ClassDef(TOPTBSimHit, 1); /**< ClassDef */

  };


} // end namespace Belle2

#endif
