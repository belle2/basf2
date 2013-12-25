/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 * 2012.03.05 SensitiveDetector -> CDCSensitiveDetector by M. Uchida      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCSensitiveDetector_H
#define CDCSensitiveDetector_H

//#include <cdc/simcdc/CDCB4VHit.h>
#include <simulation/kernel/SensitiveDetectorBase.h>



namespace Belle2 {
  namespace CDC {
    //! The Class for CDC Sensitive Detector
    /*! In this class, every variable defined in CDCB4VHit will be calculated.
      And save CDCB4VHit into hits collection.
    */

    // Wire configuration


    class CDCSensitiveDetector: public Simulation::SensitiveDetectorBase {

    public:

      //! Constructor
      CDCSensitiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy);

      //! Destructor
      ~CDCSensitiveDetector() {}

      //! Register CDC hits collection into G4HCofThisEvent
      void Initialize(G4HCofThisEvent*);

      //! Process each step and calculate variables defined in CDCB4VHit
      bool step(G4Step* aStep, G4TouchableHistory* history);

      //! Do what you want to do at the end of each event
      void EndOfEvent(G4HCofThisEvent*);

      //! Save CDCSimHit into datastore
      int saveSimHit(const G4int layerId,
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
                     const G4double speed);

      //! Save CDCEBSimHit into datastore
      int saveEBSimHit(const G4int layerId,
                       const G4double phi,
                       const G4int trackID,
                       const G4int pid,
                       const G4double edep,
                       const G4ThreeVector& mom);

      //void AddbgOne(bool doit);

    private:

      //! Assume line track to calculate  distance between track and wire (drift length).
      G4double ClosestApproach(G4ThreeVector bwp, G4ThreeVector fwp, G4ThreeVector posIn, G4ThreeVector posOut, G4ThreeVector& hitPosition, G4ThreeVector& wirePosition);//,G4double& transferT);

      //! Calculate intersection of track with cell boundary.
      /*!

        (Relations)
        Calls       RotVec, GIPLAN

        (Arguments)
        input
        ic1        integer * 4   #cell(sirial) of entrance.
        ic2        integer * 4   #cell(sirial) of exit.
        venter(6)  real * 4      (x,y,z,px/p,py/p,pz/p) at entrance.
        vexit(6)   real * 4      (x,y,z,px/p,py/p,pz/p) at exit.
        s1         real * 4      track length at entrance.
        s2         real * 4      track length at exit.
        ic         integer * 4   mode for GIPLAN ( ic=3: cubic model )
        output
        xint(6)    real * 4      (x,y,z,px/p,py/p,pz/p) at intersection of
        cell boundary.
        sint       real * 4      track length at intersection of cell
        boundary.
        iflag      integer * 4   return code from GIPLAN.
      */
      void CellBound(const G4int layerId, const G4int ic1, const G4int ic2,
                     const G4double venter[6], const G4double vexit[6],
                     const G4double s1, const G4double s2, const G4int ic,
                     G4double xint[6], G4double& sint, G4int& iflag);

      //! The method to rotate a vector
      /*!
        (Purpose)

        Rotate coordinate axes in such a way that the stareo wire is
        parallel to Z-axis in X-Z plane in the new coordinate sytem
        Rotation is done assuming Z=0 is the center of CDC (so trans-
        lation must be done before this routine if CDC not centered to
        origin of coordinate system).

        Rotation order;
        1) rotate in phi  : (Xw,Yw,0) --> (Xw',0,0), wire //to Y-Z       '
        2) rotate in theta: wire // to Z-axis

        (Input)
        MODE    = 0    : twice rotation (phi --> theta)
        = 1    : twice reverse rotation (-theta --> -phi)
        = 2    : once rotation (phi only)
        = 3    : once reverse rotation (-phi only)
        X       = X compnent of a vector
        Y       = Y compnent of a vector
        Z       = Z compnent of a vector
        phi     = phi
        theta   = theta
        (Output)
        X       = X compnent of a vector after rotation
        Y       = Y compnent of a vector after rotation
        Z       = Z compnent of a vector after rotation

        (Relation)
        None
      */
      void RotVec(G4double& x, G4double& y, G4double& z, const G4double phi, const G4double theta, const G4int mode);

