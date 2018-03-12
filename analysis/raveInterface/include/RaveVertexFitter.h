/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler,  Luigi Li Gioi                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef RaveVertexFitter_H
#define RaveVertexFitter_H

#include <analysis/dataobjects/Particle.h>
#include <analysis/raveInterface/RaveSetup.h>

//#include <tracking/dataobjects/TrackFitResult.h>
//std
#include <string>

//Genfit
#include <genfit/Track.h>
#include <genfit/GFRaveConverters.h> // has some converter functions between root and rave
//rave
#include <rave/Track.h>


namespace Belle2 {
  class TrackFitResult;

  namespace analysis {
    /**
     * The RaveVertexFitter class is part of the RaveInterface together with RaveSetup.
     * To fit a vertex the user adds tracks (different formats are supported). Then calls the fit() function where the name of the fitting method and
     * parameters can be passed as a string. RaveVertexFitter provides the getters to extract the results produced by Rave.
     */
    class RaveVertexFitter {
      /** I am using std::vector<int>::size_type because it is the official return value of .size() although on normal machines a ::size_type is just an unsigned int anyway. Because std::vector<int>::size_type is so long it is shortened to VecSize */
      typedef std::vector<int>::size_type VecSize;
    public:

      /** The constructor */
      RaveVertexFitter(/*std::string howToInterfaceRave*/);

      /** Destructor */
      ~RaveVertexFitter();

      /** add a track (in the format of a genfit::Track) to set of tracks that should be fitted to a vertex */
      void addTrack(const genfit::Track& aGFTrack);

      /** add a track (in the format of a pointer to a genfit::Track) to set of tracks that should be fitted to a vertex */
      void addTrack(const genfit::Track* aGFTrackPtr);

      /** add a track (in the format of a Belle2::Particle) to set of tracks that should be fitted to a vertex */
      void addTrack(const Particle* const aParticlePtr);

      /** add a track (in the format of a pointer to a Belle2::TrackFitResult) to set of tracks that should be fitted to a vertex */
      void addTrack(const TrackFitResult* const aTrackPtr);

      /** All daughters of the argument of this function will be used as input for the vertex fit. */
      void addMother(const Particle* const aMotherParticlePtr);

      /** do the vertex fit with all tracks previously added with the addTrack or addMother function. The argument is a string determining the Rave fitting method. See https://rave.hepforge.org/trac/wiki/RaveMethods for the different methods
       * The return value is the number of successfully found vertices (depending on the selected algorithm this can be more then one vertex). Return value 0 means the fit was not successful. -1 means not enough tracks were added*/
      int fit(std::string options = "default");

      /** get the position of the fitted vertex. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      TVector3 getPos(VecSize vertexId = 0) const;

      /** get the p value of the fitted vertex. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      double getPValue(VecSize vertexId = 0) const;

      /** get the number of degrees of freedom (NDF) of the fitted vertex. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      double getNdf(VecSize vertexId = 0) const;

      /** get the χ² of the fitted vertex. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      double getChi2(VecSize vertexId = 0) const;

      /** get the covariance matrix (3x3) of the of the fitted vertex position. If Rave was also used to find different vertices the user has to provide the index of that vertex */
      TMatrixDSym getCov(VecSize vertexId = 0) const;

      /** get the weight Rave assigned to a specific input track. The trackId is determined by the order of the addTrack calls. So the first track added with addTrack has trackId = 0, the next has 1 etc */
      double getWeight(int trackId, VecSize vertexId = 0) const;

      /** get the ids of the tracks Rave associated with a specific vertex. The trackId is determined by the order of the addTrack calls. So the first track added with addTrack has trackId = 0, the next has 1 etc */
      std::vector<int> getTrackIdsForOneVertex(VecSize vertexId = 0) const;

      /** Overwrite the global option in ReveSetup that tells the fitter if beam spot info should be used or not. The beam spot pos and cov must still be set in the RaveSetup class if you what to use it */
      void useBeamSpot(bool beamSpot = true)
      {
        if (beamSpot == true) {
          if (RaveSetup::getRawInstance()->m_useBeamSpot == false) {
            B2ERROR("Beam spot information cannot be used because the beam spot position and covariance was not set in RaveSetup");
            throw;
          }
          m_useBeamSpot = true;
        } else {
          m_useBeamSpot = false;
        }
      }

      /** Delete all information of previously added tracks and fitted results*/
      void clearTracks()
      {
        m_raveTracks.clear();
      }


      /** Print all attributes of this object to terminal*/
      void Print() const
      {
        B2INFO("useBeamSpot: " << m_useBeamSpot);
        B2INFO("number of rave::Tracks: " << m_raveTracks.size());
        B2INFO("number of fitted  rave::Vertices: " << m_raveVertices.size());

      }
      /** return the number of tracks that where added via addTrack or addMother*/
      int getNTracks() const
      {
        return m_raveTracks.size();
      }

      /** update the Daughters particles */
      void updateDaughters();


    protected:

//  /** The default constructor checks if RaveSetup was initialized and will set the attributes of RaveVertexFitter */
//  RaveVertexFitter();
      /** Initialize m_useBeamSpot. Called by constructor*/
      void initBeamSpotMember();
      /** checks if the vertex the user requested via one of the getters it actually there*/
      void isVertexIdValid(const VecSize vertexId) const;
      /** converts a track from genfit::MeasuredStateOnPlane format to rave::Track format*/
      rave::Track GFMeasuredStateToRaveTrack(const genfit::MeasuredStateOnPlane& aGFState) const;

      /** converts a track from Belle2::TrackFitResult format to rave::Track format*/
      rave::Track TrackFitResultToRaveTrack(const TrackFitResult* const aTrackPtr) const;
      bool m_useBeamSpot; /**< flag determines if the beam spot will be used or not. Overwrites the global flag in RaveSetup */


      //std::string m_raveAlgorithm;

      /** holds the tracks that were added to a RaveVertexFitter object in the format used by Rave*/
      std::vector<rave::Track> m_raveTracks;

      /** holds the fitted vertices after fit() was called in the format used by Rave*/
      std::vector < rave::Vertex > m_raveVertices;

      /** Belle Particle pointers input */
      std::vector<Particle*> m_belleDaughters;


    };
  }

}

#endif
