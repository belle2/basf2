/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributor: Timofey Uglov                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef EKLMSTEPHIT_H
#define EKLMSTEPHIT_H

#include <eklm/dataobjects/EKLMHitBase.h>

class G4VPhysicalVolume;

namespace Belle2 {

  /**
   * Class EKLMStepHit stores information on particular Geant step;
   * using information from TrackID and ParentTrackID it is possible
   * to restore the hit tree and thus another type of hits.
   */

  class EKLMStepHit : public EKLMHitBase {
  public:

    /**
     * Default constructor.
     */
    EKLMStepHit():
      EKLMHitBase(),
      m_momentum(0., 0., 0.),
      m_trackID(-1),
      m_parentTrackID(-1),
      m_pv(0),
      m_Plane(0),
      m_Strip(0) {}

    /**
     * Constructor with partial information.
     */
    EKLMStepHit(
      const TVector3 momentum,
      const double E ,
      const int  trID,
      const int  ptrID,
      const G4VPhysicalVolume* pv
    );

    /**
     * Full constructor.
     */
    EKLMStepHit(
      const int Endcap,
      const int Layer,
      const int Sector,
      const int Plane,
      const int Strip,
      const int PDG,
      const double Time,
      const double EDep,
      const TVector3 GlobalPosition,
      const TVector3 LocalPosition,
      const TVector3 momentum,
      const double E ,
      const int  trID,
      const int  ptrID,
      const G4VPhysicalVolume* pv
    );

    virtual EKLMStripID getID() const;

    /**
     * Get energy of the particle.
     * @return Energy.
     */
    double getEnergy() const;

    /**
     * Set energy of the particle.
     * @param[in] E Energy.
     */
    void setEnergy(double E);

    /**
     * Get momentum of the particle hit.
     * @return Momentum.
     */
    const TVector3* getMomentum() const;

    /**
     * Set momentum of the particle hit.
     * @param[in] momentum Momentum.
     */
    void setMomentum(TVector3& momentum) ;

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
     * Get volume name.
     * @return Name.
     */
    std::string getName()  const;

    /**
     * Set volume name.
     * @param[in] name Name.
     */
    void setName(std::string& name);

    /**
     * Get volume.
     * @return Physical volume.
     */
    const G4VPhysicalVolume* getVolume()  const ;

    /**
     * Set Gean physical volume.
     * @param[in] vol Physical volume.
     */
    void setVolume(const G4VPhysicalVolume* vol);

    /**
     * Get volume type.
     * @return Volume type.
     */
    int  getVolumeType()  const ;

    /**
     * Set volume type.
     * @param[in] type Volume type.
     */
    void setVolumeType(int type);

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

  private:

    /** Energy of particle. */
    double m_energy;

    /** Momentum of the hit. */
    TVector3 m_momentum;


    /** Track ID. */
    int m_trackID;

    /** Parent track ID. */
    int m_parentTrackID;

    /**
     * Physical volume.
     * @details
     * We do not want to  allow anyone to change the PV.
     */
    const G4VPhysicalVolume* m_pv;      //! {ROOT streamer directive}

    /** Number of plane. */
    int m_Plane;

    /** Number of strip. */
    int m_Strip;

    /** Volume type: 0 --> stip, 1 --> SiPM , 2 --> ElectronicBoard. */
    int m_volType;

    /** Volume name */
    std::string m_pvName;

    /** The class title. */
    ClassDef(EKLMStepHit, 1);

  };

} // end namespace Belle2

#endif
