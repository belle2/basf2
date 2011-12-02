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
#include "G4VPhysicalVolume.hh"



namespace Belle2 {


  /**
   * Class EKLMStepHit stores information on particular geant4 step
   * using information  from TrackID and ParentTrackID it is possible to restore the hit tree and thus another type of hits
   */

  class EKLMStepHit : public EKLMHitBase {
  public:

    //! Default constructor
    EKLMStepHit():
        EKLMHitBase(),
        m_momentum(0., 0., 0.),
        m_trackID(-1),
        m_parentTrackID(-1),
        m_pv(0),
        m_Plane(0),
        m_Strip(0) {}

    //!Partial info
    EKLMStepHit(
      const TVector3 momentum,
      const double E ,
      const int  trID,
      const int  ptrID,
      const G4VPhysicalVolume * pv
    )
        :
        EKLMHitBase() {
      m_energy = E;
      m_momentum = momentum;
      m_trackID = trID;
      m_parentTrackID = ptrID;
      m_pv = pv;
      m_pvName = pv->GetName();
    }



    //! Full constructor.
    /*!
    \param m_PDG the PDG code of the particle
    \param m_t the global time the at which the hit occured
    \param m_E energy of the particle
    \param m_position the global position at which the track occured
    \param m_momentum the of the particle that produced the hit
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
      const G4VPhysicalVolume * pv
    ) :
        EKLMHitBase(Endcap, Layer, Sector, PDG, Time,  EDep, GlobalPosition, LocalPosition) {
      m_energy = E;
      m_momentum = momentum;
      m_trackID = trID;
      m_parentTrackID = ptrID;
      m_pv = pv;
      m_pvName = pv->GetName();
    }



    /**
     * Get energy of the particle
     */
    double getEnergy() const;

    /**
     * Set energy of the particle
     */
    void setEnergy(double E);


    /**
     * Get momentum of the particle hit
     */
    const TVector3 * getMomentum() const;

    /**
     * Set momentum of the particle hit
     */
    void setMomentum(TVector3 & momentum) ;


    /**
     * Get track ID
     */
    int getTrackID() const;

    /**
     * Set track ID
     */
    void setTrackID(int track);


    /**
     * Get  ID of parent track
     */
    int getParentTrackID() const;

    /**
     * Set ID of parent track
     */
    void setParentTrackID(int track);


    /**
     * Get volume name
     */
    std::string getName()  const;
    /**
     * Set volume name
     */
    void setName(std::string & name);




    /**
     * Get volume
     */
    const G4VPhysicalVolume* getVolume()  const ;
    /**
     * Set volume
     */
    void setVolume(const G4VPhysicalVolume *);


    /**
     * Get volume type
     */
    int  getVolumeType()  const ;

    /**
     * Set volume type
     */
    void setVolumeType(int);

    /**
     * Get plane number.
     */
    int getPlane() const;

    /**
     * Set plane number.
     */
    void setPlane(int Plane);

    /**
     * Get strip number.
     */
    int getStrip() const;

    /**
     * Set strip number.
     */
    void setStrip(int Strip);




  private:

    /**
    * energy of particle
    */
    double m_energy;

    /**
     *  momentum of the hit
    */
    TVector3 m_momentum;


    /**
     * track ID
     */
    int m_trackID;
    /**
     * parent track ID
     */
    int m_parentTrackID;

    //** we do not want to  allow anyone to change the PV !! */
    const G4VPhysicalVolume * m_pv;     //! {ROOT streamer directive}


    /**
     * Number of plane.
     */
    int m_Plane;

    /**
     * Number of strip.
     */
    int m_Strip;


    /**
     * Volume type: 0 --> stip, 1 --> SiPM , 2 --> ElectronicBoard
     */
    int m_volType;


    std::string m_pvName; /** volume name */


    ClassDef(EKLMStepHit, 1);   /** the class title */

  };

} // end namespace Belle2

#endif
