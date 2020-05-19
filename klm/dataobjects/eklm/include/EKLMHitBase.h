/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <Rtypes.h>

namespace Belle2 {

  /**
   * Base hit class.
   */
  class EKLMHitBase {

  public:

    /**
     * Constructor.
     */
    EKLMHitBase();

    /**
     * Constructor with section, layer and sector initializations.
     */
    EKLMHitBase(int section, int layer, int sector);

    /**
     * Destructor.
     */
    virtual ~EKLMHitBase();

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

  protected:

    /** PDG code of the (leading) particle. */
    int m_PDG = -1;

    /** Time of the hit. */
    float m_Time = -1;

    /** Energy deposition. */
    float m_EDep = -1;

    /** Number of section. */
    int m_Section;

    /** Number of layer. */
    int m_Layer;

    /** Number of sector. */
    int m_Sector;

  private:

    /** Class version. */
    ClassDef(Belle2::EKLMHitBase, 3);

  };

}
