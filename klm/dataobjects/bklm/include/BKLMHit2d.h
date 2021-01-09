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

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/bklm/BKLMHit1d.h>
#include <klm/dataobjects/bklm/BKLMStatus.h>

/* Belle 2 headers. */
#include <framework/datastore/RelationsObject.h>

/* ROOT headers. */
#include <TVector3.h>

/* CLHEP headers. */
#include <CLHEP/Vector/ThreeVector.h>

namespace Belle2 {

  //! Store one BKLM strip hit as a ROOT object
  class BKLMHit2d : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMHit2d();

    //! Constructor with initial values
    //! @param hitPhi reconstructed BKLMHit1d along the phi-measuring direction
    //! @param hitZ reconstructed BKLMHit1d along the z-measuring direction
    //! @param globalPos reconstructed position in global coordinates
    //! @param time reconstructed time of the hit
    BKLMHit2d(const BKLMHit1d* hitPhi, const BKLMHit1d* hitZ, const CLHEP::Hep3Vector& globalPos, double time);

    //! Copy constructor
    BKLMHit2d(const BKLMHit2d&);

    //! Assignment operator
    BKLMHit2d& operator=(const BKLMHit2d&);

    //! Destructor
    ~BKLMHit2d()
    {
    }

    //! Determine whether this 2D hit is in RPC or scintillator
    //! @return whether this 2D hit is in RPC (true) or scintillator (false)
    bool inRPC() const
    {
      return getLayer() >= BKLMElementNumbers::c_FirstRPCLayer;
    }

    //! Get section number
    //! @return section number (1=forward or 0=backward) of this 2D hit.
    int getSection() const
    {
      return BKLMElementNumbers::getSectionByModule(m_ModuleID);
    }

    //! Get sector number
    //! @return sector number (1..8) of this 2D hit
    int getSector() const
    {
      return BKLMElementNumbers::getSectorByModule(m_ModuleID);
    }

    //! Get layer number
    //! @return layer number (1..15) of this 2D hit
    int getLayer() const
    {
      return BKLMElementNumbers::getLayerByModule(m_ModuleID);
    }

    //! Get lowest phi-measuring strip number
    //! @return lowest phi-measuring strip number of this 2D hit
    int getPhiStripMin() const
    {
      return BKLMElementNumbers::getStripByModule(m_ModuleID);
    }

    //! Get highest phi-measuring strip number
    //! @return highest phi-measuring strip number of this 2D hit
    int getPhiStripMax() const
    {
      return BKLMStatus::getMaximalStrip(m_ModuleID);
    }

    //! Get average phi-measuring strip number
    //! @return average phi-measuring strip number of this 2D hit
    double getPhiStripAve() const
    {
      return 0.5 * (getPhiStripMin() + getPhiStripMax());
    }

    //! Get lowest z-measuring strip number
    //! @return lowest z-measuring strip number of this 2D hit
    int getZStripMin() const
    {
      return (((m_ZStrips & BKLM_ZSTRIP_MASK) >> BKLM_ZSTRIP_BIT) + 1);
    }

    //! Get highest z-measuring strip number
    //! @return highest z-measuring strip number of this 2D hit
    int getZStripMax() const
    {
      return (((m_ZStrips & BKLM_ZMAXSTRIP_MASK) >> BKLM_ZMAXSTRIP_BIT) + 1);
    }

    //! Get average z-measuring strip number
    //! @return average z-measuring strip number of this 2D hit
    double getZStripAve() const
    {
      return 0.5 * (getZStripMin() + getZStripMax());
    }

    //! Get detector-module identifier
    //! @return detector-module identifier
    //! @sa BKLMStatus.h
    int getModuleID() const
    {
      return m_ModuleID;
    }

    //! Determine whether this 2D hit is outside the trigger-coincidence window
    //! @return whether this 2D hit is outside the trigger-coincidence window (true) or not (false)
    bool isOutOfTime()
    {
      return (m_ModuleID & BKLM_OUTOFTIME_MASK) != 0;
    }

    //! Determine whether this 2D hit is associated with a muid-extrapolated track
    //! @return whether this 2D hit is associated with a muid-extrapolated track (true) or not (false)
    bool isOnTrack()
    {
      return (m_ModuleID & BKLM_ONTRACK_MASK) != 0;
    }

