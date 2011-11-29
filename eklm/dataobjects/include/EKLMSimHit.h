/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSIMHIT_H
#define EKLMSIMHIT_H


#include <TObject.h>

#include "G4VPhysicalVolume.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

#include <eklm/dataobjects/EKLMHitBase.h>
#include <TVector3.h>

#include "G4VHit.hh"
namespace Belle2 {

  /**
   * Class to handle simulation hits
   */

  class EKLMSimHit : public EKLMHitBase  {

  public:

    /**
     * default constructor needed to make the class storable
     */
    EKLMSimHit();

    /**
     *  Constructor with initial values
     */
    EKLMSimHit(G4VPhysicalVolume *pv, TVector3 global_pos,
               TVector3 local_pos, G4double time, G4int PDGcode,
               G4double eDep);

    /**
     * Destructor
     */
    ~EKLMSimHit() {};

    /**
     *returns physical volume
     */
    const G4VPhysicalVolume *getVolume()  const;

    /**
     * set physical volume
     */
    void setVolume(const G4VPhysicalVolume *);

    /**
     * returns global position of the hit
     */
    const TVector3 * getGlobalPos() const;

    /**
     * set global position of the hit
     */
    void setGlobalPos(const TVector3 &);

    /**
     * set global position of the hit
     */
    void setGlobalPos(const TVector3 *);

    /**
     * returns local position of the hit
     */
    const TVector3 * getLocalPos() const;

    /**
     * set local position of the hit
     */
    void setLocalPos(const TVector3 &);

    /**
     * set local position of the hit
     */
    void setLocalPos(const TVector3 *);

    /**
     * returns hit time
     */
    G4double getTime()  const;

    /**
     * set hit time
     */
    void setTime(double);

    /**
     * returns energy deposition
     */
    G4double getEDep() const;

    /**
     * set  energy deposition
     */
    void setEDep(double);

    /**
     *returns PDG code of the particle
     */
    G4int getPDGCode() const;

    /**
     *sets PDG code of the particle
     */
    void setPDGCode(G4int);

    /**
     * returns track ID
     */
    G4int getTrackID() const;

    /**
     * sets track ID
     */
    void  setTrackID(G4int);

    /**
     * returs parent track ID
     */
    int getParentTrackID() const;

    /**
     * sets parent track ID
     */
    void setParentTrackID(G4int);

    /**
     * returns volume type (needed for background study mode)
     */
    bool  getVolType() const;

    /**
     * sets volume type (needed for background study mode)
     */
    void  setVolType(int);

    /**
     * returns  particle momentum
     */
    const TVector3 *  getMomentum() const;

    /**
     * sets  particle momentum
     */
    void  setMomentum(const TVector3 &p);

    /**
     * sets  particle momentum
     */
    void  setMomentum(const TVector3 *p);


    /**
     * returns  particle energy
     */
    double  getEnergy() const;

    /**
     * sets  particle energy
     */
    void  setEnergy(double);



    //! dumps hit into ASCII file
    void Save(char * filename);



  private:
    /**
     *Physical volume
     */
    const  G4VPhysicalVolume *m_pv; //! {ROOT streamer directive}

    /**
     * particle momentum
     */
    TVector3 m_momentum;

    /**
     *particle energy
     */
    double m_energy;

    /**
     * hit position (in global reference frame)
     */
    TVector3 m_global_pos;

    /**
     * hit position (in local reference frame)
     */
    TVector3 m_local_pos;

    /**
     * hit time
     */
    G4double m_time;

    /**
     * energy depostion
     */
    G4double m_eDep;

    /**
     * PDG code of the track particle
     */
    G4int m_PDGcode;

    /**
     * track ID
     */
    G4int m_trackID;

    /**
     * parent track ID
     */
    G4int m_parentTrackID;


    /**
     * Volume type (for Background studies)
     */
    G4int m_volType;


    // Needed to make root object storable
    ClassDef(Belle2::EKLMSimHit, 1);

  };


} // end of namespace Belle2

#endif //EKLMSIMHIT_H
