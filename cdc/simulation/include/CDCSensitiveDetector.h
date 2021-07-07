/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <simulation/kernel/SensitiveDetectorBase.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/WireID.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>

#include <vector>
#include <map>


namespace Belle2 {
  class CDCSimHit;
  namespace CDC {

    class CDCGeometryPar;

    //! The Class for CDC Sensitive Detector
    /*! In this class, every variable defined in CDCB4VHit will be calculated.
      And save CDCB4VHit into hits collection.
    */

    class CDCSensitiveDetector: public Simulation::SensitiveDetectorBase {

    public:

      //! Constructor
      CDCSensitiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy);

      //! Destructor
      ~CDCSensitiveDetector() {}

      //! Register CDC hits collection into G4HCofThisEvent
      void Initialize(G4HCofThisEvent*) override;

      //! Process each step and calculate variables defined in CDCB4VHit
      bool step(G4Step* aStep, G4TouchableHistory* history) override;

      //!  Do what you want to do at the beginning of each event (why this is not called ?)
      //      void BeginOfEvent(G4HCofThisEvent*);

      //!  Do what you want to do at the end of each event
      void EndOfEvent(G4HCofThisEvent*) override;

      //! Save CDCSimHit into datastore
      void saveSimHit(const G4int layerId,
                      const G4int wireId,
                      const G4int trackID,
                      const G4int pid,
                      const G4double distance,
                      const G4double tof,
                      const G4double edep,
                      const G4double stepLength,
                      const G4ThreeVector& mom,
                      const G4ThreeVector& posW,
                      const G4ThreeVector& posIn,
                      const G4ThreeVector& posOut,
                      const G4ThreeVector& posTrack,
                      const G4int lr,
                      const G4int NewLrRaw,
                      const G4int NewLr,
                      const G4double speed,
                      const G4double hitWeight);

      //void AddbgOne(bool doit);

    private:

      //! Assume line track to calculate  distance between track and wire (drift length).
      G4double ClosestApproach(G4ThreeVector bwp, G4ThreeVector fwp, G4ThreeVector posIn, G4ThreeVector posOut,
                               G4ThreeVector& hitPosition, G4ThreeVector& wirePosition);//,G4double& transferT);

      //! Calculate intersection of track with cell boundary.
      /*!

        (Relations)

        (Arguments)
        input
        ic1        integer * 4   #cell(sirial) of entrance.
        ic2        integer * 4   #cell(sirial) of exit.
        venter(6)  real * 4      (x,y,z,px/p,py/p,pz/p) at entrance.
        vexit(6)   real * 4      (x,y,z,px/p,py/p,pz/p) at exit.
        s1         real * 4      track length at entrance.
        s2         real * 4      track length at exit.
        output
        xint(6)    real * 4      (x,y,z,px/p,py/p,pz/p) at intersection of
        cell boundary.
        sint       real * 4      track length at intersection of cell
        boundary.
        iflag      integer * 4   return code from GIPLAN.
      */
      void CellBound(const G4int layerId, const G4int ic1, const G4int ic2,
                     const G4double venter[6], const G4double vexit[6],
                     const G4double s1, const G4double s2,
                     G4double xint[6], G4double& sint, G4int& iflag);


      //!
      /*!
      ******************************************************************
      *                                                                *
      *       Calculates a cubic through P1,(-X,Y1),(X,Y2),P2          *
      *       where Y2=-Y1                                             *
      *        Y=A(1)+A(2)*X+A(3)*X**2+A(4)*X**3                       *
      *        The coordinate system is assumed to be the cms system   *
      *        of P1,P2.                                               *
      *                                                                *
      *    ==>Called by : GIPLAN,GICYL                                 *
      *       Author    H.Boerner  *********                           *
      *                                                                *
      ******************************************************************
      */
      void GCUBS(const G4double x, const G4double y, const G4double d1, const G4double d2, G4double a[4]);

      /**
       * Calculates a rotation matrix.
       * Calculates a rotation matrix. in advance at a local position in lab.
       * frame.
       * The rotation is done about the coord. origin; lab.-frame to B-field
       * frame in which only Bz-comp. is non-zero.
       */
      void for_Rotat(const G4double bfld[3]);

      /**
       * Translation method.
       * Translates (x,y,z) in lab. to (x,y,z) in B-field frame (mode=1),
       * or reverse translation (mode=-1).
       */

      void Rotat(G4double& x, G4double& y,
                 G4double& z,
                 const int mode);

      /**
       * Overloaded translation method.
       */
      void Rotat(G4double x[3], const int mode);