    //! Determine whether this 2D hit is associated with a BKLM-stand-alone track
    //! @return whether this 2D hit is associated with a BKLM-stand-alone track (true) or not (false)
    bool isOnStaTrack()
    {
      return (m_ModuleID & BKLM_ONSTATRACK_MASK) != 0;
    }

    //! Get 3D hit position's x coordinate in global coordinates
    //! @return 3D hit position's x coordinate in global coordinates (cm)
    float getGlobalPositionX(void) const
    {
      return m_GlobalPosition[0];
    }

    //! Get 3D hit position's y coordinate in global coordinates
    //! @return 3D hit position's y coordinate in global coordinates (cm)
    float getGlobalPositionY(void) const
    {
      return m_GlobalPosition[1];
    }

    //! Get 3D hit position's z coordinate in global coordinates
    //! @return 3D hit position's z coordinate in global coordinates (cm)
    float getGlobalPositionZ(void) const
    {
      return m_GlobalPosition[2];
    }

    //! Get 3D hit position in global coordinates
    //! @return 3D hit position in global coordinates (cm)
    TVector3 getGlobalPosition(void) const
    {
      return TVector3(m_GlobalPosition[0], m_GlobalPosition[1], m_GlobalPosition[2]);
    }

    //! Get reconstructed hit time
    //! @return reconstructed hit time (ns), the average of the 2 projections
    float getTime() const
    {
      return m_Time;
    }

    //! Get reconstructed energy deposition
    //! @return reconstructed energy deposition (MeV), the sum of the 2 projections
    float getEnergyDeposit() const
    {
      return m_EnergyDeposit;
    }

    //! Determine whether the two BKLMHit2ds are in the same module
    //! @return whether the two BKLMHit2ds are in the same module (true) or not (false)
    bool match(const BKLMHit2d* h) const
    {
      return BKLMElementNumbers::hitsFromSameModule(m_ModuleID, h->getModuleID());
    }

    //! Set or clear the OutOfTime flag
    //! @param flag whether this hit is outside the trigger-coincidence window (true) or not (false)
    void isOutOfTime(bool flag)
    {
      if (flag)
        m_ModuleID |= BKLM_OUTOFTIME_MASK;
      else
        m_ModuleID &= ~BKLM_OUTOFTIME_MASK;
    }

    //! Set or clear the OnTrack flag
    //! @param flag whether this hit is associated with a muid-extrapolated track (true) or not (false)
    void isOnTrack(bool flag)
    {
      if (flag)
        m_ModuleID |= BKLM_ONTRACK_MASK;
      else
        m_ModuleID &= ~BKLM_ONTRACK_MASK;
    }

    //! Set or clear the On-BKLM-stand-alone-track flag
    //! @param flag whether this hit is associated with a stand-alone BKLM track (true) or not (false)
    void isOnStaTrack(bool flag)
    {
      if (flag)
        m_ModuleID |= BKLM_ONSTATRACK_MASK;
      else
        m_ModuleID &= ~BKLM_ONSTATRACK_MASK;
    }

  private:

    //! BKLMHit2d Zstrips bit position for strip-1 [0..47]
    static constexpr int BKLM_ZSTRIP_BIT = 0;

    //! BKLMHit2d Zstrips bit position for maxStrip-1 [0..47]
    static constexpr int BKLM_ZMAXSTRIP_BIT = 6;

    //! BKLMHit2d Zstrips bit mask for strip-1 [0..47]
    static constexpr int BKLM_ZSTRIP_MASK = (63 << BKLM_ZSTRIP_BIT);

    //! BKLMHit2d Zstrips bit mask for maxStrip-1 [0..47]
    static constexpr int BKLM_ZMAXSTRIP_MASK = (63 << BKLM_ZMAXSTRIP_BIT);

    //! detector-module identifier
    //! @sa BKLMStatus.h
    int m_ModuleID;

    //! z-measuring strip numbers of the 2D hit
    //! @sa BKLMStatus.h
    int m_ZStrips;

    //! position of the 2D hit in global coordinates (cm)
    float m_GlobalPosition[3];

    //! reconstructed hit time relative to trigger (ns)
    float m_Time;

    //! reconstructed pulse height (MeV)
    float m_EnergyDeposit;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMHit2d, 6)

  };

} // end of namespace Belle2
