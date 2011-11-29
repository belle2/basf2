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
#include <TVector3.h>
//#include <TString.h>

namespace Belle2 {


  /**
   * Class EKLMStepHit stores information on particular geant4 step
   * using information  from TrackID and ParentTrackID it is possible to restore the hit tree and thus another type of hits
   */

  class EKLMStepHit : public EKLMHitBase {
  public:

    //! Default constructor
    EKLMStepHit():
        m_PDG(0),
        m_t(0),
        m_E(0),
        m_position(0., 0., 0.),
        m_momentum(0., 0., 0.),
        m_energyDeposit(0.),
        m_trackID(-1),
        m_parentTrackID(-1),
        m_pv(0) {}

    //! Full constructor.
    /*!
    \param m_PDG the PDG code of the particle
    \param m_t the global time the at which the hit occured
    \param m_E energy of the particle
    \param m_position the global position at which the track occured
    \param m_momentum the of the particle that produced the hit
    */

    EKLMStepHit(
      const int PDG,
      const double t,
      const double E,
      const TVector3 position,
      const TVector3 momentum,
      const double edep ,
      const int    trID,
      const int  ptrID,
      const G4VPhysicalVolume * pv
    )  {
      m_PDG = PDG;
      m_t = t;
      m_E = E;
      m_position = position;
      m_momentum = momentum;
      m_energyDeposit = edep;
      m_trackID = trID;
      m_parentTrackID = ptrID;
      m_pv = pv;
      m_pvName = pv->GetName();
    }

    /**
    * Get the lund code of the particle that hit the sensitive area
    */
    int getPDG() const ;


    /**
     *  Set the lund code of the particle that hit the sensitive area
     */
    void setPDG(int);

    /**
     * Get the time at which the hit occured
     */
    double getTime() const;

    /**
     * Set the time at which the hit occured
     */
    void setTime(double);


    /**
     * Get energy of the particle
     */
    double getEnergy() const;

    /**
     * Set energy of the particle
     */
    void setEnergy(double E);


    /**
     * Get global position of the particle hit
     */
    const TVector3 * getPosition() const;


    /**
     * Set global position of the particle hit
     */
    void setPosition(TVector3 & position);


    /**
     * Get momentum of the particle hit
     */
    const TVector3 * getMomentum() const;

    /**
     * Set momentum of the particle hit
     */
    void setMomentum(TVector3 & momentum) ;


    /**
     * Get energy deposition
     */
    double getEDep() const ;

    /**
     * Set energy deposition
     */
    void setEDep(double edep) ;


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
     *  Increase energy deposition
     */
    void increaseEDep(double);



  private:
    int m_subDet;               /** The name of the subdetector */
    int m_identifier;           /** The identifier of subdetector component */
    int m_PDG;                  /** The PDG code of the particle that hit the sensitive area */
    double m_t;                 /** time at which the hit occured */
    double m_E;                 /** energy of particle */
    TVector3 m_position;        /** global position of the hit */
    TVector3 m_momentum;        /** momentum of the hit */
    double m_energyDeposit;     /** energy deposition */
    int m_trackID;              /** track ID */
    int m_parentTrackID;        /** parent track ID */
    //** we do not want to  allow anyone to change the PV !! */
    const G4VPhysicalVolume * m_pv;     //! {ROOT streamer directive}
    std::string m_pvName; /** volume name */


    ClassDef(EKLMStepHit, 1);   /** the class title */

  };

} // end namespace Belle2

#endif
