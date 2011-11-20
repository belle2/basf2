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
    EKLMFiberAndElectronics(std::pair<G4VPhysicalVolume *, std::vector<EKLMSimHit*> >);

    /**
     * Destructor
     */
    ~EKLMFiberAndElectronics();

    void processEntry();
    void setRootOutput(TTree * sel);

    double getFitResults(int i);

    int getFitStatus();

  private:




    std::vector <double> hitTimesVectorForward;
    std::vector <double> hitTimesVectorBackward;

    TH1D * digitizedAmplitudeDirect;
    TH1D * digitizedAmplitudeReflected;
    TH1D * digitizedAmplitude;

    TF1 * fitFunction;

    TFitResultPtr fitResultsPtr;

    std::vector<EKLMSimHit*> vectorHits;

    double forwardHitDist;
    double backwardHitDist;

    const std::string * stripName ;

    void timesToShape(std::vector <double> * , TH1D *);

    void hitTimes(int , bool isReflected = false) ;


    //! returns delay depending on the distance to the hit
    double lightPropagationTime(double);

    //! calculates 'distances' to the direct and mirrored hits
    void lightPropagationDistance(double &firstHitDist,
                                  double &secondHitDist, EKLMSimHit *sh);


    //! Filename  prefix for files saved
    //! Each file containes digitizedAmplitudeDirect  digitizedAmplitudeReflected and digitizedAmplitude histograms as well as fitFunction with fit results
    //! Empty string means no file should be saved
    std::string outputFilename;

    //! number of ADC digitization steps
    //! should be accessible via XML
    int nTimeDigitizationSteps;

    //! lightspeed in fiber
    double lightSpeed;

    //! attenuation length in fiber
    double attenuationLength;

    //! expCoefficint
    double expCoefficient;

    //! ADC digitization step
    //! should be accessible via XML
    int timeDigitizationStep;

    //! mirror reflective index
    //! should be accessible via XML
    double mirrorReflectiveIndex;

    //! meanSiPMNoise
    double meanSiPMNoise;


    //! minimal values of cos(Theta), (corresponds to maximal Theta)
    //! allowing light transmission through fiber
    //! should be accessible via XML
    double  minCosTheta;

    //! number of p.e. emitted in fiber per 1 MeV  --> to be tuned
    //! should be accessible via XML
    double nPEperMeV;


    //! self-explanatory
    //! should be accessible via XML
    double scintillatorDeExcitationTime; //ns
    double fiberDeExcitationTime; //ns

    //! Reflects time-shape of 1p.e. signal
    //! Amplitude should be 1, exp tail  defined by 1 parameter
    double signalShape(double);

    //! Adds random noise to the signal (amplitude-dependend)
    void addRandomSiPMNoise();

    //! distance amplitude attenuation
    //! f(l)=distanceAttenuation(l)*f(0)
    double distanceAttenuation(double);

  };

  double  EKLMSignalShapeFitFunction(double*, double*);

} // end of namespace Belle2

#endif
