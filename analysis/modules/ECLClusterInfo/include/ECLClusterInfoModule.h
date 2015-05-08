/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCLUSTERINFOMODULE_H
#define ECLCLUSTERINFOMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <vector>

// root
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TMatrixFSym.h"
#include "TVector3.h"

namespace Belle2 {
  /**
   * Creates an output root file which contains info from ECL clusters. Used for b2bii conversion validation.
   *
   *    *
   */
  class ECLClusterInfoModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ECLClusterInfoModule();

    /**  */
    virtual void initialize();

    /**  */
    virtual void event();

    /**  */
    virtual void terminate();

  private:

    /** Name of ROOT output file. */

    std::string m_fileName;

    /** Name of tree. */

    std::string m_treeName;

    /** ROOT output file. */

    TFile* m_file;

    /** ROOT tree for output. */

    TTree* m_tree;

    /** Corrected Energy (GeV) */

    float m_E;

    /** Theta of Cluster (radian) */

    float m_Theta;

    /** Phi of Cluster (radian) */

    float m_Phi;

    /** R (in cm) */

    float m_R;

    /** Uncorrected Energy deposited (GeV) */

    float m_Edep;

    /** E9/E25 */

    float m_E9oE25;

    /** Highest Energy stored in a Crystal in a Cluster (GeV) */

    float m_HE;

    /** Number of Crystals in a Cluster */

    int m_NC;

    /** 3x3 Error matrix element: Energy */

    float m_Err00;

    /** 3x3 Error matrix element: (Phi, Energy) */

    float m_Err10;

    /** 3x3 Error matrix element: Phi */

    float m_Err11;

    /** 3x3 Error matrix element: (Theta, Energy) */

    float m_Err20;

    /** 3x3 Error matrix element: (Theta, Phi) */

    float m_Err21;

    /** 3x3 Error matrix element: Theta */

    float m_Err22;

    /** X component of the momentum of the corresponding MCParticle */

    float m_Truth_Px;

    /** Y component of the momentum of the corresponding MCParticle */

    float m_Truth_Py;

    /** Z component of the momentum of the corresponding MCParticle */

    float m_Truth_Pz;

    /** Energy of the momentum of the corresponding MCParticle */

    float m_Truth_E;

    /** PDG code of the momentum of the corresponding MCParticle */

    int m_PDG;
  };
}

#endif /* ECLCLUSTERINFOMODULE_H */
