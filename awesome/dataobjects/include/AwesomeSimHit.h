/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <framework/datastore/RelationsObject.h>
#include <framework/geometry/B2Vector3.h>

namespace Belle2 {

  /**
   * AwesomeSimHit - Geant4 simulated hit for the Awesome detector.
   *
   * This class holds particle hit data from Geant4 simulation.
   * As the simulated hit classes are used to generate detector response,
   * they should contain _local_ information.
   */
  class AwesomeSimHit : public RelationsObject {

  public:

    /**
     * Default constructor.
     */
    AwesomeSimHit() :
      m_energyDep{0},
      m_position{0., 0., 0.},
      m_time{0}
    {}

    /**
     * Standard constructor.
     * @param energyDep Deposited energy in MeV.
     * @param position  Vector for position in cm.
     * @param time      Time in ns.
     */
    AwesomeSimHit(float energyDep, B2Vector3<float> position, float time) :
      m_energyDep{energyDep},
      m_position{position},
      m_time{time}
    {}

    /**
     * Get the deposited energy.
     * @return deposited energy in MeV.
     */
    float getEnergyDep() const
    {
      return m_energyDep;
    }

    /**
     * Set the deposited energy.
     * @param energy Deposited energy in MeV.
     */
    void setEnergyDep(float energyDep)
    {
      m_energyDep = energyDep;
    }

    /**
     * Get the vector for position.
     * @return vector for position in cm.
     */
    B2Vector3<float> getPosition() const
    {
      return m_position;
    }

    /**
     * Set the vector for position.
     * @param position Vector for position in cm.
     */
    void setPosition(B2Vector3<float> position)
    {
      m_position = position;
    }

    /**
     * Get the time.
     * @return time in ns.
     */
    float getTime() const
    {
      return m_time;
    }

    /**
     * Set the time.
     * @param time Time in ns.
     */
    void setTime(float time)
    {
      m_time = time;
    }

  private:

    /** Deposited energy in MeV. */
    float m_energyDep;

    /** Vector for position in cm. */
    B2Vector3<float> m_position;

    /** Time in ns. */
    float m_time;

    /** Class versions, required by the ROOT streamer. */
    ClassDef(AwesomeSimHit, 1)

  };

}
