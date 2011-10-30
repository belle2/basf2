/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 * 7/31: Poyuan                                                           *
 *                                                                        *
 *                                                                        *
 **************************************************************************/
#ifndef ECLGEOMETRYPAR_H
#define ECLGEOMETRYPAR_H

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>
#include <map>


#include "TVector3.h"
typedef int EclIdentifier ;
typedef double     EclGeV        ;
///
typedef double   EclCM         ;
///
typedef double     EclCC         ;
///
typedef double     EclKG         ;
///
typedef double     EclRad        ;
///
typedef double     EclDeg        ;


namespace Belle2 {

//! The Class for ECL Geometry Parameters
  /*! This class provides ECL gemetry paramters for simulation, reconstruction and so on.
      These parameters are gotten from gearbox.
  */
  class ECLGeometryPar {

  public:

    //! Constructor
    ECLGeometryPar();

    //! Destructor
    virtual ~ECLGeometryPar();

    //! Static method to get a reference to the ECLGeometryPar instance.
    /*!
        \return A reference to an instance of this class.
    */
    static ECLGeometryPar* Instance();

    //! Clears
    void clear();

    //! Print some debug information
    void Print() const;

    //! Gets geometry parameters from gearbox.
    void read();

    //! Mapping theta, phi Id
    void Mapping(int cid);


    TVector3 GetCrystalPos(int cid);  /** The Postion of crystal*/

    TVector3 GetCrystalVec(int cid);  /** The dection of crystal*/

    int GetCellID(int ThetaId, int PhiId);//! Get Cell Id

    int GetCellID() {return mPar_cellID;};//! Get Cell Id
    int GetThetaID() {return mPar_thetaID;};//! Get Theta Id
    int GetPhiID() {return mPar_phiID;};//! Get Phi Id

  private:

    double m_BLThetaCrystal[46];  /** The Theta angle of  crystal derection for Barrel crystals */
    double m_BLPhiCrystal[46];    /** The Phi angle of  crystal derection  for Barrel crystals */
    double m_BLPreppos[46];       /** The Prep of  crystal derection  for Barrel crystals */
    double m_BLPhipos[46];        /** The Theta angle of  crystal position  for Barrel crystals */
    double m_BLZpos[46];          /** The Z of  crystal position  for Barrel crystals */

    double m_ECThetaCrystal[132]; /** The Theta angle of  crystal derection  for Endcap crystals */
    double m_ECPhiCrystal[132];   /** The Phi angle of  crystal derection  for Endcap crystals */
    double m_ECThetapos[132];     /** The Theta angle of  crystal position  for Endcap crystals */
    double m_ECPhipos[132];       /** The Phi angle of  crystal position  for Endcap crystals */
    double m_ECRpos[132];         /** The R angle of  crystal position  for Endcap crystals */

    int mPar_cellID;           /** The Cell ID information*/
    int mPar_thetaID;          /** The Theta ID information*/
    int mPar_phiID;            /** The Phi ID information*/
    int mPar_thetaIndex;       /** The Theta Index information*/
    int mPar_phiIndex;         /** The Phi IndeX information*/

    static ECLGeometryPar* m_B4ECLGeometryParDB; /*!< Pointer that saves the instance of this class. */
  };



  class EclNbr {
    // friend classses and functions

  public:
    // constants, enums and typedefs
///

    typedef EclIdentifier Identifier ;

    /// Constructors and destructor
    EclNbr();
///
    EclNbr(const EclNbr& aNbr);
///
    EclNbr(
      const std::vector< Identifier >&           aNbrs     ,
      const std::vector< Identifier >::size_type aNearSize
    ) ;
///
    virtual ~EclNbr();

    // member functions

    // const member functions
///
    const std::vector< Identifier >&                nbrs()      const ;
///
    const std::vector< Identifier >::const_iterator nearBegin() const ;
///
    const std::vector< Identifier >::const_iterator nearEnd()   const ;
///
    const std::vector< Identifier >::const_iterator nextBegin() const ;
///
    const std::vector< Identifier >::const_iterator nextEnd()   const ;
///
    const std::vector< Identifier >::size_type      nearSize()  const ;
///
    const std::vector< Identifier >::size_type      nextSize()  const ;

    /// assignment operator(s)
    const EclNbr& operator=(const EclNbr& aNbr);
    EclNbr getNbr(const Identifier  aCellId);
    void printNbr();
    //! Mapping theta, phi Id
    void Mapping(int cid);
    int GetCellID(int ThetaId, int PhiId);//! Get Cell Id

