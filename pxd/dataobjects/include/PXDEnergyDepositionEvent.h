/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <algorithm>

namespace Belle2 {
  /** Class PXDEnergyDepositionEvent: PXDSimHit data container for background studies.
   *
   * This class holds particle hit data from geant4 simulation, and adds several
   * quantities for easy analysis of background studies. In particular, the class
   * is self-contained in that no relations or sensor data lookup is needed for
   * background analysis.
   */
  class PXDEnergyDepositionEvent : public RelationsObject  {

  public:

    /** default constructor for ROOT */
    PXDEnergyDepositionEvent(): m_time(0.0), m_u(0.0), m_v(0.0),
      m_energyDep(0.0), m_dose(0.0), m_expo(0.0),
      m_pdg(0), m_layer(0), m_ladder(0), m_sensor(0)
    {
      std::fill_n(m_globalPos, 3, 0.0);
    }

    /** Standard constructor
     * @param layer Layer number
     * @param ladder Ladder number
     * @param sensor Sensor number
     * @param pdg PDG Code of the particle producing the Hit
     * @param time Time of energy deposition
     * @param u Local u coordinate of the event
     * @param v Local v coordinate of the event
     * @param globalPos Global (x,y,z) of the event
     * @param energyDep Energy deposited by the event
     * @param dose Contribution to dose (Gy) by the event
     * @param expo Contribution to exposition (J/m2/s) by the event
     */
    PXDEnergyDepositionEvent(unsigned short layer, unsigned short ladder, unsigned short sensor,
                             int pdg, float time, float u, float v,
                             const float* globalPos, float energyDep, float dose, float expo):
      m_time(time), m_u(u), m_v(v), m_energyDep(energyDep), m_dose(dose), m_expo(expo),
      m_pdg(pdg), m_layer(layer), m_ladder(ladder), m_sensor(sensor)
    {
      std::copy_n(globalPos, 3, m_globalPos);
    }

    float m_globalPos[3]; /**< global (x,y,z) of particle crossing */
    float m_time;   /**< time of particle crossing */
    float m_u;      /**< local u-coordinate of particle crossing */
    float m_v;      /**< local v-coordinate of particle crossing */
    float m_energyDep;  /**< deposited energy */
    float m_dose;   /**< contribution to dose (in Gy) */
    float m_expo;   /**< contribution to exposition (J/s) */
    int m_pdg;      /**< PDG number of generating particle */
    unsigned short m_layer; /**< layer number */
    unsigned short m_ladder;  /**< ladder number */
    unsigned short m_sensor; /**< sensor number */

    ClassDef(PXDEnergyDepositionEvent, 3)
  };
} // end namespace Belle2
