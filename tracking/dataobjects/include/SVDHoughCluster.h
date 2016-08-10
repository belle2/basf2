/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_HOUGHCLUSTER_H
#define SVD_HOUGHCLUSTER_H

#include <vxd/dataobjects/VxdID.h>
#include <root/TObject.h>
#include <root/TVector3.h>
#include <root/TMath.h>

namespace Belle2 {

  /**
   * TODO: Add approriate description of class (old description was simple copy of SVDDigit.h)
   */

  class SVDHoughCluster : public TObject {

  public:
    /**
     * Standard constructor
     * If no values are given, initalize parameter with 0
     * @param trackID ID of cluster / track / track cluster
     * @param hitPos position of hit as TVector3
     * @param sensorID ID of firing sensor
     * @param qOH quality of hit
     */
    SVDHoughCluster(): trackID(0), hitPos(), sensorID(0), qOH(0) {}

    /**
     * Constructor if values are given
     */
    SVDHoughCluster(unsigned int id, TVector3 pos, VxdID sensID = 0, unsigned int _qOH = 0): trackID(id), hitPos(pos),
      sensorID(sensID), qOH(_qOH) {}

    /** Standard destructor */
    virtual ~SVDHoughCluster() {}

    /** Return ID of track cluster */
    unsigned int getClusterID() { return trackID; }
    /** Return ID of track cluster */
    unsigned int getTrackID() { return trackID; }
    /** Return quality of hit (QOH) to decide over ROI size */
    unsigned int getQOH() { return qOH; }
    /** Return position of hit */
    TVector3 getHitPos() { return hitPos; }
    /** Return sensorID */
    VxdID getSensorID() { return sensorID; }

    /**
     * Set ID of cluster
     * @param id ID of cluster
     */
    void setClusterID(unsigned int id) { trackID = id; }
    /**
     * Set ID of track
     * @param id ID of track
     */
    void setTrackID(unsigned int id) { trackID = id; }
    /**
     * Set quality of hit (QOH)
     * @param val QOH of hit (0 for high pT, 1 for medium pT, 2 for low pT)
     */
    void setQOH(unsigned int val) { qOH = val; }
    /** Set position of hit */
    void setHitPos(TVector3 pos) { hitPos = pos; }
    /** Set sensorID */
    void setSensorID(VxdID sensID) { sensorID = sensID; }

  private:
    /** track ID */
    unsigned int trackID;
    /** hit position */
    TVector3 hitPos;
    /** sensor ID of firing sensor */
    VxdID sensorID;
    /** Quality of Hit (QOH) */
    unsigned int qOH;

    ClassDef(SVDHoughCluster, 1)

  }; // class SVD_HOUGHCLUSTER_H


} // end namespace Belle2

#endif
