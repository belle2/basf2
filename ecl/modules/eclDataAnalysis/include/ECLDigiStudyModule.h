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

    virtual void terminate();

    /** Method is called for each event. */
    virtual void event();

  protected:
    /** output root file name (given as Module parameter) */
    std::string m_dataOutFileName, m_dspArrayName1, m_digiArrayName1,
        m_dspArrayName2, m_digiArrayName2;

    /** Root tree and file for saving the output */
    TTree* m_tree;
    TFile* m_rootFile;

    int m_nhits = 8736;
    int m_neclhits;
    int m_cellId[8736];
    double m_energy[8736], m_allenergy[8736];
    double m_time[8736];
    int m_theta[8736];
    int m_phi[8736];

    int m_DspHit1[8736][31], m_DspHit2[8736][31];
    int m_baseline1[8736][16], m_baseline2[8736][16];
    double m_baselineAvg1[8736], m_baselineAvg2[8736];
    int m_maxVal1[8736], m_maxVal2[8736];
    int m_digiQual1[8736], m_digiQual2[8736];
    double m_digiTime1[8736], m_digiTime2[8736];
    double m_digiE1[8736], m_digiE2[8736];
    double m_trig1, m_trig2;

  };

} // end namespace Belle2

#endif
