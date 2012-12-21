/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef RaveVertexFitter_H
#define RaveVertexFitter_H
//#include <analysis/particle/Particle.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/raveInterface/RaveSetup.h>
//std
#include <string>

//root
#include <TMath.h>

//Genfit
#include <GFTrack.h>
#include <GFRaveVertex.h>
//tracking
//#include <tracking/dataobjects/Track.h>

namespace Belle2 {

  namespace analysis {
    /**
     * The RaveVertexFitter class is part of the RaveInterface together with RaveSetup.
     * To fit a vertex the user adds tracks (different formats are supported). Then calls the fit() function where the name of the fitting method and
     * parameters can be passed as a string. RaveVertexFitter provides the getters to extract the results produced by Rave.
     */
    class RaveVertexFitter {
    public:

      RaveVertexFitter();
      ~RaveVertexFitter();

      /** add a track (in the format of a GFTrack) to set of tracks that should be fitted to a vertex */
      void addTrack(GFTrack& aGFTrack);
      /** add a track (in the format of a pointer to a GFTrack) to set of tracks that should be fitted to a vertex */
      void addTrack(GFTrack* const aGFTrackPtr);
      /** add a track (in the format of a pointer to a GFAbsTrackRep) to set of tracks that should be fitted to a vertex */
      void addTrack(GFAbsTrackRep* const aTrackRepPtr);

      //      void addTrack( const Belle1::Particle& aParticle);
      /** add a track (in the format of a Belle2::Particle) to set of tracks that should be fitted to a vertex */
      void addTrack(const Particle& aParticle);

      //void addTrack( const Track& aTrack);

      /** do the vertex fit with all tracks previously added with the addTrack function. The argument is a string determining the Rave fitting method. See https://rave.hepforge.org/trac/wiki/RaveMethods for the different methods
       * The return value is the number of succefully found vertices (depending on the selected algorithm this can be more then one vertex). Return value 0 means the fit was not successful. -1 means not enough tracks were added*/
      int fit(std::string options = "default");

      /** get the position of the fitted vertex. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      TVector3 getPos(std::vector<int>::size_type vertexId = 0) {
        if (m_GFRaveVertices.size() == 0) {
          return m_GFRaveVertices[vertexId]->getPos();
        } else if (vertexId < m_GFRaveVertices.size()) {
          B2ERROR("There is no fitted Vertex. Maybe you did not call fit() or maybe the fit was not successful");
          throw;
        } else {
          B2ERROR("The Vertex id " << vertexId << " does not correspond to any fitted vertex");
          throw;
        }
      }

      /** get the p value of the fitted vertex. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      double getPValue(std::vector<int>::size_type vertexId = 0) {
        if (m_GFRaveVertices.size() == 0) {
          return TMath::Prob(m_GFRaveVertices[vertexId]->getChi2(), int(m_GFRaveVertices[vertexId]->getNdf() + 0.5));
        } else if (vertexId < m_GFRaveVertices.size()) {
          B2ERROR("There is no fitted Vertex. Maybe you did not call fit() or maybe the fit was not successful");
          throw;
        } else {
          B2ERROR("The Vertex id " << vertexId << " does not correspond to any fitted vertex");
          throw;
        }
      }

      /** get the number of degrees of freedom (NDF) of the fitted vertex. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      double getNdf(std::vector<int>::size_type vertexId = 0) {
        if (m_GFRaveVertices.size() == 0) {
          return m_GFRaveVertices[vertexId]->getNdf();
        } else if (vertexId < m_GFRaveVertices.size()) {
          B2ERROR("There is no fitted Vertex. Maybe you did not call fit() or maybe the fit was not successful");
          throw;
        } else {
          B2ERROR("The Vertex id " << vertexId << " does not correspond to any fitted vertex");
          throw;
        }
      }

      /** get the χ² of the fitted vertex. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      double getChi2(std::vector<int>::size_type vertexId = 0) {
        if (m_GFRaveVertices.size() == 0) {
          return m_GFRaveVertices[vertexId]->getChi2();
        } else if (vertexId < m_GFRaveVertices.size()) {
          B2ERROR("There is no fitted Vertex. Maybe you did not call fit() or maybe the fit was not successful");
          throw;
        } else {
          B2ERROR("The Vertex id " << vertexId << " does not correspond to any fitted vertex");
          throw;
        }
      }

      /** get the covarance matrix (3x3) of the of the fitted vertex position. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      TMatrixDSym getCov(std::vector<int>::size_type vertexId = 0) {
        if (m_GFRaveVertices.size() == 0) {
          return m_GFRaveVertices[vertexId]->getCov();
        } else if (vertexId < m_GFRaveVertices.size()) {
          B2ERROR("There is no fitted Vertex. Maybe you did not call fit() or maybe the fit was not successful");
          throw;
        } else {
          B2ERROR("The Vertex id " << vertexId << " does not correspond to any fitted vertex");
          throw;
        }
      }

      void useBeamSpot(bool beamSpot = true) {
        if (beamSpot == true) {
          if (RaveSetup::s_instance->m_useBeamSpot == false) {
            B2ERROR("Beam spot information cannot be used because the beam spot position and covarance was not set");
            throw;
          }
          m_useBeamSpot = true;
        } else {
          m_useBeamSpot = false;
        }
      }

      //void setInvarantMass(int pdgCode);
      /** Delete all information of previously added tracks. */
      void clearTracks() {
        m_gfTrackReps.clear();
        int n = m_ownGfTrackReps.size();
        for (int i = 0; i not_eq n; ++i) {
          delete m_ownGfTrackReps[i];
        }
        m_ownGfTrackReps.clear();
        n = m_GFRaveVertices.size();
        for (int i = 0; i not_eq n; ++i) {
          delete m_GFRaveVertices[i];
        }
        m_GFRaveVertices.clear();
        //raveTrack.clear();//and clear the other one
      }
      /** Return the GFRaveVertex object. Holds all info on the fitted vertex. This is temorary and will be replaced with the Bell2 vertex object when ready */
      GFRaveVertex* getGFRaveVertex(std::vector<int>::size_type vertexId = 0) {
        if (m_GFRaveVertices.size() == 0) {
          return m_GFRaveVertices[vertexId];
        } else if (vertexId < m_GFRaveVertices.size()) {
          B2ERROR("There is no fitted Vertex. Maybe you did not call fit() or maybe the fit was not successful");
          throw;
        } else {
          B2ERROR("The Vertex id " << vertexId << " does not correspond to any fitted vertex");
          throw;
        }
      }


    protected:

      double m_chi2;
      int m_ndf;
      int m_fitStatus;

      bool m_useBeamSpot;

      std::string m_raveAlgorithm;

      std::vector<GFAbsTrackRep*> m_gfTrackReps;
      std::vector<GFAbsTrackRep*> m_ownGfTrackReps;

      std::vector < GFRaveVertex* > m_GFRaveVertices;

      //std::vector<ravetracks> raveTracks;


      /* options per fit:
       * rave algo and paramters
       * use beam spot info

       */
    };
  }

}

#endif
