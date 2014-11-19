/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSIMHIT_H
#define EKLMSIMHIT_H

/* Belle2 headers. */
#include <simulation/dataobjects/SimHitBase.h>
#include <eklm/dataobjects/EKLMHitBase.h>
#include <eklm/dataobjects/EKLMHitCoord.h>
#include <eklm/dataobjects/EKLMHitMomentum.h>

namespace Belle2 {

  /**
   * Class EKLMSimHit stores information on particular Geant step;
   * using information from TrackID and ParentTrackID it is possible
   * to restore the hit tree and thus another type of hits.
   */

  class EKLMSimHit : public SimHitBase, public EKLMHitBase, public EKLMHitCoord,
    public EKLMHitMomentum {
  public:

    /**
     * Constructor.
     */
    EKLMSimHit();

    /**
     * Destructor.
     */
    ~EKLMSimHit();

    /**
     * Get track ID
     * @return Track ID.
     */
    int getTrackID() const;

    /**
     * Set track ID.
     * @param[in] track track ID.
     */
    void setTrackID(int track);

    /**
     * Get ID of parent track.
     * @return Track ID.
     */
    int getParentTrackID() const;

    /**
     * Set ID of parent track.
     * @param[in] track track ID.
     */
    void setParentTrackID(int track);

    /**
     * Get volume identifier.
     * @return Identifier.
     */
    int getVolumeID() const;

    /**
     * Set volume identifier.
     * @param[in] id Identifier.
     */
    void setVolumeID(int id);

    /**
     * Get plane number.
     * @return Plane number.
     */
    int getPlane() const;

    /**
     * Set plane number.
     * @param[in] Plane Plane number.
     */
    void setPlane(int Plane);

    /**
     * Get strip number.
     * @return Strip number.
     */
    int getStrip() const;

    /**
     * Set strip number.
     * @param[in] Strip Strip number.
     */
    void setStrip(int Strip);





    /**
     * Get hit time (implementation of base class function)
     * @return hit time
     */
    float getGlobalTime() const { return getTime(); }

    /**
     * Shift SimHit in time (implementation of base class function)
     * @param delta time shift.
     */
    void shiftInTime(float delta) { setTime(getTime() + delta); }


  private:

    /** Track ID. */
    int m_trackID;

    /** Parent track ID. */
    int m_parentTrackID;

    /** Volume identifier. */
    int m_volid;

    /** Number of plane. */
    int m_Plane;

    /** Number of strip. */
    int m_Strip;

    /** The class title. */
    ClassDef(Belle2::EKLMSimHit, 2);

  };

} // end namespace Belle2

#endif
