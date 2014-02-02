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
    void digitize(std::map<int, std::vector<std::pair<int, BKLMSimHit*> > >, StoreArray<BKLMDigit>&);

    //! Digitize hit(s) in one scintillator strip
    enum EKLM::FPGAFitStatus processEntry(std::vector<std::pair<int, BKLMSimHit*> >);

    /**
     * Calculate StripHit times (at the end of the strip),
     * @param[in] Number of photoelectrons.
     * @param[in] Time of the SimHit.
     * @param[in] If the hit is direct or reflected.
     * @param[out] hist Histogram.
     * @return Vector of hit times.
     */
    void fillAmplitude(int nPE, double timeShift, bool isReflected, float* hist);

    /**
     * Reflect time-shape of 1p.e. signal
     * Amplitude should be 1, exp tail defined by 1 parameter
     * @param[in] t Time.
     * @return Signal shape.
     */
    double signalShape(double t);

    //! Add random noise to the signal (amplitude-dependent)
    void addRandomSiPMNoise();

    /**
     * Amplitude attenuation with a distance f(l)=distanceAttenuation(l)*f(0)
     * @param[in] dist Distance.
     * @return Amplitude attenuation.
     */
    double distanceAttenuation(double dist);

    //! Create digital signal from analog
    void simulateADC(int [], float []);

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
    double m_firstPhotonlightSpeed;

    //! Attenuation length in fiber (cm)
    double m_attenuationLength;

    //! Photoelectron attenuation frequency
    double m_PEAttenuationFreq;

    //! Mean noise level in MPPC
    int m_meanSiPMNoise;

    //! Flag to enable constant noise in MPPC
    bool m_enableConstBkg;

    //! FPGA fitter
    EKLM::FPGAFitter* m_fitter;

    //! FPGA fit status
    enum EKLM::FPGAFitStatus m_FPGAStat;

    //! FPGA fit results
    struct EKLM::FPGAFitParams m_FPGAParams;

    //! Number of photoelectrons (generated)
    int m_npe;

    //! Distance from the hitpoint to SiPM for the forward-moving photons
    double m_hitDistDirect;

    //! Distance from the hitpoint to SiPM for the backward-moving photons
    double m_hitDistReflected;

    //! User parameter: Discriminator threshold (# of photoelectrons)
    double m_discriminatorThreshold;

  };

} // end of namespace Belle2

#endif // BKLMDIGITIZERMODULE_H
