/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef SVD_DATAOBJECTS_NEUTRONFLUXEVENT_H
#define SVD_DATAOBJECTS_NEUTRONFLUXEVENT_H

#include <framework/datastore/RelationsObject.h>
#include <algorithm>

namespace Belle2 {
  /** Class SVDNeutronFluxEvent: SVDTrueHit data container for background studies.
   *
   * This class holds particle hit data from geant4 simulation, and adds several
   * quantities for easy analysis of background studies. In particular, the class
   * is self-contained in that no relations or sensor data lookup is needed for
   * baseline background analysis.
   */
  class SVDNeutronFluxEvent : public RelationsObject  {

  public:

    /** default constructor for ROOT */
    SVDNeutronFluxEvent(): m_layer(0), m_ladder(0), m_sensor(0),
      m_pdg(0), m_time(0.0), m_u(0.0), m_v(0.0),
      m_kineticEnergy(0.0), m_stepLength(0.0), m_nielFactor(0.0), m_rawFlux(0.0),
      m_nielFlux(0.0)
    {
      std::fill_n(m_globalPos, 3, 0.0);
      std::fill_n(m_globalMom, 3, 0.0);
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
     * @param globalMom Global (px,py,pz) of the particle
     * @param kineticEnergy Kinetic energy of the particle
     * @param stepLength Distance traveled by the particle in the sensor
     * @param nielFactor NIEL correction factor for the particle and its kinetic energy
     * @param rawFlux Raw (unscaled) contribution to flux by the event
     * @param nielFlux Scaled contribution to flux by the event
     */
    SVDNeutronFluxEvent(unsigned short layer, unsigned short ladder, unsigned short sensor,
                        int pdg, float time, float u, float v,
                        const float* globalPos, const float* globalMom, float kineticEnergy,
                        float stepLength, float nielFactor, float rawFlux, float nielFlux):
      m_layer(layer), m_ladder(ladder), m_sensor(sensor),
      m_pdg(pdg), m_time(time), m_u(u), m_v(v), m_kineticEnergy(kineticEnergy),
      m_stepLength(stepLength), m_nielFactor(nielFactor), m_rawFlux(rawFlux),
      m_nielFlux(nielFlux)
    {
      std::copy_n(globalPos, 3, m_globalPos);
      std::copy_n(globalMom, 3, m_globalMom);
    }

    unsigned short m_layer; /**< layer number */
    unsigned short m_ladder;  /**< ladder number */
    unsigned short m_sensor; /**< sensor number */
    int m_pdg;      /**< PDG number of generating particle */
    float m_time;   /**< time of particle crossing */
    float m_u;      /**< local u-coordinate of particle crossing */
    float m_v;      /**< local v-coordinate of particle crossing */
    float m_globalPos[3]; /**< global (x,y,z) of particle crossing */
    float m_globalMom[3]; /**< global (px, py, pz) of the particle */
    float m_kineticEnergy; /**< kinetic energy of the particle */
    float m_stepLength; /**< step length */
    float m_nielFactor; /**< NIEL scaling factor for the particle and kinetic energy */
    float m_rawFlux;    /**< Raw particle flux */
    float m_nielFlux;   /**< NIEL-corrected flux */

    ClassDef(SVDNeutronFluxEvent, 1)
  };
} // end namespace Belle2

#endif
