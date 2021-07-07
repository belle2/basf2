/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <map>
#include <G4ThreeVector.hh>
#include <TVector3.h>

typedef int EclIdentifier;
typedef double EclGeV;
typedef double EclCM;
typedef double EclRad;

class G4VTouchable;
namespace HepGeom {
  class Transform3D;
}
typedef HepGeom::Transform3D G4Transform3D;

namespace Belle2 {
  namespace ECL {


    //! The Class for ECL Geometry Parameters
    /*! This class provides ECL gemetry parameters for simulation, reconstruction and so on.
        These parameters are got from geometry description
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

      //! Gets geometry parameters from PhysicalVolumeStore
      void read();

      //! Mapping theta, phi Id
      void Mapping(int cid);

      /** Get Cell Id */
      int GetCellID(int ThetaId, int PhiId);
      /** Get Cell Id */
      int GetCellID() {return mPar_cellID;};
      /** Get Theta Id */
      int GetThetaID() {return mPar_thetaID;};
      /** Get Phi Id */
      int GetPhiID() {return mPar_phiID;};
      /** Get Cell Id (LEP: new way)*/
      int ECLVolumeToCellID(const G4VTouchable*); /**< Mapping from G4VTouchable copyNumbers to Crystal CellID */
      int TouchableToCellID(const G4VTouchable*); /**< The same as above but without sanity checks */

      int TouchableDiodeToCellID(const G4VTouchable*); /**< Mapping from G4VTouchable copyNumbers to Crystal CellID */

      /** The Position of crystal*/
      G4ThreeVector getCrystalPos(int cid)
      {
        if (cid != m_ini_cid) InitCrystal(cid);
        return m_current_crystal.pos;
      }

      /** The direction of crystal*/
      G4ThreeVector getCrystalVec(int cid)
      {
        if (cid != m_ini_cid) InitCrystal(cid);
        return m_current_crystal.dir;
      }

      /** The Position of crystal*/
      TVector3 GetCrystalPos(int cid)
      {
        if (cid != m_ini_cid) InitCrystal(cid);
        const G4ThreeVector& t = m_current_crystal.pos;
        return TVector3(t.x(), t.y(), t.z());
      }

      /** The direction of crystal*/
      TVector3 GetCrystalVec(int cid)
      {
        if (cid != m_ini_cid) InitCrystal(cid);
        const G4ThreeVector& t = m_current_crystal.dir;
        return TVector3(t.x(), t.y(), t.z());
      }

      double time2sensor(int cid, const G4ThreeVector& hit_pos);
    private:
      /** Global transformations for the forward part*/
      G4Transform3D* m_ECLForwardGlobalT = nullptr;
      /** Global transformations for the backward part*/
      G4Transform3D* m_ECLBackwardGlobalT = nullptr;
      /** Global transformations for the barrel part*/
      G4Transform3D* m_ECLBarrelGlobalT = nullptr;
      /** initialise the crystal */
      void InitCrystal(int cid);
      /** crystal geometry */
      struct CrystalGeom_t {
        G4ThreeVector pos, dir;
      };
      /** the crystals */
      std::vector<CrystalGeom_t> m_crystals;
      /** the current crystal */
      CrystalGeom_t m_current_crystal;
      /** The Cell ID information*/
      int mPar_cellID;
      /** The Theta ID information*/
      int mPar_thetaID;
      /** The Phi ID information*/
      int mPar_phiID;
      /** . */
      int m_ini_cid;
      /** Pointer that saves the instance of this class. */
      static ECLGeometryPar* m_B4ECLGeometryParDB;
    };


    /** EclNbr class  */
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

      /** destructor */
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
      EclNbr& operator=(const EclNbr& aNbr);
      /** get crystals nbr */
      EclNbr getNbr(const Identifier  aCellId);
      /** print crystals nbrs */
      void printNbr();
      //! Mapping theta, phi Id
      void Mapping(int cid);
      int GetCellID(int ThetaId, int PhiId);/**< Get Cell Id */

      int GetCellID() {return mNbr_cellID;};/**< Get Cell Id */
      int GetThetaID() {return mNbr_thetaID;};/**< Get Theta Id */
      int GetPhiID() {return mNbr_phiID;};/**< Get Phi Id */

