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
   * Creates an output root file which contains info from ECL clusters.
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
    virtual void initialize() override;

    /**  */
    virtual void event() override;

    /**  */
    virtual void terminate() override;

  private:

    /** Name of ROOT output file. */

    std::string m_fileName;

    /** Name of tree. */

    std::string m_treeName;

    /** ROOT output file. */

    TFile* m_file;

    /** ROOT tree for output. */

    TTree* m_tree;

    /** ConnectedRegionID */
    int m_CRID;

    /** ClusterID */
    int m_clusterID;

    /** Hypothesis ID */
    int m_hypothesisID;

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

    /** E1/E9 */

    float m_E1oE9;

    /** E9/E21 */

    float m_E9oE21;

    /** Highest Energy stored in a Crystal in a Cluster (GeV) */

    float m_HE;

    /** Absolute value of Zernike moment 40*/

    float m_absZernikeMoment40;

    /** Absolute value of Zernike moment 51*/

    float m_absZernikeMoment51;

    /** Result of MVA using zernike moments */

    float m_zernikeMVA;

    /** Second moment shower shape variable */

    float m_secondMoment;

    /** Lateral energy (LAT) shower shape variable */

    float m_LAT;

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
