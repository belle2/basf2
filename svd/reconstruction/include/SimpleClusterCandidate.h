/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_SIMPLECLUSTERCANDIDATE_H
#define SVD_SIMPLECLUSTERCANDIDATE_H

#include <vxd/dataobjects/VxdID.h>
#include <vector>

namespace Belle2 {

  namespace SVD {

    struct stripInCluster {
      int recoDigitIndex;
      float charge;
      float noise;
      int cellID;
      float time;
    };

    /**
     * Class representing a cluster during simple clustering of the SVD
      */
    class SimpleClusterCandidate {
    public:
      /** Constructor to create an empty Cluster */
      SimpleClusterCandidate(VxdID vxdID, bool isUside, int sizeHeadTail, double cutSeed, double cutAdjacent);

      /** Add a Strip to the current cluster.
       * Update the cluster seed seed.
       * @param stripInCluster aStrip to add to the cluster
       * return true if the strip is on the expected side and sensor and it's next to the last strip added to the cluster candidate
       */
      bool add(VxdID vxdID, bool isUside, struct  stripInCluster& aStrip);

      void clear();

      void finalizeCluster();

      bool isGoodCluster();

      VxdID getSensorID() {return m_vxdID;}

      bool isUSide() {return m_isUside;}

      /** get the strip-wise charge of the cluster (sum of quadratic fitcharges). */
      float getCharge() const { return m_charge; }
      /** get the strip-wise charge of the cluster (sum of quadratic fitcharges). */
      float getChargeError() const { return m_chargeError; }

      /** get the seed charge of the cluster */
      float getSeedCharge() const { return m_seedCharge; }

      /** get the strip-wise time of the cluster (sum of quadratic fittimes). */
      float getTime() const { return m_time; }
      /** get the strip-wise time of the cluster (sum of quadratic fittimes). */
      float getTimeError() const { return m_timeError; }

      /** get the strip-wise position of the cluster (sum of quadratic fitpositions). */
      float getPosition() const { return m_position; }
      /** get the strip-wise position of the cluster (sum of quadratic fitpositions). */
      float getPositionError() const { return m_positionError; }

      float getSNR() const { return m_SNR; }

      /** get the cluster size */
      int size() const { return m_strips.size(); }


    protected:

      VxdID m_vxdID;

      bool m_isUside;

      int m_sizeHeadTail;

      double m_cutSeed;
      double m_cutAdjacent;

      /** Charge of the cluster */
      float m_charge;
      /** Error on Charge of the cluster */
      float m_chargeError;
      /** Seed Charge of the cluster */
      float m_seedCharge;
      /** Time of the cluster */
      float m_time;
      /** Error on Time of the cluster */
      float m_timeError;
      /** Position of the cluster */
      float m_position;
      /** Error on Position of the cluster */
      float m_positionError;

      float m_SNR;

      int m_seedIndex;

      std::vector<stripInCluster> m_strips;

    };

  }

}

#endif //SVD_SIMPLECLUSTERCANDIDATE_H
