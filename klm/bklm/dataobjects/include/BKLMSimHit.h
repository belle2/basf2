/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <klm/bklm/dataobjects/BKLMElementNumbers.h>
#include <klm/bklm/dataobjects/BKLMStatus.h>
#include <framework/datastore/RelationsObject.h>
#include <simulation/dataobjects/SimHitBase.h>

//#define BKLM_INNER 1
//#define BKLM_OUTER 2

namespace Belle2 {

  //! Store one simulation hit as a ROOT object
  class BKLMSimHit : public SimHitBase {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMSimHit();

    //! Constructor with initial values
    //! @param moduleID module identifier
    //! @param x position along the strip (cm)
    //! @param t time since start of event (ns)
    //! @param dE deposited energy (MeV)
    //! @sa BKLMStatus.h for moduleID
    BKLMSimHit(int moduleID, double x, double t, double dE);

    //! Destructor
    virtual ~BKLMSimHit() {}

    //! Copy constructor
    BKLMSimHit(const BKLMSimHit&);

    //! Assignment operator
    BKLMSimHit& operator=(const BKLMSimHit&);

    //! Determine whether this hit is in an RPC or scintillator
    //! @return whether this hit is in an RPC (true) or scintillator (false)
    bool inRPC() const { return ((m_ModuleID & BKLM_INRPC_MASK) != 0); }

    //! Get section number
    //! @return section number (1=forward or 0=backward) of this hit
    int getSection() const { return ((m_ModuleID & BKLM_END_MASK) >> BKLM_END_BIT); }

    //! Get sector number
    //! @return sector number of this hit
    int getSector() const { return (((m_ModuleID & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1); }

    //! Get layer number
    //! @return layer number of this hit
    int getLayer() const { return (((m_ModuleID & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1); }

    //! Get plane number.
    //! @return Plane number (0=z, 1=phi).
    bool getPlane() const { return BKLMElementNumbers::getPlaneByModule(m_ModuleID);}

    //! Get readout coordinate
    //! @return readout coordinate (TRUE=phi, FALSE=z) of this hit
    bool isPhiReadout() const { return ((m_ModuleID & BKLM_PLANE_MASK) != 0); }

    //! Get strip number of this hit
    //! @return readout strip number of this hit (assuming one strip per hit)
    int getStrip() const { return BKLMElementNumbers::getStripByModule(m_ModuleID); }

    //! Get lowest readout strip number of a contiguous set
    //! @return lowest readout strip number of this hit (assuming a contiguous set of RPC strips)
    int getStripMin() const { return BKLMElementNumbers::getStripByModule(m_ModuleID); }

    //! Get highest readout strip number of a contiguous set
    //! @return highest readout strip number of this hit (assuming a contiguous set of RPC strips)
    int getStripMax() const { return (((m_ModuleID & BKLM_MAXSTRIP_MASK) >> BKLM_MAXSTRIP_BIT) + 1); }

    //! Get detector-module identifier
    //! @return detector-module identifier
    int getModuleID() const { return m_ModuleID; }

    //! Get signal propagation time alont strip
    //! @return time-of-propagation (ns) from hit to sensor
    float getPropagationTime() const { return m_PropagationTime; }

    //! Get the hit time (implementation of base class function
    //! @return the hit time (ns) since start of the event
    float getGlobalTime() const override { return m_Time; }

    //! Get the hit time
    //! @return the hit time (ns) since start of the event
    double getTime() const { return (double)m_Time; }

    //! Get energy deposition
    //! @return energy deposition (MeV)
    double getEDep() const { return (double)m_EDep; }

    //! Increase energy deposition
    //! @param eDep Amount of additional energy deposition (MeV)
    void increaseEDep(double eDep) { m_EDep += eDep; }

    //! Adjust time (for background-overlay hit)
    //! @param delta time shift (ns)
    void shiftInTime(float delta) override { m_Time += delta; }

  private:

    //! detector-module identifier
    //! @sa BKLMStatus.h
    int m_ModuleID;

    //! event hit time (ns)
    float m_Time;

    //! energy deposition (MeV)
    float m_EDep;

    //! time-of-propagation (ns) from hit to sensor
    float m_PropagationTime;

    //! Needed to make the ROOT object storable
    ClassDefOverride(BKLMSimHit, 5);

  };

} // end of namespace Belle2
