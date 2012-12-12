/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Timofey Uglov                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMFIBERANDELECTRONICS_H
#define EKLMFIBERANDELECTRONICS_H

/* External headers. */
#include <TTree.h>
#include <TH1D.h>
#include <TFitResult.h>
#include <TF1.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/geometry/TransformData.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * Digitize EKLMSimHits to get EKLM StripHits.
     */
    class FiberAndElectronics {

    public:

      /**
       * Constructor.
       */
      FiberAndElectronics(std::pair <int, std::vector<EKLMSimHit*> >,
                          struct EKLM::TransformData* transf);

      /**
       * Destructor.
       */
      ~FiberAndElectronics();

      /**
       * Process.
       */
      void processEntry();

      /**
       * Get fit results.
       * @return Pointer to TFitResult object.
       */
      TFitResultPtr getFitResultsPtr() const;

      /**
       * Get fit parameter.
       * @return i'th parameter of the fit.
       */
      double getFitResults(int i) const;

      /**
       * Get fit status.
       * @return Status of the fit.
       */
      int getFitStatus() const;

    private:

      /** Transformation data. */
      struct EKLM::TransformData* m_transf;

      /** Pointer to histogram with forward hits. */
      TH1D* m_digitizedAmplitudeDirect;

      /** Pointer to histogram with backward hits. */
      TH1D* m_digitizedAmplitudeReflected;

      /** Pointer to resulting histogram. */
      TH1D* m_digitizedAmplitude;

      /** Pointer to fit function. */
      TF1* m_fitFunction;

      /** Pointer to fit parameters. */
      TFitResultPtr m_fitResultsPtr;

      /** Pointer to vector if the SimHits. */
      std::vector<EKLMSimHit*> m_vectorHits;

      /**
       * Distance from the hitpoint to SiPM for the (forward, backward) photons
       * (no account for the angle).
       */
      std::pair<double, double> m_hitDist;

      /** Name of the strip. */
      std::string m_stripName;

      /**
       * Filename prefix for files saved.
       * Each file containes digitizedAmplitudeDirect
       * digitizedAmplitudeReflected and digitizedAmplitude
       * histograms as well as fitFunction with fit results.
       * Empty string means no file should be saved.
       */
      std::string m_outputFilename;

      /**
       * Number of ADC digitization steps.
       * @details
       * Should be accessible via XML.
       */
      int m_nTimeDigitizationSteps;

      /** ADC digitization step. */
      int m_timeDigitizationStep;

      /** Stands for m_nTimeDigitizationSteps*m_timeDigitizationStep. */
      double m_histRange;

      /** Speed pf light in fiber. */
      double m_lightSpeed;

      /** Attenuation length in fiber. */
      double m_attenuationLength;

      /** Exponent coefficient. */
      double m_expCoefficient;

      /**
       * Minimal values of cos(Theta), (corresponds to maximal Theta)
       * allowing light transmission through fiber.
       * @details
       * Should be accessible via XML.
       */
      double  m_minCosTheta;

      /** Number of p.e. emitted in fiber per 1 MeV  --> to be tuned. */
      double m_nPEperMeV;

      /** Mirror reflective index. */
      double m_mirrorReflectiveIndex;

      /** Mean SiPM noise. */
      double m_meanSiPMNoise;

      /**
       * If non-zero used as a seed value for constant background fraction
       * in the fit.
       * If zero fix constant background fraction to zero.
       */
      double m_enableConstBkg;

      /** Deexcitation time for scintillator (unit=ns). */
      double m_scintillatorDeExcitationTime;


      /** Deexcitation time for fiber (unit=ns). */
      double m_fiberDeExcitationTime;

      /**
       * Speed of the first photoelectoron.
       * @details
       * Used to calculate distance from SiPm to the Hit using time delay.
       */
      double  m_firstPhotonlightSpeed;

      /**
       * Convert time to the histogram (TDC).
       * @param times Vector of hits.
       * @param shape Histogram.
       */
      void timesToShape(const std::vector <double> & times, TH1D* shape);

      /**
       * Calculate StripHit times (at the end of the strip),
       * @param[in] Number of photoelectrons.
       * @param[in] Time of the SimHit.
       * @param[in] If the hit is direct or reflected.
       * @return Vector of hit times.
       */
      std::vector<double> hitTimes(int nPE, double timeShift,
                                   bool isReflected = true);

      /**
       * Get delay depending on the distance to the hit.
       * @param[in] L Distance in cm.
       * @return Delay.
       */
      double lightPropagationTime(double L);

      /**
       * Calculate 'distances' to the direct and mirrored hits.
       * @param[in] sh EKLMSimHit.
       */
      void lightPropagationDistance(EKLMSimHit*);

      /**
       * Reflect time-shape of 1p.e. signal.
       * Amplitude should be 1, exp tail defined by 1 parameter
       * @param[in] t Time.
       * @return Signal shape.
       */
      double signalShape(double t);

      /**
       * Add random noise to the signal (amplitude-dependend).
       */
      void addRandomSiPMNoise();

      /**
       * Amplitude attenuation with a distance f(l)=distanceAttenuation(l)*f(0).
       * @param[in] dist Distance.
       * @return Amplitude attenuation.
       */
      double distanceAttenuation(double dist);

      /** Minimal hit time. */
      double m_min_time;

    };

    /**
     * Fitting function.
     * @param[in] x   1-dimensional coordinate.
     * @param[in] par Array of fit parameters.
     */
    double SignalShapeFitFunction(double* _x, double* par);

  }

}

#endif

