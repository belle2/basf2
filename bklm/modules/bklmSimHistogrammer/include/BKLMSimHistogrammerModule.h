/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSIMHistogrammerMODULE_H
#define BKLMSIMHistogrammerMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <bklm/dataobjects/BKLMHit1d.h>
#include <eklm/simulation/FPGAFitter.h>
#include "TFile.h"
#include "TH1D.h"
#include "TH1I.h"
#include "TH2D.h"
#include <string>
#include <map>
#include <iostream>

namespace Belle2 {

  class BKLMSimHit;
  class BKLMDigit;

  //! Convert BKLM raw simulation hits to digitizations
  class BKLMSimHistogrammerModule : public Module {

  public:

    //! Constructor
    BKLMSimHistogrammerModule();

    //! Destructor
    virtual ~BKLMSimHistogrammerModule();

    //! Initialize at start of job
    virtual void initialize();

    //! Do any needed actions at the start of a simulation run
    virtual void beginRun();

    //! Digitize one event and write hits, digis, and relations into DataStore
    virtual void event();

    //! Do any needed actions at the end of a simulation run
    virtual void endRun();

    //! Terminate at the end of job
    virtual void terminate();

  protected:


    //! hits per channel
    TH2D* m_hSimHitPerChannelLayer;

    //!keep track of events
    TH1D* m_hEvt;
    //! histogram for sim hit phi
    TH1D* m_hSimHitPhiRPC;
    //! bg source of 1D hits
    TH2D* m_bgSourcePerLayer;
    //! bg source of 2D hits
    TH2D* m_bgSourcePerLayer2D;
    //! bg source of 2D hits vs phi
    TH2D* m_bgSourceVsPhi;
    //! bg source of 2D hits vs theta
    TH2D* m_bgSourceVsTheta;
    //! bg source of 1D hits
    TH1D* m_bgSource;
    //! bg source of 2D hits
    TH1D* m_bgSource2D;



    //! histogram for sim hit phi
    TH1D* m_hSimHitPhiScinti;
    //! histogram for sim hit theta
    TH1D* m_hSimHitThetaRPC;
    //! histogram for sim hit theta scint
    TH1D* m_hSimHitThetaScinti;
    //! histogram for the layers of 1D hits
    TH1I* m_hSimHit_layer;
    //! histogram for the layers of 2D hits
    TH1I* m_hSimHit_layer2D;

    //! 2D histogram for sim hit phi/theta
    TH2D* m_hSimHitThetaPhiRPC;
    //! 2D histogram for sim hit phi/theta
    TH2D* m_hSimHitThetaPhiScinti;
  private:


    /**
     * Digitize hit(s) in one scintillator strip with pulse-shape fit
     * @param[in] vHits Vector of BKLMSimHits
     * @param[out] fitParams Pulse-shape parameters from fit
     * @param[out] nPE Number of surviving photoelectrons
     * @return Fit status
     */


    /**
     * Calculate pulse(s) histogram at the MPPC end of the strip
     * @param[in] nPEsample Sampled number of photoelectrons
     * @param[in] timeShift Time of the SimHit
     * @param[in] isReflected Whether the hit is direct (false) or reflected (true)
     * @param[out] adcPulse Pulse-shape histogram
     * @return Number of surviving photoelectrons
     */



    //! TFile to save everything
    TFile* m_file;
    //! filename for the root file
    std::string m_filename;
    //! time this simulation corrsponds to
    float m_realTime;
    //! weight for each event (inverse of the realTime)
    float m_weight;
    //! BKLMSimHit StoreArray
    StoreArray<BKLMSimHit> simHits;
    //! digits StoreArray
    StoreArray<BKLMDigit> digits;
    //! mcParticles StoreArray
    StoreArray<MCParticle> mcParticles;
    //! hits2D StoreArray
    StoreArray<BKLMHit2d> hits2D;
    //! hits1D StoreArray
    StoreArray<BKLMHit1d> hits1D;


  };

} // end of namespace Belle2

#endif // BKLMDIGITIZERMODULE_H
