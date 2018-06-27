/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDClusterQualityEstimatorModule_H
#define SVDClusterQualityEstimatorModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <svd/dataobjects/SVDCluster.h>
#include <TH2.h>
#include <TFile.h>

namespace Belle2 {


  /* Calculates the probability of a cluster originating from signal hit */

  class SVDClusterQualityEstimatorModule : public Module {

  public:

    /* Constructor */
    SVDClusterQualityEstimatorModule();

    /** Init the module.*/
    virtual void initialize();
    /** EventWise jobs (determination of likelihoods). */
    virtual void event();
    /** Final output.*/
    virtual void terminate();


  protected:
    // Data members

    /** SVDCluster collection name. */
    std::string m_svdClustersName;

    /** The storeArray for svdClusters */
    StoreArray<SVDCluster> m_svdClusters;

    std::string m_inputPDF; /** File path of root file containing pdf histograms */

    TFile* m_calibrationFile; /**Pointer to root TFile containing PDF histograms */
  };
} //end namespace Belle2

#endif
