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

#include "eklm/eklmhit/EKLMHitBase.h"
#include <TVector3.h>

#include "G4VHit.hh"
namespace Belle2 {

  //! Class to handle simulation hits
  //  class EKLMSimHit  : public G4VHit  {
  class EKLMSimHit : public EKLMHitBase  {

  public:

    //! default constructor needed to make the class storable
    EKLMSimHit();

    //! Constructor with initial values
    EKLMSimHit(G4VPhysicalVolume *pv, TVector3 global_pos,
               TVector3 local_pos, G4double time, G4int PDGcode,
               G4double eDep);

    //! Destructor
    ~EKLMSimHit() {};

    //! returns physical volume
    G4VPhysicalVolume *getPV();
    void setPV(G4VPhysicalVolume *);

    //! returns global position of the hit
    TVector3 getGlobalPos();

    //! returns local position of the hit
    TVector3 getLocalPos();

    //! set global position of the hit
    void setGlobalPos(const TVector3 &);

    //! set local position of the hit
    void setLocalPos(const TVector3 &);




    //! returns hit time
    G4double getTime();
    void setTime(double);

    //! returns energy deposition
    G4double getEDep();
    void setEDep(double);

    //! returns PDG code of the particle
    G4int getPDGCode();
    void setPDGCode(G4int);


    //! dumps hit into ASCII file
    void Save(char * filename);

    //! returm track ID
    G4int getTrackID();
    void  setTrackID(G4int id);

    //! returs parent track ID
    G4int getParentTrackID();
    void setParentTrackID(G4int id);

    //! returns  first hit status
    bool  getVolType();
    void  setVolType(int vt);

    //! returns  particle momentum
    TVector3  getMomentum();
    void  setMomentum(const TVector3 &p);


    //! returns  particle energy
    double  getEnergy();
    void  setEnergy(const double e);



  private:
    //! Physical volume
    G4VPhysicalVolume *m_pv; //! {ROOT streamer directive}

    //! particle momentum
    TVector3 m_momentum;

    //! particle energy
    double m_energy;

    //! hit position (in global reference frame)
    TVector3 m_global_pos;

    //! hit position (in local reference frame)
    TVector3 m_local_pos;

    //!hit time
    G4double m_time;

    //! energy depostion
    G4double m_eDep;

    //! PDG code of the track particle
    G4int m_PDGcode;

    //! track ID
    G4int m_trackID;

    //! parent track ID
    G4int m_parentTrackID;

    //! is first step in volume
    int m_volType;

    //! Needed to make root object storable
    ClassDef(Belle2::EKLMSimHit, 1);

  };

  //! define collections of Sim Hits
  //  typedef G4THitsCollection<EKLMSimHit> EKLMSimHitsCollection;

} // end of namespace Belle2

#endif //EKLMSIMHIT_H
