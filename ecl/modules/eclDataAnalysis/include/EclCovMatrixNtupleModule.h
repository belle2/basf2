/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Lukin, Alexander Bobrov, Guglielmo De Nardo        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCOVMATRIXNTUPLEMODULE_H
#define ECLCOVMATRIXNTUPLEMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <TTree.h>
#include <TFile.h>

namespace Belle2 {

  class EclCovMatrixNtupleModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    EclCovMatrixNtupleModule();

    /** Destructor. */
    virtual ~EclCovMatrixNtupleModule() {}

    /** Initializes the module. */
    virtual void initialize();

    /** Terminates the module. */
    virtual void terminate();

    /** Method is called for each event. */
    virtual void event();

  protected:
    /** output root file name (given as Module parameter) */
    std::string m_dataOutFileName,
        /** eclDSPs array name */
        m_dspArrayName,
        /** eclDigit array name */
        m_digiArrayName;

    /** Root tree name */
    TTree* m_tree;
    /** Root file for saving the output */
    TFile* m_rootFile;

    double m_energy;
    /** Event number */
    int m_nevt;
    /** Number of eclHits */
    int m_nhits;

    /** Crystal ID */
    int m_cellID[8736];
    /** Crystal Theta ID */
    int m_theta[8736];
    /** Crystal Phi ID */
    int m_phi[8736];
    /** eclDsp sample Array */
    int m_DspHit[8736][31];
    /** eclHit Energy */
    double m_hitE[8736];
    /** eclHit Time */
    double m_hitTime[8736];
    /** eclDigit Time */
    int m_DigiTime[8736];
    /** eclTrig Time */
    double m_DeltaT[8736];
  };

} // end namespace Belle2

#endif
