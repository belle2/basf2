/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMHit1d.h>
#include <klm/dataobjects/KLMElementNumbers.h>

/* Basf2 headers. */
#include <framework/datastore/RelationsObject.h>

/* ROOT headers. */
#include <Math/Vector3D.h>

/* CLHEP headers. */
#include <CLHEP/Vector/ThreeVector.h>

namespace Belle2 {

  /**
   * KLM 2d hit.
   */
  class KLMHit2d : public RelationsObject {

  public:

    /**
     * Constructor.
     */
    KLMHit2d();

    /**
     * Constructor from two orthogonal KLMDigits (EKLM).
     * @param[in] digit1 KLMDigit in plane 1.
     * @param[in] digit2 KLMDigit in plane 2.
     */
    explicit KLMHit2d(KLMDigit* digit1, KLMDigit* digit2);

    /**
     * Constructor with initial values (BKLM).
     *
     * @param[in] hitPhi
     * Reconstructed BKLMHit1d along the phi-measuring direction.
     *
     * @param[in] hitZ
     * Reconstructed BKLMHit1d along the z-measuring direction.
     *
     * @param[in] globalPos
     * Reconstructed position in global coordinates.
     *
     * @param[in] time
     * Reconstructed time of the hit.
     */
    KLMHit2d(const BKLMHit1d* hitPhi, const BKLMHit1d* hitZ,
             const CLHEP::Hep3Vector& globalPos, double time);

    /**
     * Destructor.
     */
    ~KLMHit2d()
    {
    }

    /**
     * Get subdetector number.
     * @return Subdetector number.
     */
    int getSubdetector() const
    {
      return m_Subdetector;
    }

