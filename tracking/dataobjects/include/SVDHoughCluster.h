/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//#ifndef SVD_HOUGHTRACK_H
//#define SVD_HOUGHTRACK_H

#include <vxd/dataobjects/VxdID.h>
#include <root/TObject.h>
#include <root/TVector3.h>
#include <root/TMath.h>

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /**
   * The SVD digit class.
   *
   * The SVDDigit is an APV25 signal sample.
   * This is a development implementation which is intentionally kept
   * somewhat bulky. I record strip coordinates that won't be kept in future.
   * Also the sensor and cell IDs could be somewhat compressed, if desired.
   */

  class SVDHoughCluster : public TObject {

  public:
    SVDHoughCluster(): trackID(0), hitPos(), sensorID(0) {}

    SVDHoughCluster(unsigned int id, TVector3 pos, VxdID sensID = 0): trackID(id), hitPos(pos),
      sensorID(sensID) {}

    unsigned int getClusterID() { return trackID; }
    TVector3 getHitPos() { return hitPos; }
    VxdID getSensorID() { return sensorID; }

    void setClusterID(unsigned int id) { trackID = id; }
    void setHitPos(TVector3 pos) { hitPos = pos; }
    void setSensorID(VxdID sensID) { sensorID = sensID; }

  private:
    unsigned int trackID;
    TVector3 hitPos;
    VxdID sensorID;

    ClassDef(SVDHoughCluster, 1)

  }; // class SVDHoughCluster

  /** @}*/

} // end namespace Belle2

//#endif
