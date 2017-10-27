/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Lukin, Alexander Bobrov, Guglielmo De Nardo                                             *
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

    virtual void terminate();

    /** Method is called for each event. */
    virtual void event();

  protected:
    /** output root file name (given as Module parameter) */
    std::string m_dataOutFileName, m_dspArrayName, m_digiArrayName;

    /** Root tree and file for saving the output */
    TTree* m_tree;
    TFile* m_rootFile;

    double m_energy;

    int m_nevt;
    int m_nhits;

    int m_cellID[8736];
    int m_theta[8736];
    int m_phi[8736];
    int m_DspHit[8736][31];
    double m_hitE[8736];
    double m_hitTime[8736];
    int m_DigiTime[8736];
    double m_DeltaT[8736];
  };

} // end namespace Belle2

#endif
