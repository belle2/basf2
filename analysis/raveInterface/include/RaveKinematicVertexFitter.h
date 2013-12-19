/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler, Luigi Li Gioi                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef RaveKinematicVertexFitter_H
#define RaveKinematicVertexFitter_H

#include <analysis/dataobjects/Particle.h>
#include <analysis/raveInterface/RaveSetup.h>

#include <tracking/dataobjects/TrackFitResult.h>
//std
#include <string>

//root
#include <TMatrixFSym.h>
//Genfit
#include <genfit/Track.h>
#include <genfit/GFRaveVertex.h>
//rave
#include <rave/Track.h>
#include <rave/KinematicTree.h>

namespace Belle2 {

  namespace analysis {
    /**
     * The RaveKinematicVertexFitter class is part of the RaveInterface together with RaveSetup.
     * To fit a vertex the user adds tracks (different formats are supported). Then calls the fit() function where the name of the fitting method and
     * parameters can be passed as a string. RaveKinematicVertexFitter provides the getters to extract the results produced by Rave.
     */
    class RaveKinematicVertexFitter {
    public:
      /** The default constructor checks if RaveSetup was initialized and will set the attributes of RaveKinematicVertexFitter */
      RaveKinematicVertexFitter();

      //RaveKinematicVertexFitter(std::string howToInterfaceRave);
      /** Destructor */
      ~RaveKinematicVertexFitter();
      /** add a track (in the format of a Belle2::Particle) to set of tracks that should be fitted to a vertex */
      void addTrack(const Particle* aParticlePtr);
      /** All daughters of the argument of this function will be used as input for the vertex fit. Writes back the result directly to the mother particle */
      void addMother(const Particle* aMotherParticlePtr);
      /** Set Mother particle for Vertex/momentum update. Not to be used with addMother */
      void setMother(const Particle* aMotherParticlePtr);

      /** do the vertex fit with all tracks previously added with the addTrack or addMother function. The argument is a string determining the Rave fitting method. See https://rave.hepforge.org/trac/wiki/RaveMethods for the different methods
       * The return value is the number of successfully found vertices (depending on the selected algorithm this can be more then one vertex). Return value 0 means the fit was not successful. -1 means not enough tracks were added*/
      int fit(std::string options = "default");

      /** get the position of the fitted vertex. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      TVector3 getPos(std::vector<int>::size_type vertexId = 0) ;
      /** get the p value of the fitted vertex. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      double getPValue(std::vector<int>::size_type vertexId = 0);

      /** get the number of degrees of freedom (NDF) of the fitted vertex. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      double getNdf(std::vector<int>::size_type vertexId = 0);

      /** get the χ² of the fitted vertex. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      double getChi2(std::vector<int>::size_type vertexId = 0) ;

      /** get the covariance matrix (3x3) of the of the fitted vertex position. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      TMatrixDSym getCov(std::vector<int>::size_type vertexId = 0);
      /** Overwrite the global option in ReveSetup that tells the fitter if beam spot info should be used or not. The beam spot pos and cov must still be set in the RaveSetup class if you what to use it */
      void useBeamSpot(bool beamSpot = true) {
        if (beamSpot == true) {
          if (RaveSetup::s_instance->m_useBeamSpot == false) {
            B2ERROR("Beam spot information cannot be used because the beam spot position and covariance was not set in RaveSetup");
            throw;
          }
          m_useBeamSpot = true;
        } else {
          m_useBeamSpot = false;
        }
      }
      /** returns a pointer to the updated mother particle */
      Particle* getMother();

      /* TLorentzVector get4Vector(std::vector<int>::size_type vertexId = 0){ */
      /*   if (m_GFRaveVertices.empty()) { */
      /*     B2ERROR("There is no fitted Vertex. Maybe you did not call fit() or maybe the fit was not successful"); */
      /*     throw; */
      /*   } else if (vertexId < m_GFRaveVertices.size()) { */
      /*     //does not work this way hat to determin which track belong to which vertex */
      /*     TLorentzVector result; */

      /*     for ( int i = 0; i not_eq m_GFRaveVertices[vertexId]->getNTracks(); ++i){ */
      /*       TParticlePDG* part = TDatabasePDG::Instance()->GetParticle(m_gfTrackReps[i]->getPDG()); */
      /*       result += TLorentzVector(m_gfTrackReps[i]->getPos(), part->Mass()); */
      /*     } */
      /*     return result; */
      /*   } else { */
      /*     B2ERROR("The Vertex id " << vertexId << " does not correspond to any fitted vertex"); */
      /*     throw; */
      /*   } */
      /* } */

      //TMatrixDSym get4CovarianceMatrix(std::vector<int>::size_type vertexId = 0){
      //}

      //void setInvarantMass(int pdgCode);

      /** Delete all information of previously added tracks and fitted results*/
      void clearTracks() {
        m_inputParticles.clear();
        m_motherParticlePtr = NULL;
      }


      /** Set mass constrained fit   */
      void setMassConstFit(bool isConstFit = true);
      /** Set vertex fit: set false in case of mass fit only */
      void setVertFit(bool isVertFit = true);


    protected:

      /** flag determines if the beam spot will be used or not. Overwrites the global flag in RaveSetup */
      bool m_useBeamSpot;

      /** pointer to the mother particle who's  daughters will be used in the fit. the fit result will be written back to the mother particle */
      Particle* m_motherParticlePtr;

      /** Algorithm used by rave (kalman, avr, ...) */
      std::string m_raveAlgorithm;

      /** the output of the kinematic fit */
      rave::KinematicTree m_fittedResult;

      /** holds the tracks that were added to a RaveVertexFitter object in the format used by Rave */
      std::vector<rave::Track> m_raveTracks;

      /** holds the fitted vertices after fit() was called in the format used by Rave */
      std::vector < rave::Vertex > m_raveVertices;

      /** input particles for vertex fit in rave format */
      std::vector< rave::KinematicParticle > m_inputParticles;
      /** Particle fit output */
      rave::KinematicParticle m_fittedParticle;

      /** flag determines if the mass fit is performed */
      bool m_massConstFit;
      /** flag determines if the vertex fit is performed */
      bool m_vertFit;
      /** Ndf of the vertex fit */
      double m_fittedNdf;
      /** Pvalue of the fit result */
      double m_fittedPValue;
      /** chi^2 of the vertex fit */
      double m_fittedChi2;
      /** Fitted vertex position */
      TVector3 m_fittedPos;
      /** 4 momentum of the mother particle after the fit */
      TLorentzVector m_fitted4Vector;
      /** 7x7 errror matrix of the mother particle after the fit */
      TMatrixFSym m_fitted7Cov;

    };
  }

}

#endif
