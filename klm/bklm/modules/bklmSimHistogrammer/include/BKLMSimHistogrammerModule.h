/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMHit1d.h>
#include <klm/dataobjects/bklm/BKLMHit2d.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>

/* ROOT headers. */
#include <TFile.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TH2D.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  class BKLMSimHit;
  class KLMDigit;

  //! Convert BKLM raw simulation hits to digitizations
  class BKLMSimHistogrammerModule : public Module {

  public:

    //! Constructor
    BKLMSimHistogrammerModule();

    //! Destructor
    ~BKLMSimHistogrammerModule();

    //! Initialize at start of job
    void initialize() override;

    //! Do any needed actions at the start of a simulation run
    void beginRun() override;

    //! Digitize one event and write hits, digis, and relations into DataStore
    void event() override;

    //! Do any needed actions at the end of a simulation run
    void endRun() override;

    //! Terminate at the end of job
    void terminate() override;

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
    StoreArray<KLMDigit> digits;
    //! mcParticles StoreArray
    StoreArray<MCParticle> mcParticles;
    //! hits2D StoreArray
    StoreArray<BKLMHit2d> hits2D;
    //! hits1D StoreArray
    StoreArray<BKLMHit1d> hits1D;


  };

} // end of namespace Belle2
