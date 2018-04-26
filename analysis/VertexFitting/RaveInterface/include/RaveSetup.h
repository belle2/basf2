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

#include <framework/logging/Logger.h>

//root
#include <TVector3.h>
#include <TMatrixDSym.h>

namespace rave {
  class KinematicTreeFactory;
  class VertexFactory;
}

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
      static RaveSetup* getInstance()
      {
        RaveSetup* instance = getRawInstance();
        if (!instance->m_initialized) {
          B2FATAL("RaveSetup::initialize was not called. It has to be called before RaveSetup or RaveVertexFitter are used");
        }
        return instance;
      }
      /** Set everything up so everything needed for vertex fitting is there. Must be called before RaveVertexFitter can be used*/
      static void initialize(int verbosity = 1, double MagneticField = 1.5);
      /** The beam spot position and covarance is known you can set it here so that and a vertex in the beam spot should be fitted you can set it here */
      void setBeamSpot(TVector3 beamSpot, TMatrixDSym beamSpotCov);
      /** unset beam spot constraint */
      void unsetBeamSpot();

      /** frees memory allocated by initialize(). Beam spot is not modified. */
      void reset();

      /** Print() writes all RaveSetup member variables to the terminal  */
      static void Print();

    protected:
      /** Same as getInstance(), but no check if the instance is initialised. */
      static RaveSetup* getRawInstance();
      /** default constructor */
      RaveSetup();
      /** default destructor */
      ~RaveSetup();

      bool m_useBeamSpot; /**< flag determines if beam spot information should be used for vertex fit. Can be overruled by individual fits in RaveVertexFitter */
      TVector3 m_beamSpot; /**< beam spot position. Can be used as additional information by RaveVertexFitter */
      TMatrixDSym m_beamSpotCov; /**< beam spot position covariance matrix. Can be used as additional information by RaveVertexFitter */

      rave::VertexFactory*
      m_raveVertexFactory; /**< The RAVE vertex factory is the principal interface offered by the RAVE vertex fitting library. */

      //GFRaveVertexFactory* m_GFRaveVertexFactory;
      /**< The RAVE Kinematic Tree factory is the principal interface offered by the RAVE for kinematic vertex fitting. */
      rave::KinematicTreeFactory* m_raveKinematicTreeFactory;

      bool m_initialized; /**< Has initialize() been called? unusable otherwise. */
    };

  }

}

#endif
