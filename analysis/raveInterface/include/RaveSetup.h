/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef RaveSetup_H
#define RaveSetup_H

//std
#include <string>

//Genfit
#include <genfit/Track.h>
#include <genfit/GFRaveVertexFactory.h>
//tracking
//#include <tracking/dataobjects/Track.h>
//basf2
#include <framework/logging/Logger.h>

//root
#include <TVector3.h>
#include <TMatrixDSym.h>

//rave
#include <rave/VacuumPropagator.h>
#include <rave/MagneticField.h>
#include <rave/ConstantMagneticField.h>
#include <rave/VertexFactory.h>
#include <rave/KinematicTreeFactory.h>

namespace Belle2 {

  namespace analysis {
    /**
     * The RaveSetup class is part of the RaveInterface together with RaveVertexFitter
     * It holds all global options for the RaveVertexFitter and also takes care that stuff like the
     * magnetic field is correctly set up in way Rave can access it.
     */
    class RaveSetup {

    public:
      friend class RaveVertexFitter;
      friend class RaveKinematicVertexFitter;
      /** get the pointer to the instance to get/set any of options stored in RaveSetup*/
      static RaveSetup* getInstance() {
        if (s_instance == NULL) {
          B2FATAL("RaveSetup::initialize was not called. It has to be called before RaveSetup or RaveVertexFitter are used");
        }
        return s_instance;
      }
      /** Set everything up so everything needed for vertex fitting is there. Must be called before RaveVertexFitter can be used*/
      static void initialize(int verbosity = 1, double MagneticField = 1.5);
      /** The beam spot position and covarance is known you can set it here so that and a vertex in the beam spot should be fitted you can set it here */
      void setBeamSpot(TVector3 beamSpot, TMatrixDSym beamSpotCov);

      /** Print() writes all RaveSetup member variables to the terminal  */
      static void Print();

    protected:
      /** default constructor */
      RaveSetup();
      /** default destructor */
      ~RaveSetup();
      static RaveSetup* s_instance; /**< pointer to unique and global object of this class (singleton) */

      bool m_useBeamSpot; /**< flag determines if beam spot information should be used for vertex fit. Can be overruled by individual fits in RaveVertexFitter */
      TVector3 m_beamSpot; /**< beam spot position. Can be used as additional information by RaveVertexFitter */
      TMatrixDSym m_beamSpotCov; /**< beam spot position covariance matrix. Can be used as additional information by RaveVertexFitter */

      rave::VertexFactory* m_raveVertexFactory; /**< The RAVE vertex factory is the principal interface offered by the RAVE vertex fitting library. */

      //GFRaveVertexFactory* m_GFRaveVertexFactory;
      /**< The RAVE Kinematic Tree factory is the principal interface offered by the RAVE for kinematic vertex fitting. */
      rave::KinematicTreeFactory* m_raveKinematicTreeFactory;




    };

  }

}

#endif
