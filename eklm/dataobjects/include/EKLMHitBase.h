/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHITBASE_H
#define EKLMHITBASE_H

#include <sys/types.h>
#include <TObject.h>
#include <TVector3.h>
#include  <string>

namespace Belle2 {

  /**
   * Base Hit reconstruction class.
   * @details
   * EKLMHitBase class is inherited from TObject to make all hits storable.
   */
  class EKLMHitBase: public TObject  {

  public:

    /**
     * Constructor.
     */
    EKLMHitBase();

    /**
     * Constructor.
     */
    EKLMHitBase(int Endcap, int Layer, int Sector, int PDG, double Time,
                double EDep, TVector3 GlobalPosition, TVector3 LocalPosition);

    /**
     * Constructor with endcap, layer and sector initializations.
     */
    EKLMHitBase(int Endcap, int Layer, int Sector);

    /**
     * Destructor.
     */
    ~EKLMHitBase() {};

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
    double getEDep() const;

    /**
     * Set EDep.
     * @param[in] eDep Energy deposit.
     */
    void setEDep(double eDep);

    /**
     * Increase energy deposit.
     * @param[in] eDep Energy deposit.
     */
    void increaseEDep(double deltaEDep);

    /**
     * Get hit time.
     * @return Hit time.
     */
    double getTime() const;

    /**
     * Set hit time.
     * @param[in] time hit time.
     */
    void setTime(double time);

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
     * Get global position of the particle hit.
     * @return Hit coordinates.
     */
    const TVector3* getPosition() const;

    /**
     * Set global position of the particle hit
     * @param[in] position Hit coordinates.
     */
    void setPosition(TVector3& position);

    /**
     * Set global position of the particle hit.
     * @param[in] position Hit coordinates.
     */
    void setPosition(const TVector3* position);

    /**
     * Get local position of the particle hit.
     * @return Hit coordinates.
     */
    const TVector3* getLocalPosition() const;

    /**
     * Set local position of the particle hit.
     * @param[in] position Hit coordinates.
     */
    void setLocalPosition(TVector3& position);

    /**
     * Set local position of the particle hit.
     * @param[in] position Hit coordinates.
     */
    void setLocalPosition(const TVector3* position);

  protected:

    /** PDG code of the (leading) particle. */
    int m_PDG;

    /** Time of the hit. */
    double m_Time;

    /** Energy deposition. */
    double m_EDep;

    /** Global position of the hit. */
    TVector3 m_GlobalPosition;

    /** Local position of the hit. */
    TVector3 m_LocalPosition;

    /** Number of endcap. */
    int m_Endcap;

    /** Number of layer. */
    int m_Layer;

    /** Number of sector. */
    int m_Sector;

  private:

    /**
     * Needed to make objects storable.
     */
    ClassDef(Belle2::EKLMHitBase, 1);

  };

} // end of namespace Belle2

#endif //EKLMHITBASE_H