    protected:
      // protected member functions

      // protected const member functions

    private:
      // Constructors and destructor

      // private member functions

      // private const member functions

      /** data members */
      int mNbr_cellID;           /**< The Cell ID information*/
      int mNbr_thetaID;          /**< The Theta ID information*/
      int mNbr_phiID;            /**< The Phi ID information*/
      std::vector< Identifier >&           m_nbrs     ; /**< id  of m_brs */
      std::vector< Identifier >::size_type m_nearSize ;/**<  size of near brs*/

      // static data members

    };

    /**  define class TEclEnergyHit */
    class TEclEnergyHit {
      // friend classes and functions

    public:
      // constants, enums and typedefs

      /** type define Identifier */
      typedef EclIdentifier Identifier;

      /// Constructors and destructor
      /** Constructor of TEclEnergyHit */
      TEclEnergyHit() : fId(0), fEnergy(0), fCellId(0)
      {
      }
      /** Constructor of TEclEnergyHit */
      TEclEnergyHit(const TEclEnergyHit& ahit)
        : fId(ahit.Id()), fEnergy(ahit.Energy()), fCellId(ahit.CellId())
      {
      }
      /** Constructor of TEclEnergyHit */
      TEclEnergyHit(
        const Identifier hid, const EclGeV energy, const Identifier cid)
        : fId(hid), fEnergy(energy), fCellId(cid)
      {
      }
      /// destructor
      virtual ~TEclEnergyHit() { }

      /// assignment operator(s)
      TEclEnergyHit& operator=(const TEclEnergyHit& hit)
      {
        if (this != &hit) {
          fId = hit.fId;
          fEnergy = hit.fEnergy;
          fCellId = hit.fCellId;
        }
        return *this;
      }

      // member functions
      /** get Id */
      Identifier Id(void) const
      {
        return fId;
      }
      /** set Id */
      Identifier Id(Identifier id)
      {
        return fId = id;
      }
      /** get Cell Id */
      Identifier CellId(void) const
      {
        return fCellId;
      }
      /** set Cell Id */
      Identifier CellId(int cId)
      {
        return fCellId = cId;
      }



      /** get Energy  */
      EclGeV Energy(void) const
      {
        return fEnergy;
      }
      /** set Energy  */
      EclGeV Energy(EclGeV energy)
      {
        return fEnergy = energy;
      }

      // const member functions

      // static member functions

      /** comparison operators */
      bool operator<(const TEclEnergyHit& rhs) const
      {
        return
          fId < rhs.Id();
      }
      /** comparison operators */
      bool operator>(const TEclEnergyHit& rhs) const
      {
        return
          fId > rhs.Id();
      }
      /** comparison operators */
      bool operator==(const TEclEnergyHit& rhs) const
      {
        return
          (fId == rhs.Id())
          && (fEnergy == rhs.Energy())
          && (fCellId == rhs.CellId())
          ;
      }
      /** comparison operators */
      bool operator!=(const TEclEnergyHit& rhs) const
      {
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
        const
        {
          return
            lhs.Energy() < rhs.Energy();
        }
      };
      /** struct less_CellId */
      struct less_CellId {
        /** operator */
        bool operator()(const TEclEnergyHit& lhs, const TEclEnergyHit& rhs)
        const
        {
          return
            lhs.CellId() < rhs.CellId();
        }
      };
      /** struct greater_Energy  */
      struct greater_Energy {
        /** operator */
        bool operator()(const TEclEnergyHit& lhs, const TEclEnergyHit& rhs)
        const
        {
          return
            lhs.Energy() > rhs.Energy();
        }
      };
      /** struct greater_CellId */
      struct greater_CellId {
        /** operator */
        bool operator()(const TEclEnergyHit& lhs, const TEclEnergyHit& rhs)
        const
        {
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
      Identifier fId; /**< Id  */
      EclGeV fEnergy; /**< Energy  */
      int fCellId;    /**< Cell Id  */
      // static data members

    };

    /**  define EclEnergyHitMap */
    typedef std::map <TEclEnergyHit::Identifier, TEclEnergyHit,
            std::less<TEclEnergyHit::Identifier> > EclEnergyHitMap;

  } // end of namespace ECL
} // end of namespace Belle2