      //! The method to calculate intersection of track with a plane
      /*!
      ******************************************************************
      *                                                                *
      *       Calculates intersection of track (X1,X2)                 *
      *       with plane parallel to (X-Z)                             *
      *        The track is approximated by a cubic in the             *
      *       track length.                                            *
      *       To improve stability, the coordinate system              *
      *       is shifted.                                              *
      *       input parameters                                         *
      *        YC    = Y COORDINATE OF PLANE                           *
      *        X1    = X,Y,Z,XP,YP,ZP OF 1ST POINT                     *
      *        X2    =                   2ND                           *
      *        S1(2) = S AT 1ST(2ND) POINT                             *
      *        IC    = 1 STRAIGHT LINE DEFINED BY X+XP                 *
      *        IC    = 2 STRAIGHT LINE DEFINED BY X1+X2                *
      *        IC    = 3 CUBIC MODEL                                   *
      *                                                                *
      *      output parameters                                         *
      *        XINT  = X,Y,Z,XP,YP,ZP AT INTERSECTION POINT            *
      *        SINT  = S AT INTERSECTION POINT                         *
      *        PZINT = PHI,Z,DPHI/DR,DZ/DR                             *
      *        IFLAG = 1 IF TRACK INTERSECTS PLANE                     *
      *              = 0 IF NOT                                        *
      *                                                                *
      *      Warning : the default accuracy is 10 microns. The value   *
      *      of EPSI must be changed for a better precision            *
      *                                                                *
      *    ==>Called by : <USER>, GUDIGI                               *
      *                                                                *
      *        Authors: R.BRUN/JJ.DUMONT from an original routine by   *
      *       H. BOERNER  KEK  OCTOBER 1982                            *
      *                                                                *
      *                                                                *
      ******************************************************************
      */
      void GIPLAN(const G4double yc, const G4double x1[6], const G4double x2[6],
                  const G4double s1, const G4double s2, const G4int ic,
                  G4double xint[6], G4double& sint, G4double pzint[4], G4int& iflag);

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

      //! Calculates a rotation matrix in advance at a local position in lab.
      /*!
        The rotation is done about the coord. origin; lab.-frame to B-field
        frame in which only Bz-comp. is non-zero.
      */
      void for_Rotat(const G4double bfld[3]);

      //! Translation method
      /*! Translates (x,y,z) in lab. to (x,y,z) in B-field frame (mode=1), or reverse
        translation (mode=-1).
      */
      void Rotat(G4double& x, G4double& y,
                 G4double& z,
                 const int mode);

      //! Overloaded translation method
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

        Output
        q2(1) : x of wire  at closest point in lab.
        q2(2) : y of wire  at closest point   "
        q2(3) : z of wire  at closest point   "
        q1(1) : x of helix at closest point   "
        q1(2) : y of helix at closest point   "
        q1(3) : z of helix at closest point   "
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
                  G4double q2[3], G4double q1[3],
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

      //! Sort wire id
      std::vector<int>  WireId_in_hit_order(int id0, int id1, int nWires);

      /**
       * Magnetic field is uniform or non-uniform.
       * 0: uniform B field (1.5 T), 1: non-uniform B field.
       */
      G4int m_nonUniformField;
      G4double alpha;       /**< Helix parameter alpha     */
      G4double brot[3][3];

    private:

      /**
       * Threshold energy deposit to be stored.
       */
      G4double m_thresholdEnergyDeposit;

      /**
       * Threshold kinetic energy to be stored.
       */
      G4double m_thresholdKineticEnergy;

      G4bool m_wireSag; /**< Switch to activate wire sag effect. */

      int m_hitNumber; /**< The current number of created hits in an event. Used to fill the DataStore CDC array.*/
      int m_EBhitNumber; /**< The current number of created hits in an event. Used to fill the DataStore CDC EB array.*/
    };
  }
} // end of namespace Belle2

#endif
