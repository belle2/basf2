/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMDIGITIZERMODULE_H
#define BKLMDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <bklm/dbobjects/BKLMADCThreshold.h>
#include <framework/database/DBObjPtr.h>
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>
#include <klm/dbobjects/KLMTimeConversion.h>
#include <klm/simulation/ScintillatorFirmware.h>

#include <map>

namespace Belle2 {

  class BKLMSimHit;
  class BKLMDigit;

  //! Convert BKLM raw simulation hits to digitizations
  class BKLMDigitizerModule : public Module {

  public:

    //! Constructor
    BKLMDigitizerModule();

    //! Destructor
    virtual ~BKLMDigitizerModule();

    //! Initialize at start of job
    virtual void initialize() override;

    //! Do any needed actions at the start of a simulation run
    virtual void beginRun() override;

    //! Digitize one event and write hits, digis, and relations into DataStore
    virtual void event() override;

    //! Do any needed actions at the end of a simulation run
    virtual void endRun() override;

    //! Terminate at the end of job
    virtual void terminate() override;

  protected:

  private:

    //! Digitize all BKLMSimHits
    void digitize(std::multimap<int, BKLMSimHit*>&, StoreArray<BKLMDigit>&);

    //! FPGA fitter.
    KLM::ScintillatorFirmware* m_Fitter;

    //! Scintillator digitization params read from database
    DBObjPtr<KLMScintillatorDigitizationParameters> m_digitParams;

    //! Time conversion.
    DBObjPtr<KLMTimeConversion> m_TimeConversion;

    //! simHits StoreArray
    StoreArray<BKLMSimHit> simHits;

    //! digits StoreArray
    StoreArray<BKLMDigit> bklmDigits;
  };

} // end of namespace Belle2

#endif // BKLMDIGITIZERMODULE_H
