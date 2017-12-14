/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMHIT1D_H
#define BKLMHIT1D_H

#include <framework/datastore/RelationsObject.h>
#include <bklm/dataobjects/BKLMStatus.h>

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
    bool inRPC() const { return ((m_ModuleID & BKLM_INRPC_MASK) != 0); }

    //! Get detector end
    //! @return detector end (TRUE=forward or FALSE=backward) of this 1D hit
    bool isForward() const { return ((m_ModuleID & BKLM_END_MASK) != 0); }

    //! Get sector number
    //! @return sector number (1..8) of this 1D hit
    int getSector() const { return (((m_ModuleID & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1); }

    //! Get layer number
    //! @return layer number (1..15) of this 1D hit
    int getLayer() const { return (((m_ModuleID & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1); }

    //! Get readout coordinate
    //! @return readout coordinate of this 1D hit
    bool isPhiReadout() const { return ((m_ModuleID & BKLM_PLANE_MASK) != 0); }

    //! Get lowest strip number of this 1D hit
    //! @return lowest strip number of this 1D hit
    int getStripMin() const { return (((m_ModuleID & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1); }

    //! Get highest strip number of this 1D hit
    //! @return highest strip number of this 1D hit
    int getStripMax() const { return (((m_ModuleID & BKLM_MAXSTRIP_MASK) >> BKLM_MAXSTRIP_BIT) + 1); }

    //! Get average strip number
    //! @return average strip number of this 1D hit
    double getStripAve() const
    {
      return 0.5 * ((((m_ModuleID & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1) +
                    (((m_ModuleID & BKLM_MAXSTRIP_MASK) >> BKLM_MAXSTRIP_BIT) + 1));
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

#endif //BKLMHIT1D_H
