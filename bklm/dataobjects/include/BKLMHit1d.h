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

#include <TVector3.h>
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

    //! Constructor from a vector of BKLMDigits
    BKLMHit1d(const std::vector<Belle2::BKLMDigit*>&);

    //! Constructor from a real RPC hit *DIVOT* not ready
    BKLMHit1d(int);

    //! Copy constructor
    BKLMHit1d(const BKLMHit1d&);

    //! Destructor
    virtual ~BKLMHit1d() {}

    //! returns status word of this 1D hit
    unsigned int getStatus() const { return m_Status; }

    //! returns flag whether this 1D hit is in RPC (true) or scintillator (false)
    bool isInRPC() const { return ((m_Status & STATUS_INRPC) != 0); }

    //! returns end (TRUE=forward or FALSE=backward) of this 1D hit
    bool isForward() const { return m_IsForward; }

    //! returns sector number (1..8) of this 1D hit
    int getSector() const { return m_Sector; }

    //! returns layer number (1..15) of this 1D hit
    int getLayer() const { return m_Layer; }

    //! returns layer number (1..15) of this 1D hit
    bool isPhiReadout() const { return m_IsPhiReadout; }

    //! returns lowest strip number of this 1D hit
    int getStripMin() const { return m_StripMin; }

    //! returns lowest strip number of this 1D hit
    int getStripMax() const { return m_StripMax; }

    //! returns average strip number of this 1D hit
    double getStripAve() const { return m_StripAve; }

    //! returns average strip uncertainty of this 1D hit
    double getStripErr() const { return m_StripErr; }

    //! returns number of strips of this 1D hit
    int getStripCount() const { return m_StripCount; }

    //! returns detector-module identifier
    int getModuleID() const { return m_ModuleID; }

    //! returns reconstructed hit time
    double getTime() const { return m_Time; }

    //! returns energy deposition
    double getEDep() const { return m_EDep; }

    //! sets status word
    void setStatus(int status) { m_Status = status; }

    //! sets some status bit(s)
    void setStatusBits(unsigned int status) { m_Status |= status; }

    //! clears status word (all bits)
    void clearStatus(void) { m_Status = 0; }

    //! clears some status bit(s)
    void clearStatusBits(unsigned int status) { m_Status &= (~status); }

  private:

    //! status word
    int m_Status;

    //! axial end (true=forward or false=backward) of the 1D hit
    bool m_IsForward;

    //! sector number (1..8) of the 1D hit
    int m_Sector;

    //! layer number (1..15) of the 1D hit
    int m_Layer;

    //! phi-sensitive (true) or z-sensitive (false) strip(s)
    bool m_IsPhiReadout;

    //! unique detector-module identifier
    int m_ModuleID;

    //! lowest strip number of the 1D hit
    int m_StripMin;

    //! highest strip number of the 1D hit
    int m_StripMax;

    //! number of strip of the 1D hit
    int m_StripCount;

    //! average strip number of the 1D hit
    double m_StripAve;

    //! uncertainty in the average strip number of the 1D hit
    double m_StripErr;

    //! reconstructed hit time (ns)
    double m_Time;

    //! reconstructed pulse height (MeV)
    double m_EDep;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMHit1d, 1)

  };

} // end of namespace Belle2

#endif //BKLMHIT1D_H
