/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/eklm/EKLMHitBase.h>
#include <klm/dataobjects/eklm/EKLMHitCoord.h>
#include <klm/dataobjects/eklm/EKLMHitMomentum.h>

/* Belle 2 headers. */
#include <simulation/dataobjects/SimHitBase.h>

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
    EKLMSimHit()
    {
    }

    /**
     * Destructor.
     */
    ~EKLMSimHit()
    {
    }

    /**
     * Get track ID
     * @return Track ID.
     */
    int getTrackID() const
    {
      return m_trackID;
    }

    /**
     * Set track ID.
     * @param[in] track track ID.
     */
    void setTrackID(int track)
    {
      m_trackID = track;
    }

    /**
     * Get ID of parent track.
     * @return Track ID.
     */
    int getParentTrackID() const
    {
      return m_parentTrackID;
    }

    /**
     * Set ID of parent track.
     * @param[in] track track ID.
     */
    void setParentTrackID(int track)
    {
      m_parentTrackID = track;
    }

    /**
     * Get volume identifier.
     * @return Identifier.
     */
    int getVolumeID() const
    {
      return m_volid;
    }

    /**
     * Set volume identifier.
     * @param[in] id Identifier.
     */
    void setVolumeID(int id)
    {
      m_volid = id;
    }

    /**
     * Get plane number.
     * @return Plane number.
     */
    int getPlane() const
    {
      return m_Plane;
    }

    /**
     * Set plane number.
     * @param[in] plane Plane number.
     */
    void setPlane(int plane)
    {
      m_Plane = plane;
    }

    /**
     * Get strip number.
     * @return Strip number.
     */
    int getStrip() const
    {
      return m_Strip;
    }

    /**
     * Set strip number.
     * @param[in] strip Strip number.
     */
    void setStrip(int strip)
    {
      m_Strip = strip;
    }

    /**
     * Get hit time (implementation of base class function)
     * @return hit time
     */
    float getGlobalTime() const override
    {
      return getTime();
    }

    /**
     * Shift SimHit in time (implementation of base class function)
     * @param delta time shift.
     */
    void shiftInTime(float delta) override
    {
      setTime(getTime() + delta);
    }

  private:

    /** Track ID. */
    int m_trackID = -1;

    /** Parent track ID. */
    int m_parentTrackID = -1;

    /** Volume identifier. */
    int m_volid = -1;

    /** Number of plane. */
    int m_Plane = -1;

    /** Number of strip. */
    int m_Strip = -1;

    /** Class version. */
    ClassDefOverride(Belle2::EKLMSimHit, 2);

  };

}