    int GetCellID() {return mNbr_cellID;};//! Get Cell Id
    int GetThetaID() {return mNbr_thetaID;};//! Get Theta Id
    int GetPhiID() {return mNbr_phiID;};//! Get Phi Id

  protected:
    // protected member functions

    // protected const member functions

  private:
    // Constructors and destructor

    // private member functions

    // private const member functions

    // data members
    int mNbr_cellID;           /** The Cell ID information*/
    int mNbr_thetaID;          /** The Theta ID information*/
    int mNbr_phiID;            /** The Phi ID information*/
    std::vector< Identifier >&           m_nbrs     ;
    std::vector< Identifier >::size_type m_nearSize ;

    // static data members

  };


  class TEclEnergyHit {
    // friend classes and functions

  public:
    // constants, enums and typedefs
    ///
    typedef EclIdentifier Identifier;
//      has trouble with g++ stl, poor compiler
//      typedef int Identifier;

    /// Constructors and destructor
    TEclEnergyHit() : fId(0), fEnergy(0), fCellId(0) {
    }
    ///
    TEclEnergyHit(const TEclEnergyHit& ahit)
        : fId(ahit.Id()), fEnergy(ahit.Energy()), fCellId(ahit.CellId()) {
    }
    ///
    TEclEnergyHit(
      const Identifier hid, const EclGeV energy, const Identifier cid)
        : fId(hid), fEnergy(energy), fCellId(cid) {
    }
    ///
    virtual ~TEclEnergyHit() { }

    /// assignment operator(s)
    const TEclEnergyHit& operator=(const TEclEnergyHit& hit) {
      if (this != &hit) {
        fId = hit.fId;
        fEnergy = hit.fEnergy;
        fCellId = hit.fCellId;
      }
      return *this;
    }

    // member functions
    ///
    const Identifier Id(void) const {
      return fId;
    }
    ///
    Identifier Id(Identifier id) {
      return fId = id;
    }
    ///
    const Identifier CellId(void) const {
      return fCellId;
    }
    ///
    Identifier CellId(int cId) {
      return fCellId = cId;
    }



    ///
    const EclGeV Energy(void) const {
      return fEnergy;
    }
    ///
    EclGeV Energy(EclGeV energy) {
      return fEnergy = energy;
    }

    // const member functions

    // static member functions

    /// comparison operators
    bool operator<(const TEclEnergyHit& rhs) const {
      return
        fId < rhs.Id();
    }
    ///
    bool operator>(const TEclEnergyHit& rhs) const {
      return
        fId > rhs.Id();
    }
    ///
    bool operator==(const TEclEnergyHit& rhs) const {
      return
        (fId == rhs.Id())
        && (fEnergy == rhs.Energy())
        && (fCellId == rhs.CellId())
        ;
    }
    ///
    bool operator!=(const TEclEnergyHit& rhs) const {
      return
        (fId != rhs.Id())
        || (fEnergy != rhs.Energy())
        || (fCellId != rhs.CellId())
        ;
    }

    ///
    struct less_Energy {
      ///
      bool operator()(const TEclEnergyHit& lhs, const TEclEnergyHit& rhs)
      const {
        return
          lhs.Energy() < rhs.Energy();
      }
    };
    ///
    struct less_CellId {
      ///
      bool operator()(const TEclEnergyHit& lhs, const TEclEnergyHit& rhs)
      const {
        return
          lhs.CellId() < rhs.CellId();
      }
    };
    ///
    struct greater_Energy {
      ///
      bool operator()(const TEclEnergyHit& lhs, const TEclEnergyHit& rhs)
      const {
        return
          lhs.Energy() > rhs.Energy();
      }
    };
    ///
    struct greater_CellId {
      ///
      bool operator()(const TEclEnergyHit& lhs, const TEclEnergyHit& rhs)
      const {
        return
          lhs.CellId() > rhs.CellId();
      }
    };


  protected:
    // protected member functions

    // protected const member functions

  private:
    // Constructors and destructor


    // private member functions

    // private const member functions
    // private const member functions

    // data members
    Identifier fId;
    EclGeV fEnergy;
    int fCellId;
    // static data members

  };

///
  typedef std::map < TEclEnergyHit::Identifier, TEclEnergyHit,
  std::less<TEclEnergyHit::Identifier> > EclEnergyHitMap;

} // end of namespace Belle2

#endif