    /**
     * Set subdetector number.
     * @param[in] subdetector Subdetector number.
     */
    void setSubdetector(int subdetector)
    {
      m_Subdetector = subdetector;
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
     * @param[in] section Section number.
     */
    void setSection(int section)
    {
      m_Section = section;
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
     * Set first strip number for EKLM hit in the x-measuring plane.
     * @param[in] strip Strip number.
     */
    void setXStripMin(int strip)
    {
      m_Strip[0] = strip;
    }

    /**
     * Set last strip number for EKLM hit in the x-measuring plane.
     * @param[in] strip Strip number.
     */
    void setXStripMax(int strip)
    {
      m_LastStrip[0] = strip;
    }

    /**
     * Set first strip number for EKLM hit in the y-measuring plane.
     * @param[in] strip Strip number.
     */
    void setYStripMin(int strip)
    {
      m_Strip[1] = strip;
    }

    /**
     * Set last strip number for EKLM hit in y-measuring plane.
     * @param[in] strip Strip number.
     */
    void setYStripMax(int strip)
    {
      m_LastStrip[1] = strip;
    }

    /**
     * Determine whether this 2D hit is in RPC or scintillator.
     */
    bool inRPC() const
    {
      return (m_Subdetector == KLMElementNumbers::c_BKLM) &&
             (m_Layer >= BKLMElementNumbers::c_FirstRPCLayer);
    }

    /**
     * Get strip number for z plane.
     */
    int getZStripMin() const
    {
      return m_Strip[BKLMElementNumbers::c_ZPlane];
    }

    /**
     * Get last strip number for z plane.
     */
    int getZStripMax() const
    {
      return m_LastStrip[BKLMElementNumbers::c_ZPlane];
    }

    /**
     * Get average strip number for z plane.
     */
    double getZStripAve() const
    {
      return 0.5 * (getZStripMin() + getZStripMax());
    }

    /**
     * Get strip number for phi plane.
     */
    int getPhiStripMin() const
    {
      return m_Strip[BKLMElementNumbers::c_PhiPlane];
    }

    /**
     * Get last strip number for phi plane.
     */
    int getPhiStripMax() const
    {
      return m_LastStrip[BKLMElementNumbers::c_PhiPlane];
    }

    /**
     * Get average strip number for phi plane.
     */
    double getPhiStripAve() const
    {
      return 0.5 * (getPhiStripMin() + getPhiStripMax());
    }

    /**
     * Get first strip number for EKLM hit in the x-measuring plane.
     */
    int getXStripMin() const
    {
      return m_Strip[BKLMElementNumbers::c_PhiPlane];
    }

    /**
     * Get last strip number for EKLM hit in the x-measuring plane.
     */
    int getXStripMax() const
    {
      return m_LastStrip[BKLMElementNumbers::c_PhiPlane];
    }

    /**
     * Get first strip number for EKLM hit in the y-measuring plane.
     */
    int getYStripMin() const
    {
      return m_Strip[BKLMElementNumbers::c_ZPlane];
    }

    /**
     * Get last strip number for EKLM hit in the y-measuring plane.
     */
    int getYStripMax() const
    {
      return m_LastStrip[BKLMElementNumbers::c_ZPlane];
    }

    /**
     * Set hit global position.
     * @param[in] x Hit x coordinate.
     * @param[in] y Hit y coordinate.
     * @param[in] z Hit z coordinate.
     */
    void setPosition(float x, float y, float z)
    {
      m_GlobalX = x;
      m_GlobalY = y;
      m_GlobalZ = z;
    }

    /**
     * Set hit global position.
     * @param[in] pos Position.
     */
    //void setPosition(const ROOT::Math::XYZVector& pos);

    /**
     * Get hit global position x coordinate.
     * @return Hit x coordinate.
     */
    float getPositionX() const
    {
      return m_GlobalX;
    }

    /**
     * Get hit global position y coordinate.
     * @return Hit y coordinate.
     */
    float getPositionY() const
    {
      return m_GlobalY;
    }

    /**
     * Get hit global position z coordinate.
     * @return Hit z coordinate.
     */
    float getPositionZ() const
    {
      return m_GlobalZ;
    }

    /**
     * Get hit global position.
     * @return Hit coordinates.
     */
    ROOT::Math::XYZVector getPosition() const
    {
      return ROOT::Math::XYZVector(m_GlobalX, m_GlobalY, m_GlobalZ);
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
     * Set MC time.
     * @param[in] t TIme.
     */
    void setMCTime(float t)
    {
      m_MCTime = t;
    }

    /**
     * Get MC time.
     * @return Time.
     */
    float getMCTime() const
    {
      return m_MCTime;
    }

    /**
     * Get energy deposit.
     * @return Energy deposit.
     */
    float getEnergyDeposit() const
    {
      return m_EnergyDeposit;
    }

    /**
     * Set energy deposit.
     * @param[in] energyDeposit Energy deposit.
     */
    void setEnergyDeposit(float energyDeposit)
    {
      m_EnergyDeposit = energyDeposit;
    }

    /**
     * Get Chi^2 of the crossing point.
     * @return Chi^2.
     */
    float getChiSq() const
    {
      return m_ChiSq;
    }

    /**
     * Set Chi^2 of the crossing point.
     * @param[in] chisq Chi^2.
     */
    void setChiSq(float chisq)
    {
      m_ChiSq = chisq;
    }

    /**
     * Determine whether this 2D hit is outside the trigger-coincidence window.
     */
    bool isOutOfTime() const
    {
      return m_IsOutOfTime;
    }

    /**
     * Set whether this 2D hit is outside the trigger-coincidence window.
     */
    void isOutOfTime(bool outOfTime)
    {
      m_IsOutOfTime = outOfTime;
    }

    /**
     * Determine whether this 2D hit is associated with
     * a muid-extrapolated track.
     */
    bool isOnTrack() const
    {
      return m_IsOnTrack;
    }

    /**
     * Set whether this 2D hit is associated with a muid-extrapolated track.
     */
    void isOnTrack(bool onTrack)
    {
      m_IsOnTrack = onTrack;
    }

    /**
     * Determine whether this 2D hit is associated with
     * a BKLM stand-alone track.
     */
    bool isOnStaTrack() const
    {
      return m_IsOnStaTrack;
    }

    /**
     * Set whether this 2D hit is associated with a BKLM stand-alone track.
     */
    void isOnStaTrack(bool onStaTrack)
    {
      m_IsOnStaTrack = onStaTrack;
    }

  private:

    /** Number of subdetector. */
    int m_Subdetector = 0;

    /** Number of section. */
    int m_Section = 0;

    /** Number of sector. */
    int m_Sector = 0;

    /** Number of layer. */
    int m_Layer = 0;

    /** Number of strip in each plane. */
    int m_Strip[KLMElementNumbers::getMaximalPlaneNumber()] = {0};

    /** Number of last strip in each plane. */
    int m_LastStrip[KLMElementNumbers::getMaximalPlaneNumber()] = {0};

    /** Global position X coordinate. */
    float m_GlobalX = 0.0;

    /** Global position Y coordinate. */
    float m_GlobalY = 0.0;

    /** Global position Z coordinate. */
    float m_GlobalZ = 0.0;

    /** Time of the hit. */
    float m_Time = 0.0;

    /** MC time. */
    float m_MCTime = 0.0;

    /** Energy deposition. */
    float m_EnergyDeposit = 0.0;

    /* From EKLMHit2d. */

    /** Chi^2 of the hit. */
    float m_ChiSq = 0.0;

    /* From BKLMHit2d. */

    /** Whether this 2D hit is outside the trigger-coincidence window. */
    bool m_IsOutOfTime = false;

    /** Whether this 2D hit is associated with a muid-extrapolated track. */
    bool m_IsOnTrack = false;

    /** Wether this 2D hit is associated with a BKLM stand-alone track. */
    bool m_IsOnStaTrack = false;

    /** Class version. */
    ClassDef(Belle2::KLMHit2d, 1);

  };

}
