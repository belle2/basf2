/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/bklm/BKLMStatus.h>

/* Belle 2 headers. */
#include <simulation/dataobjects/SimHitBase.h>

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
    ~BKLMSimHit()
    {
    }

    //! Copy constructor
    BKLMSimHit(const BKLMSimHit&);

    //! Assignment operator
    BKLMSimHit& operator=(const BKLMSimHit&);

    //! Determine whether this hit is in an RPC or scintillator
    //! @return whether this hit is in an RPC (true) or scintillator (false)
    bool inRPC() const
    {
      return getLayer() >= BKLMElementNumbers::c_FirstRPCLayer;
    }

    //! Get section number
    //! @return section number (1=forward or 0=backward) of this hit
    int getSection() const
    {
      return BKLMElementNumbers::getSectionByModule(m_ModuleID);
    }

    //! Get sector number
    //! @return sector number of this hit
    int getSector() const
    {
      return BKLMElementNumbers::getSectorByModule(m_ModuleID);
    }

    //! Get layer number
    //! @return layer number of this hit
    int getLayer() const
    {
      return BKLMElementNumbers::getLayerByModule(m_ModuleID);
    }

    //! Get plane number.
    //! @return Plane number (0=z, 1=phi).
    int getPlane() const
    {
      return BKLMElementNumbers::getPlaneByModule(m_ModuleID);
    }

    //! Get readout coordinate
    //! @return readout coordinate (TRUE=phi, FALSE=z) of this hit
    bool isPhiReadout() const
    {
      return BKLMElementNumbers::getPlaneByModule(m_ModuleID) ==
             BKLMElementNumbers::c_PhiPlane;
    }

    //! Get strip number of this hit
    //! @return readout strip number of this hit (assuming one strip per hit)
    int getStrip() const
    {
      return BKLMElementNumbers::getStripByModule(m_ModuleID);
    }

    //! Get lowest readout strip number of a contiguous set
    //! @return lowest readout strip number of this hit (assuming a contiguous set of RPC strips)
    int getStripMin() const
    {
      return BKLMElementNumbers::getStripByModule(m_ModuleID);
    }

    //! Get highest readout strip number of a contiguous set
    //! @return highest readout strip number of this hit (assuming a contiguous set of RPC strips)
    int getStripMax() const
    {
      return BKLMStatus::getMaximalStrip(m_ModuleID);
    }

    //! Get detector-module identifier
    //! @return detector-module identifier
    int getModuleID() const
    {
      return m_ModuleID;
    }

    //! Get signal propagation time alont strip
    //! @return time-of-propagation (ns) from hit to sensor
    float getPropagationTime() const
    {
      return m_PropagationTime;
    }

    //! Get the hit time (implementation of base class function
    //! @return the hit time (ns) since start of the event
    float getGlobalTime() const override
    {
      return m_Time;
    }

    //! Get the hit time
    //! @return the hit time (ns) since start of the event
    double getTime() const
    {
      return (double)m_Time;
    }

    //! Get energy deposition
    //! @return energy deposition (MeV)
    double getEnergyDeposit() const
    {
      return (double)m_EDep;
    }

    //! Adjust time (for background-overlay hit)
    //! @param delta time shift (ns)
    void shiftInTime(float delta) override
    {
      m_Time += delta;
    }

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
