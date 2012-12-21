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
#include <GFTrack.h>

//tracking
#include <tracking/dataobjects/Track.h>

#include <framework/logging/Logger.h>

//root
#include <TVector3.h>
#include <TMatrixDSym.h>



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
      /** get the pointer to the instance to get/set any of options stored in RaveSetup*/
      static RaveSetup* getInstance() {
        if (s_instance == NULL) {
          B2FATAL("RaveSetup::initialize was not called. It has to be called before RaveSetup or RaveVertexFitter are used");
        }
        return s_instance;
      }
      /** Set everything up so everything needed for vertex fitting is there. Must be called before RaveVertexFitter can be used*/
      static void initialize(std::string options = "default");
      /** The beam spot position and covarance is known you can set it here so that and a vertex in the beam spot should be fitted you can set it here */
      void setBeamSpot(TVector3 beamSpot, TMatrixDSym beamSpotCov);
      /** Select the propagation method used during vertex fitting.  True selects Genfit propation (can handle inhomogenoios mangetic field and material) false selects Rave's vacuum propagator*/
      void setGFPropagation(bool setGFProp = true) {
        m_gfPropagation = setGFProp;
      }
      /** Sets the verbosity level of the Rave Vertex fitter library. Will be passed to Rave when an actual vertex fit is executed  */
      void setRaveVerbosity(int verbosity) {
        m_raveVerbosity = verbosity;
      }
      /** Print() writes all RaveSetup member variables to the terminal  */
      void Print();


    protected:

      RaveSetup();
      ~RaveSetup();
      static RaveSetup* s_instance;// = NULL??;

      bool m_gfRave;
      bool m_gfPropagation;
      int m_raveVerbosity;

      bool m_setupComplete;

      bool m_useBeamSpot;
      TVector3 m_beamSpot;
      TMatrixDSym m_beamSpotCov;


      /* global stuff:

       * GFRave yes/no
       * magnetic field
       * use beamspot yes/no if yes beam spot info
       *

       */
    };

  }

}

#endif
