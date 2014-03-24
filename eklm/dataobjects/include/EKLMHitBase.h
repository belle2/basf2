/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHITBASE_H
#define EKLMHITBASE_H

/* Extrenal headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <TObject.h>

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
     * Constructor with endcap, layer and sector initializations.
     */
    EKLMHitBase(int Endcap, int Layer, int Sector);

    /**
     * Destructor.
     */
    virtual ~EKLMHitBase();

    /**
     * Get endcap number.
     * @return Endcap number.
     */
    int getEndcap() const;

    /**
     * Set endcap number.
     * @param[in] Endcap Endcap number.
     */
    void setEndcap(int Endcap);

    /**
     * Get layer number.
     * @return Layer number.
     */
    int getLayer() const;

    /**
     * Set layer number.
     * @param[in] nLayer Layer number.
     */
    void setLayer(int nLayer);

    /**
     * Get sector number.
     * @return Sector number.
     */
    int getSector() const;

    /**
     * Set sector number.
     * @param[in] nSector Sector number.
     */
    void setSector(int nSector);

    /**
     * Get energy deposit.
     * @return Energy deposit.
     */
    float getEDep() const;

    /**
     * Set EDep.
     * @param[in] eDep Energy deposit.
     */
    void setEDep(float eDep);

    /**
     * Increase energy deposit.
     * @param[in] eDep Energy deposit.
     */
    void increaseEDep(float deltaEDep);

    /**
     * Get hit time.
     * @return Hit time.
     */
    float getTime() const;

    /**
     * Set hit time.
     * @param[in] time hit time.
     */
    void setTime(float time);

    /**
     * Get the lund code of the (leading) particle.
     * @return Particle code.
     */
    int getPDG() const;

    /**
     * Set the lund code of the (leading) particle
     * @param[in] PDG Particle code.
     */
    void setPDG(int PDG);

    /**
     * Operator to find duplicated (or just similar) hits.
     * Should be overloaded for each type of hits (?)
     */
    bool operator==(const EKLMHitBase& right) const;


  protected:

    /** PDG code of the (leading) particle. */
    int m_PDG;

    /** Time of the hit. */
    float m_Time;

    /** Energy deposition. */
    float m_EDep;

    /** Number of endcap. */
    int m_Endcap;

    /** Number of layer. */
    int m_Layer;

    /** Number of sector. */
    int m_Sector;

  private:

    /** Needed to make objects storable. */
    ClassDef(Belle2::EKLMHitBase, 2);

  };



}

#endif

