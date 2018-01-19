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
#include <eklm/simulation/FPGAFitter.h>
#include <bklm/dbobjects/BKLMScinDigitizationParams.h>
#include <bklm/dbobjects/BKLMADCThreshold.h>
#include <framework/database/DBObjPtr.h>

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

  private:

    //! Digitize all BKLMSimHits
    void digitize(const std::map<int, std::vector<std::pair<int, BKLMSimHit*> > >&, StoreArray<BKLMDigit>&);

    /**
     * Digitize hit(s) in one scintillator strip with pulse-shape fit
     * @param[in] vHits Vector of BKLMSimHits
     * @param[out] fitParams Pulse-shape parameters from fit
     * @param[out] nPE Number of surviving photoelectrons
     * @return Fit status
     */
    void processEntry(std::vector<std::pair<int, BKLMSimHit*> > vHits, BKLMDigit* bklmDigit);

    /**
     * Calculate pulse(s) histogram at the MPPC end of the strip
     * @param[in] nPEsample Sampled number of photoelectrons
     * @param[in] timeShift Time of the SimHit
     * @param[in] isReflected Whether the hit is direct (false) or reflected (true)
     * @param[out] adcPulse Pulse-shape histogram
     * @return Number of surviving photoelectrons
     */
    int fillAmplitude(int nPEsample, double timeShift, bool isReflected, double dist, std::vector<double>& adcPulse);

    //! ADC sampling time (ns)
    double m_ADCSamplingTime;

    //! Number of ADC digitizations
    unsigned int m_nDigitizations;

    //! Mean number of photoelectrons per MeV of energy deposition
    int m_nPEperMeV;

    //! Minimum cos(theta) for total internal reflection
    double m_minCosTheta;

    //! Reflection fraction by mirrored end of optical fiber
    double m_mirrorReflectiveIndex;

    //! Scintillator de-excitation lifetime (microseconds)
    double m_scintillatorDeExcitationTime;

    //! Fiber de-escitation lifetime (microseconds)
    double m_fiberDeExcitationTime;

    //! Speed of internally-reflected light in fiber (cm/us)
    double m_fiberLightSpeed;

    //! Attenuation length in fiber (cm)
    double m_attenuationLength;

    //! Photoelectron attenuation frequency
    double m_PEAttenuationFreq;

    //! Mean noise level in MPPC
    int m_meanSiPMNoise;

    //! Time resolution (ns)
    double m_timeResolution;

    //! Flag to enable constant noise in MPPC
    bool m_enableConstBkg;

    //! FPGA's ADC range (# of channels for full range)
    int m_ADCRange;

    //! MPPC's gain
    double m_MPPCGain;

    //! ADC offset
    int m_ADCOffset;

    //! ADC thrshold
    double m_ADCThreshold;

    //! User parameter: Discriminator threshold (# of photoelectrons)
    double m_discriminatorThreshold;

    //! Scintillator digitization params read from database
    DBObjPtr<BKLMScinDigitizationParams> m_digitParams;

    //! MPPC gain, pedestal and threshold read from database
    DBObjPtr<BKLMADCThreshold> m_ADCParams;

    //! simHits StoreArray
    StoreArray<BKLMSimHit> simHits;

    //! digits StoreArray
    StoreArray<BKLMDigit> bklmDigits;
  };

} // end of namespace Belle2

#endif // BKLMDIGITIZERMODULE_H
