/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCHIT_H
#define CDCHIT_H

#include <simkernel/B4VHit.h>

//Geant4 classes
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4Circle.hh"

namespace Belle2 {

//! The Class for CDC Hit
  /*! Implementation of CDC hit for BelleII experiment.
  */

  class CDCHit : public B4VHit {

  public:

    //! Constructor
    CDCHit() :
        _layer(0), _wire(0), _trackID(0), _PDG(0), _driftLength(0.), _globalTime(0.),
        _edep(0.), _stepLength(0.), _posFlag(0) {
      _momentum.setX(0.); _momentum.setY(0.); _momentum.setZ(0.);
      _posw.setX(0.); _posw.setY(0.); _posw.setZ(0.);
      _posIn.setX(0.); _posIn.setY(0.); _posIn.setZ(0.);
      _posOut.setX(0.); _posOut.setY(0.); _posOut.setZ(0.);
    }

    //! Constructor with initial values
    CDCHit(G4int layer, G4int wire, G4int trackID, G4int PDG, G4double driftLength, G4double globalTime,
           G4double edep, G4double stepLength, G4ThreeVector momentum,
           G4ThreeVector posw, G4ThreeVector posIn, G4ThreeVector posOut, G4int posFlag)
        :
        _layer(layer), _wire(wire), _trackID(trackID), _PDG(PDG), _driftLength(driftLength), _globalTime(globalTime),
        _edep(edep), _stepLength(stepLength), _posFlag(posFlag) {
      _momentum.setX(momentum.getX()); _momentum.setY(momentum.getY()); _momentum.setZ(momentum.getZ());
      _posw.setX(posw.getX()); _posw.setY(posw.getY()); _posw.setZ(posw.getZ());
      _posIn.setX(posIn.getX()); _posIn.setY(posIn.getY()); _posIn.setZ(posIn.getZ());
      _posOut.setX(posOut.getX()); _posOut.setY(posOut.getY()); _posOut.setZ(posOut.getZ());
    }

    //! Destructor
    ~CDCHit() {}

    //! Copy Constructor
    CDCHit(const CDCHit &right);

    //! Operator =
    const CDCHit& operator=(const CDCHit &right);

    //! Operator ==
    int operator==(const CDCHit &right) const;

    //! Operator new
    inline void *operator new(size_t);

    //! Operator delete
    inline void operator delete(void *aCDCHit);

    //! Draw hits
    void Draw();

    //! Print hits information
    void Print();

    //! Save hits into ASCII file
    void Save(FILE *oFile);

    //! Save cdc hits into ROOT file
    void Save(int ihit);

    //! Load hits from text file
    G4bool Load(FILE *iFile);

  private:
    G4int _layer;             /*!< Layer number */
    G4int _wire;              /*!< wire number */
    G4int _trackID;           /*!< track id of this hit */
    G4int _PDG;               /*!< particle PDG (can be one of secondaries) */
    G4double _driftLength;    /*!< drift length */
    G4double _globalTime;     /*!< flight time  */
    G4double _edep;           /*!< deposited energy of this hit */
    G4double _stepLength ;    /*!< Total step length in this hit */
    G4ThreeVector _momentum;  /*!< momentum of pre-step */
    G4ThreeVector _posw;      /*!< position on wire of closest point between wire and step */
    G4ThreeVector _posIn;     /*!< position of pre-step */
    G4ThreeVector _posOut;    /*!< position of post-step */
    G4int _posFlag;           /*!< left and right flag of signal */

  public:

    //! Get layer number where was registered hit
    inline G4int GetLayerNumber() const { return _layer;}

    //! Get wire number where was registered hit
    inline G4int GetWireNumber() const { return _wire; }

    //! Get PDG code of this particle
    inline G4int GetPDG() const { return _PDG; }

    //! Get track ID
    inline G4int GetTrackID() const { return _trackID; }

    //! Get drift length of this hit
    inline G4double GetDriftLength() const { return _driftLength; }

    //! Get TOF
    inline G4double GetGlobalTime() const { return _globalTime; }

    //! Get energy deposition in this hit
    inline G4double GetEDep() const { return _edep; }

    //! Get step length
    inline G4double GetStepLength() const { return _stepLength; }

    //! Get 3 momentum of this hit
    inline G4ThreeVector GetVecP() const { return _momentum; }

    //! Get each component of momentum
    inline G4double GetPx() const { return _momentum.getX(); }
    inline G4double GetPy() const { return _momentum.getY(); }
    inline G4double GetPz() const { return _momentum.getZ(); }

    //! Get position on wire of closest point between wire and step
    inline G4ThreeVector GetVecPosW() { return _posw; }

    //! Get each component of PosW
    inline G4double GetPosWx() { return _posw.getX(); }
    inline G4double GetPosWy() { return _posw.getY(); }
    inline G4double GetPosWz() { return _posw.getZ(); }

    //! Get position of pre-step
    inline G4ThreeVector GetVecPosIn() { return _posIn; }

    //! Get each component of PosIn
    inline G4double GetPosInx() { return _posIn.getX(); }
    inline G4double GetPosIny() { return _posIn.getY(); }
    inline G4double GetPosInz() { return _posIn.getZ(); }

    //! Get position of post-step
    inline G4ThreeVector GetVecPosOut() { return _posOut; }

    //! Get each component of PosOut
    inline G4double GetPosOutx() { return _posOut.getX(); }
    inline G4double GetPosOuty() { return _posOut.getY(); }
    inline G4double GetPosOutz() { return _posOut.getZ(); }
  };

  typedef G4THitsCollection<CDCHit> CDCHitsCollection;

  extern G4Allocator<CDCHit> CDCHitAllocator;

// Operator new
  inline void* CDCHit::operator new(size_t)
  {
    void *aCDCHit;
    aCDCHit = (void *) CDCHitAllocator.MallocSingle();
    return aCDCHit;
  }

// Operator delete
  inline void CDCHit::operator delete(void *aCDCHit)
  {
    CDCHitAllocator.FreeSingle((CDCHit*) aCDCHit);
  }

} // end of namespace Belle2

#endif //CDCHIT_H
