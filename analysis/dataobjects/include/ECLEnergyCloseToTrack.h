/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /** Class to store energy information for ECL hits closest to a track */
  class ECLEnergyCloseToTrack : public RelationsObject {

  public:

    /**
     * Default constructor.
     */
    ECLEnergyCloseToTrack() :
      m_energy3FWDBarrel(0.0),
      m_energy3FWDEndcap(0.0),
      m_energy3BWDBarrel(0.0),
      m_energy3BWDEndcap(0.0),
      m_extTheta(-999.),
      m_extPhi(-999.),
      m_extPhiId(-1)
    {}

    /** Set m_energy3FWDBarrel */
    void setEnergy3FWDBarrel(float energy3low) {m_energy3FWDBarrel = energy3low;}

    /** Get m_energy3FWDBarrel */
    float getEnergy3FWDBarrel() const {return m_energy3FWDBarrel;}

    /** Set m_energy3FWDEndcap */
    void setEnergy3FWDEndcap(float energy3high) {m_energy3FWDEndcap = energy3high;}

    /** Get m_energy3FWDEndcap */
    float getEnergy3FWDEndcap() const {return m_energy3FWDEndcap;}

    /** Set m_energy3FWDBarrel */
    void setEnergy3BWDBarrel(float energy3low) {m_energy3BWDBarrel = energy3low;}

    /** Get m_energy3BFWDBarrel */
    float getEnergy3BWDBarrel() const {return m_energy3BWDBarrel;}

    /** Set m_energy3BWDEndcap */
    void setEnergy3BWDEndcap(float energy3high) {m_energy3BWDEndcap = energy3high;}

    /** Get m_energy3BWDEndcap */
    float getEnergy3BWDEndcap() const {return m_energy3BWDEndcap;}

    /** Set m_extTheta */
    void setExtTheta(float theta) {m_extTheta = theta;}

    /** Get m_extTheta */
    float getExtTheta() const {return m_extTheta;}

    /** Set m_extPhi */
    void setExtPhi(float phi) {m_extPhi = phi;}

    /** Get m_extPhi */
    float getExtPhi() const {return m_extPhi;}

    /** Set m_extPhiId */
    void setExtPhiId(float phi) {m_extPhiId = phi;}

    /** Get m_extPhiId */
    float getExtPhiId() const {return m_extPhiId;}
  private:

    float m_energy3FWDBarrel; /**<energy sum of the three ECLCalDigits on the barrel side, FWD */
    float m_energy3FWDEndcap; /**<energy sum of the three ECLCalDigits on the endcap side, FWD */
    float m_energy3BWDBarrel; /**<energy sum of the three ECLCalDigits on the barrel side, BWD */
    float m_energy3BWDEndcap; /**<energy sum of the three ECLCalDigits on the endcap side, BWD */
    float m_extTheta; /**< track extrapolation theta */
    float m_extPhi; /**< track extrapolation phi */
    float m_extPhiId; /**< track extrapolation phi id */

    ClassDef(ECLEnergyCloseToTrack, 1); /**< class definition */

  };

} // end namespace Belle2
