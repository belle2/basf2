/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* This is modified version of the genFitterModule to also use reco hits form
 * pxd and svd at a hopfully not so far away point in the future this module
 * will be merged with genFitterModule */


#ifndef GENFITTER2MODULE_H
#define GENFITTER2MODULE_H

#include <framework/core/Module.h>
//genfit stuff
#include <GFKalman.h>
#include <GFDaf.h>

#include <fstream>


namespace Belle2 {


  class GenFitter2Module : public Module {

  public:

    /** Constructor .
     */
    GenFitter2Module();

    /** Destructor.
     */
    virtual ~GenFitter2Module();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    virtual void initialize();

    /** Called when entering a new run.
     */
    virtual void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    virtual void event();

    /** This method is called if the current run ends.
     */
    virtual void endRun();

    /** This method is called at the end of the event processing.
     */
    virtual void terminate();

  protected:

    std::ofstream dataOut;
    int m_failedFitCounter;
    int m_fitCounter;
    int m_notPerfectCounter;
    int m_largeAngleCounter;
    //steering file switches
    bool m_filter;
    int m_nLayerWithHit;
    double m_angleCut;

    //genfit fitter objects and options for them
    GFDaf m_daf;
    double m_probCut;
    GFKalman m_kalmanFilter;
    bool m_useDaf; //Determines if normal kalman filter or daf is used
    int m_nGFIter; // number of iterations for the normal kalman filter. One iteration is forward + backward
    double m_blowUpFactor; // when filter directions changes the filter covariance matrix is multipled with this number
    // steering file switches to control the material effects in genfit
    bool m_energyLossBetheBloch;
    bool m_noiseBetheBloch;
    bool m_noiseCoulomb;
    bool m_energyLossBrems;
    bool m_noiseBrems;
    bool m_noEffects;
    std::string m_mscModel;
    std::string m_hitType;
    int m_hitTypeId;
    std::vector<double> m_dafTemperatures;
    int m_smoothing;
    bool m_uselrAmbiInfo;


  };
} // end namespace Belle2


#endif /* GENFITTER2MODULE_H */
