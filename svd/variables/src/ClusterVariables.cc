#include <svd/dataobjects/SVDCluster.h>
#include <svd/variables/ClusterVariables.h>

namespace Belle2::SVD::Variables {

  double clusterCharge(const SVDCluster* svdCluster)
  {
    return svdCluster->getCharge();
  }

  double clusterChargeNormalized(const SVDCluster* svdCluster)
  {
    double normalization = 1; // TODO: get normalization from track length
    return normalization * svdCluster->getCharge();
  }

  double clusterSNR(const SVDCluster* svdCluster)
  {
    return svdCluster->getSNR();
  }

  double clusterSize(const SVDCluster* svdCluster)
  {
    return svdCluster->getSize();
  }
}
