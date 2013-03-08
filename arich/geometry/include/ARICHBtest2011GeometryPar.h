/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHBTEST2011GEOMETRYPAR_H
#define ARICHBTEST2011GEOMETRYPAR_H

#include <vector>
#include <string>
#include <map>
#include <framework/gearbox/GearDir.h>
#include "TRotation.h"
#include "TVector3.h"
#include "TVector2.h"
#include <G4ThreeVector.hh>
#include <G4MaterialPropertyVector.hh>
#include <cmath>
#include <boost/format.hpp>


namespace Belle2 {
  class ARICHTracking {
  public:
    ARICHTracking() {};
    ~ARICHTracking() {};
    int tdc[4], atdc;
    float slp[2], offset[2];
    int cutll[2], cutul[2];
    float pos[3];

    int diff[2];
    int sum[2];
    int status[2];
    double reco[3];
    void Print() {
      printf("----------------------------------------------------------------\n");
      for (int i = 0; i < 4; i++) printf("tdc%d= %d\t", i, tdc[i]); printf("\n");
      for (int i = 0; i < 1; i++) printf("atdc%d= %d\t", i, atdc); printf("\n");
      for (int i = 0; i < 2; i++) printf("slp%d= %3.3f\t", i, slp[i]); printf("\n");
      for (int i = 0; i < 2; i++) printf("off%d= %3.3f\t", i, offset[i]); printf("\n");
      for (int i = 0; i < 2; i++) printf("cutll%d= %d\t", i, cutll[i]); printf("\n");
      for (int i = 0; i < 2; i++) printf("cutul%d= %d\t", i, cutul[i]); printf("\n");
      for (int i = 0; i < 3; i++) printf("pos%d= %3.3f\t", i, pos[i]); printf("\n");
    };
  };

  namespace arich {
    //! The Class for ARICH Geometry Parameters
    /*! This class provides ARICH gemetry paramters for simulation, reconstruction and so on.
      These parameters are gotten from gearbox.
    */


    class ARICHBtest2011GeometryPar {

    public:

      //! Constructor
      ARICHBtest2011GeometryPar();

      //! Destructor
      virtual ~ARICHBtest2011GeometryPar();

      //! Static method to get a reference to the ARICHBtest2011GeometryPar instance.
      /*!
      \return A reference to an instance of this class.
      */
      static ARICHBtest2011GeometryPar* Instance();
      //! calculates detector parameters needed for geometry build and reconstruction.
      void Initialize(const GearDir& content);
      //! calculates detector parameters needed for geometry build and reconstruction.
      void Initialize(const GearDir& modulecontent, const GearDir& mirrorcontent);


      //! Clears
      void clear(void);

      //! Print some debug information
      void Print(void) const;

      //! gets geometry parameters from gearbox.
      void read(const GearDir& content);


      void setTrackingShift(TVector3&);
      TVector3 getTrackingShift();
      void setRotationCenter(TVector3&);
      TVector3 getRotationCenter();
      void setFrameRotation(double);
      TRotation getFrameRotation();
      void setAverageAgel(bool);
      bool getAverageAgel();
      void setMwpc(ARICHTracking* m_mwpc);
      ARICHTracking* getMwpc();
    private:
      ARICHTracking* m_mwpc;
      TVector3 m_trackingShift;
      TVector3 m_rotationCenter;
      TRotation   m_frameRotation;
      bool      m_averageAgel;


      int m_nPads;                           /*!< Number of detector module pads */

      bool m_init;                          /*!< True if parametrization is already initialized */
      bool m_simple;                        /*!< True if parametrization initialized with simple geometry (beamtest) */



      static ARICHBtest2011GeometryPar* p_B4ARICHBtest2011GeometryParDB; /*!< Pointer that saves the instance of this class. */

      // vectors holding information on HAPDs and chips and pads positions.


    };

    //-----------------------------------------------------------------------------






  } // end of namespace arich
} // end of namespace Belle2

#endif

