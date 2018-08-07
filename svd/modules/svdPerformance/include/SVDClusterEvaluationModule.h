/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Giulia Casarosa                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_CLUSTEREVALUATION_H_
#define SVD_CLUSTEREVALUATION_H_

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDHistograms.h>
#include <tracking/dataobjects/SVDIntercept.h>
#include <framework/dataobjects/EventMetaData.h>


#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TDirectory.h>
#include <TCollection.h>
#include <TList.h>
#include <TH3F.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {

  /** The SVD ClusterEvaluation Module
   *
   */

  class SVDClusterEvaluationModule : public Module {

  public:

    SVDClusterEvaluationModule();

    virtual ~SVDClusterEvaluationModule();
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */
    std::string m_ClusterName;   /**< SVDCluster StoreArray name */
    std::string m_InterceptName;   /**< SVDIntercept StoreArray name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

    double m_UbinWidth;
    double m_VbinWidth;

  private:

    StoreArray<SVDCluster> m_svdClusters;
    StoreArray<SVDIntercept> m_svdIntercepts;
    StoreObjPtr<EventMetaData> m_eventMetaData;

    SVDHistograms<TH2F>* m_interCoor;
    SVDHistograms<TH1F>* m_clsCoor;
    SVDHistograms<TH1F>* m_clsResid;
    SVDHistograms<TH1F>* m_clsResol;

  };
}

#endif /* SVDClusterEvaluationModule_H_ */

