/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo, Benjamin Oberhof                     *
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
    std::string m_dataOutFileName,  /**< Root file name for saving the output */
        m_dspArrayName1, /**< Name of first DSP array */
        m_digiArrayName1,  /**< Name of first digit array */
        m_dspArrayName2,  /**< Name of second DSP array */
        m_digiArrayName2;  /**< Name of second digit array */

    TTree* m_tree;  /**< Root tree for saving the output */
    TFile* m_rootFile;  /**< Root file for saving the output */

    int m_nhits = 8736; /**< Maximum number of hits */
    int m_neclhits; /**< Actual number of hits */
    int m_cellId[8736]; /**< Array of cellIDs */
    double m_energy[8736], /**< Array of deposited MC energy */
           m_allenergy[8736]; /**< Array of deposited energy */
    double m_time[8736]; /**< Array of digit time */
    int m_theta[8736]; /**< Array oh ThetaID */
    int m_phi[8736]; /**< Array of PhiID */

    int m_DspHit1[8736][31], /**< WF sampling points for first digit array */
        m_DspHit2[8736][31];  /**< WF sampling points for second digit array */
    int m_baseline1[8736][16],  /**< Baseline sampling points for first digit array */
        m_baseline2[8736][16];  /**< Baseline sampling points for second digit array */
    double m_baselineAvg1[8736], /**< Baseline energy for first digit array */
           m_baselineAvg2[8736]; /**< Baseline energy for second digit array */
    int m_maxVal1[8736], /**< WF maximum for first digit array */
        m_maxVal2[8736]; /**< WF maximum for second digit array */
    int m_digiQual1[8736], /**< Digit quality for first digit array */
        m_digiQual2[8736]; /**< Digit quality for second digit array */
    double m_digiTime1[8736], /**< Digit time for first digit array */
           m_digiTime2[8736] /**< Digit time for second digit array */;
    double m_digiE1[8736], /**< Deposited energy for first digit array */
           m_digiE2[8736]; /**< Deposited energy for second digit array */
    double m_trig1, /**< Trigger time for array 1 */
           m_trig2; /**< Trigger time for array 2 */

  };

} // end namespace Belle2

#endif
