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
#include <G4String.hh>

#include "TVector3.h"
/** tyep define EclIdentifier */
typedef int EclIdentifier ;
/** tyep define EclGeV */
typedef double     EclGeV        ;
/** tyep define EclGeV */
typedef double   EclCM         ;
/** tyep define EclCC */
typedef double     EclCC         ;
/** tyep define EclKG */
typedef double     EclKG         ;
/** tyep define EclRad */
typedef double     EclRad        ;
/** tyep define EclDeg */
typedef double     EclDeg        ;


namespace Belle2 {
  namespace ECL {

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

      /** The Postion of crystal*/
      TVector3 GetCrystalPos(int cid);
      /** The direction of crystal*/
      TVector3 GetCrystalVec(int cid);

      /** Get Cell Id */
      int GetCellID(int ThetaId, int PhiId);
      /** Get Cell Id */
      int GetCellID() {return mPar_cellID;};
      /** Get Theta Id */
      int GetThetaID() {return mPar_thetaID;};
      /** Get Phi Id */
      int GetPhiID() {return mPar_phiID;};
      /** Get Cell Id */
      int ECLVolNameToCellID(const G4String VolumeName);//Mapping from VolumeName to Crystal CellID

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
      /** constants, enums and typedefs */
      typedef EclIdentifier Identifier ;

      /** Constructors and destructor */
      EclNbr();
      /** Constructor of EclNbr */
      EclNbr(const EclNbr& aNbr);
      /** Constructor of EclNbr */
      EclNbr(
        const std::vector< Identifier >&           aNbrs     ,
        const std::vector< Identifier >::size_type aNearSize
      ) ;
///
      virtual ~EclNbr();

      // member functions

      // const member functions
      /** get crystals nbrs */
      const std::vector< Identifier >&                nbrs()      const ;
      /** get crystals nearBegin */
      const std::vector< Identifier >::const_iterator nearBegin() const ;
      /** get crystals nearEnd */
      const std::vector< Identifier >::const_iterator nearEnd()   const ;
      /** get crystals nextBegin */
      const std::vector< Identifier >::const_iterator nextBegin() const ;
      /** get crystals nextEnd */
      const std::vector< Identifier >::const_iterator nextEnd()   const ;
      /** get crystals nearSize */
      std::vector< Identifier >::size_type      nearSize()  const ;
      /**get crystals nextSize */
      std::vector< Identifier >::size_type      nextSize()  const ;

      /// assignment operator(s)
      const EclNbr& operator=(const EclNbr& aNbr);
      /** get crystals nbr */
      EclNbr getNbr(const Identifier  aCellId);
      /** print crystals nbrs */
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

      /** data members */
      int mNbr_cellID;           /** The Cell ID information*/
      int mNbr_thetaID;          /** The Theta ID information*/
      int mNbr_phiID;            /** The Phi ID information*/
      std::vector< Identifier >&           m_nbrs     ; /** id  of m_brs */
      std::vector< Identifier >::size_type m_nearSize ;/**  size of near brs*/

      // static data members

    };

    /**  define class TEclEnergyHit */
    class TEclEnergyHit {
      // friend classes and functions

    public:
      // constants, enums and typedefs
      ///
      typedef EclIdentifier Identifier;
//      has trouble with g++ stl, poor compiler
//      typedef int Identifier;

      /// Constructors and destructor
      /** Constructor of TEclEnergyHit */
      TEclEnergyHit() : fId(0), fEnergy(0), fCellId(0) {
      }
      /** Constructor of TEclEnergyHit */
      TEclEnergyHit(const TEclEnergyHit& ahit)
        : fId(ahit.Id()), fEnergy(ahit.Energy()), fCellId(ahit.CellId()) {
      }
      /** Constructor of TEclEnergyHit */
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
      /** get Id */
      const Identifier Id(void) const {
        return fId;
      }
      /** set Id */
      Identifier Id(Identifier id) {
        return fId = id;
      }
      /** get Cell Id */
      const Identifier CellId(void) const {
        return fCellId;
      }
      /** set Cell Id */
      Identifier CellId(int cId) {
        return fCellId = cId;
      }



      /** get Energy  */
      const EclGeV Energy(void) const {
        return fEnergy;
      }
      /** set Energy  */
      EclGeV Energy(EclGeV energy) {
        return fEnergy = energy;
      }

      // const member functions

      // static member functions

      /** comparison operators */
      bool operator<(const TEclEnergyHit& rhs) const {
        return
          fId < rhs.Id();
      }
      /** comparison operators */
      bool operator>(const TEclEnergyHit& rhs) const {
        return
          fId > rhs.Id();
      }
      /** comparison operators */
      bool operator==(const TEclEnergyHit& rhs) const {
        return
          (fId == rhs.Id())
          && (fEnergy == rhs.Energy())
          && (fCellId == rhs.CellId())
          ;
      }
      /** comparison operators */
      bool operator!=(const TEclEnergyHit& rhs) const {
        return
          (fId != rhs.Id())
          || (fEnergy != rhs.Energy())
          || (fCellId != rhs.CellId())
          ;
      }

      /** struct less_Energy */
      struct less_Energy {
        /** operator */
        bool operator()(const TEclEnergyHit& lhs, const TEclEnergyHit& rhs)
        const {
          return
            lhs.Energy() < rhs.Energy();
        }
      };
      /** struct less_CellId */
      struct less_CellId {
        /** operator */
        bool operator()(const TEclEnergyHit& lhs, const TEclEnergyHit& rhs)
        const {
          return
            lhs.CellId() < rhs.CellId();
        }
      };
      /** struct greater_Energy  */
      struct greater_Energy {
        /** operator */
        bool operator()(const TEclEnergyHit& lhs, const TEclEnergyHit& rhs)
        const {
          return
            lhs.Energy() > rhs.Energy();
        }
      };
      /** struct greater_CellId */
      struct greater_CellId {
        /** operator */
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

      /** data members */
      Identifier fId; /**  Id  */
      EclGeV fEnergy; /**  Energy  */
      int fCellId;    /**  Cell Id  */
      // static data members

    };


    /**  define EclEnergyHitMap */
    typedef std::map < TEclEnergyHit::Identifier, TEclEnergyHit,
            std::less<TEclEnergyHit::Identifier> > EclEnergyHitMap;

  } // end of namespace ECL
} // end of namespace Belle2

namespace Belle2 {
  /**  function of G4  VolumeName to Crystal CellID */
  int ECLG4VolNameToCellID(const G4String VolumeName);//Mapping from VolumeName to Crystal CellID
} // end of namespace Belle2

#endif
