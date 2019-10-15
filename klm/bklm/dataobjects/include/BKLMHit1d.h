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
#include <klm/bklm/dataobjects/BKLMElementNumbers.h>
#include <klm/bklm/dataobjects/BKLMStatus.h>

/* Belle 2 headers. */
#include <framework/datastore/RelationsObject.h>

/* C++ headers. */
#include <vector>

namespace Belle2 {

  class BKLMDigit;

  //! Store one reconstructed BKLM 1D hit as a ROOT object
  class BKLMHit1d : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMHit1d();

    //! Constructor with initial values
    //! @param digits vector of contiguous BKLMDigits
    explicit BKLMHit1d(const std::vector<Belle2::BKLMDigit*>& digits);

    //! Copy constructor
    BKLMHit1d(const BKLMHit1d&);

    //! Assignment operator
    BKLMHit1d& operator=(const BKLMHit1d&);

    //! Destructor
    virtual ~BKLMHit1d() {}

    //! Determine whether this 1D hit is in RPC or scintillator
    //! @return whether this 1D hit is in RPC (true) or scintillator (false)
    bool inRPC() const
    {
      return getLayer() >= BKLMElementNumbers::c_FirstRPCLayer;
    }

    //! Get section number
    //! @return section number (1=forward or 0=backward) of this 1D hit
    int getSection() const
    {
      return BKLMElementNumbers::getSectionByModule(m_ModuleID);
    }

    //! Get sector number
    //! @return sector number (1..8) of this 1D hit
    int getSector() const
    {
      return BKLMElementNumbers::getSectorByModule(m_ModuleID);
    }

    //! Get layer number
    //! @return layer number (1..15) of this 1D hit
    int getLayer() const
    {
      return BKLMElementNumbers::getLayerByModule(m_ModuleID);
    }

    //! Get plane number.
    //! @return Plane number (0=z, 1=phi).
    bool getPlane() const
    {
      return BKLMElementNumbers::getPlaneByModule(m_ModuleID);
    }

    //! Get readout coordinate
    //! @return readout coordinate of this 1D hit
    bool isPhiReadout() const
    {
      return BKLMElementNumbers::getPlaneByModule(m_ModuleID) ==
             BKLMElementNumbers::c_PhiPlane;
    }

    //! Get lowest strip number of this 1D hit
    //! @return lowest strip number of this 1D hit
    int getStripMin() const
    {
      return BKLMElementNumbers::getStripByModule(m_ModuleID);
    }

    //! Get highest strip number of this 1D hit
    //! @return highest strip number of this 1D hit
    int getStripMax() const
    {
      return BKLMStatus::getMaximalStrip(m_ModuleID);
    }

    //! Get average strip number
    //! @return average strip number of this 1D hit
    double getStripAve() const
    {
      return 0.5 * (getStripMin() + getStripMax());
    }

    //! Get detector-module identifier
    //! @return detector-module identifier
    //! @sa BKLMStatus.h
    int getModuleID() const { return m_ModuleID; }

    //! Get reconstructed hit time
    //! @return reconstructed hit time (ns)
    float getTime() const { return m_Time; }

    //! Get energy deposition
    //! @return energy deposition (MeV)
    float getEDep() const { return m_EDep; }

  private:

    //! detector-module identifier
    //! @sa BKLMStatus.h
    int m_ModuleID;

    //! reconstructed hit time (ns)
    float m_Time;

    //! reconstructed pulse height (MeV)
    float m_EDep;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMHit1d, 3)

  };

} // end of namespace Belle2