      //! Calculate closest points between helix and wire.
      /*!
        Input
        xwb4 : x of wire at backward endplate in lab.
        ywb4 : y of wire at backward endplate   "
        zwb4 : z of wire at backward endplate   "
        xwf4 : x of wire at forward  endplate   "
        ywf4 : y of wire at forward  endplate   "
        zwf4 : z of wire at forward  endplate   "
        xp   : x of helix in lab.
        yp   : y of helix   "
        zp   : z of helix   "
        px   : px of helix in lab.
        py   : py of helix   "
        pz   : pz of helix   "

        Output
        q2(1) : x of wire  at closest point in lab.
        q2(2) : y of wire  at closest point   "
        q2(3) : z of wire  at closest point   "
        q1(1) : x of helix at closest point   "
        q1(2) : y of helix at closest point   "
        q1(3) : z of helix at closest point   "
        q3    : momentum of helix at closest point in lab.
        ntry  :
      */
      void HELWIR(const G4double xwb4,
                  const G4double ywb4,
                  const G4double zwb4,
                  const G4double xwf4,
                  const G4double ywf4,
                  const G4double zwf4,
                  const G4double xp,
                  const G4double yp,
                  const G4double zp,
                  const G4double px,
                  const G4double py,
                  const G4double pz,
                  const G4double B_kG[3],
                  const G4double charge,
                  const G4int ntryMax,
                  G4double& distance,
                  G4double q2[3],
                  G4double q1[3],
                  G4double q3[3],
                  G4int& ntry);

      //! Calculate the result of a matrix times vector.
      /*!
        Input
        ndim             : dimension
        b(1-ndim)        : vector
        m(1-ndim,1-ndim) : matrix
        a(1-ndim)        : vector
        c(1-ndim)        : vector
        mode             : c = m * a     for mode=0
        c = b * m * a for mode=1
        Output
        c(1-ndim)        : for mode 1, solution is put on c[0]
      */
      void Mvopr(const G4int ndim, const G4double b[3], const G4double m[3][3],
                 const G4double a[3], G4double c[3], const G4int mode);

      //! Sort wire id.
      std::vector<int>  WireId_in_hit_order(int id0, int id1, int nWires);

      //!  set left/right flag modified for tracking
      void setModifiedLeftRightFlag();

      //!  Re-assign left/right info.
      void reAssignLeftRightInfo();

      /**
       * Check if neighboring cell in the same super-layer; essentially a copy from cdcLocalTracking/mclookup.
       * @param[in] wireId wire-id. in question (reference)
       * @param[in] otherWireId another wire-id. in question
       */
      unsigned short areNeighbors(const WireID& wireId, const WireID& otherWireId) const;

      /**
       * Check if neighboring cell in the same super-layer; essentially a copy from cdcLocalTracking/mclookup.
       * @param[in] iCLayer later-id (continuous) in question (reference)
       * @param[in] iSuperLayer super-later-id in question (reference)
       * @param[in] iLayer later-id in the super-layer in question (reference)
       * @param[in] iWire wire-id in the layer in question (reference)
       * @param[in] otherWireId another wire-id. in question
       */
      unsigned short areNeighbors(unsigned short iCLayer, unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire,
                                  const WireID& otherWireId) const;


      /** MC particles. */
      StoreArray<MCParticle> m_MCParticles;

      /** CDC simulation hits. */
      StoreArray<CDCSimHit> m_CDCSimHits;

      /**
       * Magnetic field is on or off.
       */
      G4int m_magneticField;

      /**
       * Magnetic field is uniform or non-uniform.
       * 0: uniform B field (1.5 T), 1: non-uniform B field.
       */
      G4int m_nonUniformField;

      G4double m_brot[3][3];  /*!< a rotation matrix. */

      //      CDCGeometryPar& m_cdcgp; /**< Reference to CDCGeometryPar object. */
      CDCGeometryPar* m_cdcgp; /**< Pointer to CDCGeometryPar object. */

      /**
       * Threshold energy deposit to be stored.
       */
      G4double m_thresholdEnergyDeposit;

      /**
       * Threshold kinetic energy to be stored.
       */
      G4double m_thresholdKineticEnergy;

      G4bool m_wireSag; /**< Switch to activate wire sag effect. */

      G4bool m_modifiedLeftRightFlag; /**< Switch for left/right flag modified for tracking. */

      G4double m_minTrackLength; /**< Min. track length (mm) required for saving in MCParticle. */

      int m_hitNumber; /**< The current number of created hits in an event. Used to fill the DataStore CDC array.*/

      std::multimap<unsigned short, CDCSimHit*>
      m_hitWithPosWeight; /**< Map containing hits with positive weight. Map may be replaced by vector, which may make the job speed faster... Try later. */

      std::vector<CDCSimHit*> m_hitWithNegWeight; /**< Vector containing hits with negative weight. */

      //      std::vector<std::multimap<unsigned short, CDCSimHit*>::iterator> m_posWeightMapItBegin; /**< Iterator showing begin of positive hit map*/

      //      std::vector<std::multimap<unsigned short, CDCSimHit*>::iterator> m_posWeightMapItEnd; /**< Iterator showing end of positive hit map*/

      //      int m_nPosHits, m_nNegHits;

      const signed short CCW = 1; ///< Constant for counterclockwise orientation
      const signed short CW  = -1; ///< Constant for clockwise orientation
      const signed short CW_OUT_NEIGHBOR  = 1; ///< Constant for clockwise outwards
      const signed short CW_NEIGHBOR      = 3; ///< Constant for clockwise
      const signed short CW_IN_NEIGHBOR   = 5; ///< Constant for clockwise inwards
      const signed short CCW_IN_NEIGHBOR  = 7; ///< Constant for counterclockwise inwards
      const signed short CCW_NEIGHBOR     = 9; ///< Constant for counterclockwise
      const signed short CCW_OUT_NEIGHBOR = 11; ///< Constant for counterclockwise outwards
    };
  }
} // end of namespace Belle2
