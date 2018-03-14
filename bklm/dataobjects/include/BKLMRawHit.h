/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMRAWHIT_H
#define BKLMRAWHIT_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  //! Store one BKLM strip hit as a ROOT object
  class BKLMRawHit : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMRawHit();

    //! Constructor with initial values
    BKLMRawHit(unsigned int, int, int, int, int, int);

    //! Copy constructor
    BKLMRawHit(const BKLMRawHit&);

    //! Assignment operator
    BKLMRawHit& operator=(const BKLMRawHit&);

    //! Destructor
    virtual ~BKLMRawHit() {}

    //! returns status word
    unsigned int getStatus() const { return m_Status; }

    //! returns FEE crate number of the hit
    int getCrate() const { return m_Crate; }

    //! returns FEE slot number of the hit
    int getSlot() const { return m_Slot; }

    //! returns FEE channel number of the hit
    int getChannel() const { return m_Channel; }

    //! returns FEE time of the hit (in hardware units)
    int getTDC() const { return m_TDC; }

    //! returns FEE pulse height of the hit (in hardware units)
    int getADC() const { return m_ADC; }

    //! determines if two BKLMRawHits are equal based on location only
    bool match(const BKLMRawHit*) const;

    //! sets status word
    void setStatus(int status) { m_Status = status; }

    //! sets FEE crate of the hit
    void setCrate(int crate) { m_Crate = crate; }

    //! sets FEE slot of the hit
    void setSlot(int slot) { m_Slot = slot; }

    //! sets FEE channel of the hit
    void setChannel(int channel) { m_Channel = channel; }

    //! sets FEE time and pulse height of the hit (in hardware units)
    void setTDCADC(int tdc, int adc) { m_TDC = tdc; m_ADC = adc; }

  private:

    //! status word
    unsigned int m_Status;

    //! FEE crate number of the hit
    int m_Crate;

    //! FEE slot number of the hit
    int m_Slot;

    //! FEE channel number of the hit
    int m_Channel;

    //! FEE time of the hit (uncalibrated, in hardware units)
    int m_TDC;

    //! FEE pulse height of the hit (uncalibrated, in hardware units)
    int m_ADC;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMRawHit, 2)

  };

} // end of namespace Belle2

#endif //BKLMRAWHIT_H
