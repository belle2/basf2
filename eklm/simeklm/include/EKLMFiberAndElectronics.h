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

#include <eklm/eklmutils/EKLMutils.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/EKLMStripHit.h>


#include "TTree.h"
#include "TH1D.h"
#include "TFitResult.h"
#include "TF1.h"
namespace Belle2 {

  //! Digitize EKLMSimHits  to get EKLM StripHits
  class EKLMFiberAndElectronics {

  public:


    /**
     * Constructor
     */
    EKLMFiberAndElectronics(std::pair<const G4VPhysicalVolume *, std::vector<EKLMSimHit*> >);

    /**
     * Destructor
     */
    ~EKLMFiberAndElectronics();

    /**
     * all work are did here
     */
    void processEntry();

    /**
     * returns pointer to  TFitResult object
     */
    TFitResultPtr getFitResultsPtr() const;


    /**
     * returns i'th parameter of the fit
     */
    double getFitResults(int i) const;

    /**
     * returns status of the fit
     */
    int getFitStatus() const;

  private:

    /**
     * Pointer to histogramm with forward hits
     */
    TH1D * m_digitizedAmplitudeDirect;

    /**
     * Pointer to histogramm with backward hits
     */
    TH1D * m_digitizedAmplitudeReflected;

    /**
     * Pointer to resulting histogramm
     */
    TH1D * m_digitizedAmplitude;


    /**
     * Pointer to fit function
     */
    TF1 * m_fitFunction;


    /**
     * Pointer to fit parameters
     */
    TFitResultPtr m_fitResultsPtr;


    /**
     * Pointer to vector if the SimHits
     */
    std::vector<EKLMSimHit*> m_vectorHits;


    /**
     * Distance from the hitpoint to  SiPM  for the (forward, backward) photons  (no account for the angle)
     */
    std::pair<double, double> m_hitDist;


    /**
     * name of the strip
     */
    const std::string * m_stripName ;


    /**
     * Filename  prefix for files saved
     * Each file containes digitizedAmplitudeDirect  digitizedAmplitudeReflected and digitizedAmplitude histograms as well as fitFunction with fit results
     * Empty string means no file should be saved
     */
    std::string m_outputFilename;

    /**
     * number of ADC digitization steps
     * should be accessible via XML
     */
    int m_nTimeDigitizationSteps;

    /**
     * lightspeed in fiber
     */
    double m_lightSpeed;

    /**
     * attenuation length in fiber
     */
    double m_attenuationLength;

    /**
     * expCoefficint
     */
    double m_expCoefficient;


    /**
     *  minimal values of cos(Theta), (corresponds to maximal Theta)
     *  allowing light transmission through fiber
     *  should be accessible via XML
     */
    double  m_minCosTheta;

    /**
     * number of p.e. emitted in fiber per 1 MeV  --> to be tuned
     */
    double m_nPEperMeV;


    /**
     * ADC digitization step
     */
    int m_timeDigitizationStep;

    /**
     * mirror reflective index
     */
    double m_mirrorReflectiveIndex;

    /**
     * meanSiPMNoise
     */
    double m_meanSiPMNoise;

    /**
     * Deexcitation time for scintillator
     * unit=ns
     */
    double m_scintillatorDeExcitationTime;


    /**
     * Deexcitation time for a fiber
     * unit=ns
     */
    double m_fiberDeExcitationTime;

    /**
     * speed of the first photoelectoron
     * used to calculate distance from SiPm to the Hit using time delay
     */
    double  m_firstPhotonlightSpeed;

    // -------------------------- methods

    /**
     * converts time to the histogramm (TDC)
     * first argument is vector of hits
     * second is the pointer to the histogramm
     */
    void timesToShape(const std::vector <double>  &  , TH1D *);

    /**
     * calculates hit time,
     * first argument stands for # of p.e.
     * the second indicates if the hit is direct or reflected
     */
    std::vector<double>  hitTimes(int , bool isReflected = false) ;


    /**
     * returns delay depending on the distance to the hit
     * argument is the distance in cm
     */
    double lightPropagationTime(double);


    /**
     * calculates 'distances' to the direct and mirrored hits
     */
    void lightPropagationDistance(EKLMSimHit *);



    /**
     * Reflects time-shape of 1p.e. signal
     * Amplitude should be 1, exp tail  defined by 1 parameter
     * argument is a time
    */
    double signalShape(double);

    /**
     * Adds random noise to the signal (amplitude-dependend)
     */
    void addRandomSiPMNoise();

    /**
     *  Amplitude attenuation with a distance
     *  f(l)=distanceAttenuation(l)*f(0)
     */
    double distanceAttenuation(double);

  };

  double  EKLMSignalShapeFitFunction(double*, double*);

} // end of namespace Belle2

#endif
