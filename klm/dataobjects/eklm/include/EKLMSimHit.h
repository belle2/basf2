/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <simulation/dataobjects/SimHitBase.h>

/* ROOT headers. */
#include <Math/Vector4D.h>
#include <TVector3.h>

namespace Belle2 {

  /**
   * Class EKLMSimHit stores information on particular Geant step;
   * using information from TrackID and ParentTrackID it is possible
   * to restore the hit tree and thus another type of hits.
   */

  class EKLMSimHit : public SimHitBase {

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
     * Get the lund code of the (leading) particle.
     * @return Particle code.
     */
    int getPDG() const
    {
      return m_PDG;
    }

    /**
     * Set the lund code of the (leading) particle
     * @param[in] pdg Particle code.
     */
    void setPDG(int pdg)
    {
      m_PDG = pdg;
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
     * Get section number.
     * @return Section number.
     */
    int getSection() const
    {
      return m_Section;
    }

    /**
     * Set section number.
     * @param[in] Section Section number.
     */
    void setSection(int Section)
    {
      m_Section = Section;
    }

    /**
     * Get layer number.
     * @return Layer number.
     */
    int getLayer() const
    {
      return m_Layer;
    }

    /**
     * Set layer number.
     * @param[in] layer Layer number.
     */
    void setLayer(int layer)
    {
      m_Layer = layer;
    }

    /**
     * Get sector number.
     * @return Sector number.
     */
    int getSector() const
    {
      return m_Sector;
    }

    /**
     * Set sector number.
     * @param[in] sector Sector number.
     */
    void setSector(int sector)
    {
      m_Sector = sector;
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
     * Get energy deposit.
     * @return Energy deposit.
     */
    float getEnergyDeposit() const
    {
      return m_EDep;
    }

    /**
     * Set EnergyDeposit.
     * @param[in] eDep Energy deposit.
     */
    void setEnergyDeposit(float eDep)
    {
      m_EDep = eDep;
    }

    /**
     * Get hit time.
     * @return Hit time.
     */
    float getTime() const
    {
      return m_Time;
    }

    /**
     * Set hit time.
     * @param[in] time hit time.
     */
    void setTime(float time)
    {
      m_Time = time;
    }

    /**
     * Set hit local position.
     * @param[in] x Hit x coordinate.
     * @param[in] y Hit y coordinate.
     * @param[in] z Hit z coordinate.
     */
    void setLocalPosition(float x, float y, float z)
    {
      m_localX = x;
      m_localY = y;
      m_localZ = z;
    }

    /**
     * Get hit local position x coordinate.
     * @return Hit x coordinate.
     */
    float getLocalPositionX() const
    {
      return m_localX;
    }

    /**
     * Get hit local position y coordinate.
     * @return Hit y coordinate.
     */
    float getLocalPositionY() const
    {
      return m_localY;
    }

    /**
     * Get hit local position z coordinate.
     * @return Hit z coordinate.
     */
    float getLocalPositionZ() const
    {
      return m_localZ;
    }

    /**
     * Get ihit local position.
     * @return Hit coordinates.
     */
    TVector3 getLocalPosition() const
    {
      return TVector3(m_localX, m_localY, m_localZ);
    }

    /**
     * Set hit global position.
     * @param[in] x Hit x coordinate.
     * @param[in] y Hit y coordinate.
     * @param[in] z Hit z coordinate.
     */
    void setPosition(float x, float y, float z)
    {
      m_globalX = x;
      m_globalY = y;
      m_globalZ = z;
    }

    /**
     * Set hit global position.
     * @param[in] pos Position.
     */
    void setPosition(const TVector3& pos)
    {
      m_globalX = pos.X();
      m_globalY = pos.Y();
      m_globalZ = pos.Z();
    }

    /**
     * Get hit global position x coordinate.
     * @return Hit x coordinate.
     */
    float getPositionX() const
    {
      return m_globalX;
    }

    /**
     * Get hit global position y coordinate.
     * @return Hit y coordinate.
     */
    float getPositionY() const
    {
      return m_globalY;
    }

    /**
     * Get hit global position z coordinate.
     * @return Hit z coordinate.
     */
    float getPositionZ() const
    {
      return m_globalZ;
    }

    /**
     * Get hit global position.
     * @return Hit coordinates.
     */
    TVector3 getPosition() const
    {
      return TVector3(m_globalX, m_globalY, m_globalZ);
    }

    /**
     * Set momentum.
     * @param[in] p momentum
     */
    void setMomentum(const ROOT::Math::PxPyPzEVector& p)
    {
      m_e = p.E();
      m_pX = p.Px();
      m_pY = p.Py();
      m_pZ = p.Pz();
    }

    /**
     * Get momentum.
     * @return Momentum.
     */
    ROOT::Math::PxPyPzEVector getMomentum() const
    {
      return ROOT::Math::PxPyPzEVector(m_pX, m_pY, m_pZ, m_e);
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

    /** PDG code of the (leading) particle. */
    int m_PDG = -1;

    /** Track ID. */
    int m_trackID = -1;

    /** Parent track ID. */
    int m_parentTrackID = -1;

    /** Volume identifier. */
    int m_volid = -1;

    /** Number of section. */
    int m_Section = 0;

    /** Number of layer. */
    int m_Layer = 0;

    /** Number of sector. */
    int m_Sector = 0;

    /** Number of plane. */
    int m_Plane = 0;

    /** Number of strip. */
    int m_Strip = 0;

    /** Time of the hit. */
    float m_Time = -1;

    /** Energy deposition. */
    float m_EDep = -1;

    /** Local position X coordinate. */
    float m_localX = 0;

    /** Local position Y coordinate. */
    float m_localY = 0;

    /** Local position Z coordinate. */
    float m_localZ = 0;

    /** Global position X coordinate. */
    float m_globalX;

    /** Global position Y coordinate. */
    float m_globalY;

    /** Global position Z coordinate. */
    float m_globalZ;

    /** Energy. */
    float m_e = 0;

    /** Momentum X component. */
    float m_pX = 0;

    /** Momentum Y component. */
    float m_pY = 0;

    /** Momentum Z component. */
    float m_pZ = 0;

    /** Class version. */
    ClassDefOverride(Belle2::EKLMSimHit, 3);

  };

}
