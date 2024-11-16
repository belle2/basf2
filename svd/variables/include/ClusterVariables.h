#pragma once

namespace Belle2 {
  class SVDCluster;

  namespace SVD::Variables {
    double clusterCharge(const SVDCluster*);

    double clusterChargeNormalized(const SVDCluster*);

    double clusterSNR(const SVDCluster*);

    double clusterSize(const SVDCluster*);
  }
}
