/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGISTUDYMODULE_H
#define ECLDIGISTUDYMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <TTree.h>
#include <TFile.h>

namespace Belle2 {

  class ECLDigiStudyModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    ECLDigiStudyModule();

    /** Destructor. */
    virtual ~ECLDigiStudyModule() {}

    /** Initializes the module. */
    virtual void initialize();

    /** Terminates the module. */
    virtual void terminate();

    /** Method is called for each event. */
    virtual void event();

  protected:
    /** output root file name (given as Module parameter) */
    std::string m_dataOutFileName,
        /**name of input ECLDsp Array*/
        m_dspArrayName1,
        /**name of input Pure ECLDigit Array*/
        m_digiArrayName1,
        /**name of input Pure CsI ECLDsp Array*/
        m_dspArrayName2,
        /**name of input Pure CsI ECLDigit Array*/
        m_digiArrayName2;

    /** Root tree name */
    TTree* m_tree;
    /** Root file name for saving the output */
    TFile* m_rootFile;

    int m_nhits = 8736;
    /** Number of eclHits */
    int m_neclhits;
    /** eclHit Crystal ID */
    int m_cellId[8736];
    /** eclHit Non-bkg energy deposition */
    double m_energy[8736];
    /** eclHit Energy deposition */
    double m_allenergy[8736];
    /** eclHit Time average */
    double m_time[8736];
    /** eclThetaID*/
    int m_theta[8736];
    /** eclPhiID*/
    int m_phi[8736];

    /** Waveform sample */
    int m_DspHit1[8736][31];
    /** PureCsI Waveform sample */
    int m_DspHit2[8736][31];
    /** Baseline sample */
    int m_baseline1[8736][16];
    /** PureCsI baseline sample */
    int  m_baseline2[8736][16];
    /** Baseline average */
    double m_baselineAvg1[8736];
    /** PureCsI baseline average */
    double m_baselineAvg2[8736];
    /** eclDigit Fit Max */
    int m_maxVal1[8736];
    /** PureCsI eclDigit Fit Max */
    int m_maxVal2[8736];
    /** eclDigit Fit Quality */
    int m_digiQual1[8736];
    /** PureCsI eclDigit Fit Quality */
    int m_digiQual2[8736];
    /** eclDigit Time */
    double m_digiTime1[8736];
    /** PureCsI eclDigit Time */
    double m_digiTime2[8736];
    /** eclDigit Energy */
    double m_digiE1[8736];
    /** PureCsI eclDigit Energy */
    double  m_digiE2[8736];
    /** eclTrigTime */
    double m_trig1;
    /** PureCsI eclTrigTime */
    double m_trig2;

  };

} // end namespace Belle2

#endif
