/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMK0LRECONSTRUCTORMODULE_H
#define BKLMK0LRECONSTRUCTORMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <mdst/dataobjects/KLMCluster.h>

namespace Belle2 {

  //! Reconstruct K0L-meson clusters
  class BKLMK0LReconstructorModule : public Module {

  public:

    //! Constructor
    BKLMK0LReconstructorModule();

    //! Destructor
    ~BKLMK0LReconstructorModule();

    //! Once-only initialization at start of job
    void initialize();

    //! Prepare for start of each run
    void beginRun();

    //! Process each event
    void event();

    //! Aggregate information at end of each run
    void endRun();

    //! Once-only termination at the end of the job
    void terminate();

  private:

    //! square of the Klong mass (GeV^2)
    double m_KaonMassSq;

    //! maximum cone angle (from IP) separating two hits in a cluster
    double m_MaxHitConeAngle;

    //! Name of the KLMCluster collection
    std::string m_KLMClustersColName;

    //! 2d hits StoreArray
    StoreArray<BKLMHit2d> hit2ds;

    //! klmClusters StoreArray
    StoreArray<KLMCluster> klmClusters;

  };

}

#endif

