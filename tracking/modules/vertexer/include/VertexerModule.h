/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Moritz Nadler                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VERTEXERMODULE_H
#define VERTEXERMODULE_H

#include <framework/core/Module.h>
#include <string>

#include <GFRaveVertexFactory.h>

#include <vector>



namespace Belle2 {

  /** Very simple module to use GFRave from basf2. It will take GFTrack objects from the dataStore and feed them to GFRave
   * which itself will use rave to do the vertex finding and fitting. The output are GFRaveVertex objects that will written to the dataStore
  */

  class VertexerModule : public Module {

  public:
    VertexerModule();
    ~VertexerModule() {}

    void initialize();
    void beginRun();
    void event();
    void endRun();
    void terminate();

  protected:

    int m_verbosity;      /**< verbosity parameter for the GFRaveFactory */
    std::string m_method;   /**< Name of method used by rave to find/fit the vertices */
    bool m_useBeamSpot;  /**< flag to switch on/off the usage of beam spot info for vertex fitting */
    GFRaveVertexFactory* m_gfRaveVertexFactoryPtr;   /**< pointer to the GFRaveVertexFactory that is the primary interface from Genfit to rave*/
    bool m_useGenfitPropagation; /**< flag to switch between Genfit and Rave propagation for vertex fitting */

    std::vector<double> m_beamSpotPos; /**< the user chosen beam spot position. User must provide a container with 3 (x,y,z) values */
    std::vector<double> m_beamSpotCov; /**< the user chosen beam spot position covariance matrix. User must provide 9 values (will be interpreted as a 3x3 matrix)*/
    int m_ndfTooSmallCounter; /**< counter for the events that have too little information (number of degrees of freedom (ndf)) to reconstruct at least one vertex */
    int m_fittedVertices; /**< counts the number of vertices rave was able to fit */
    std::string m_gfTracksColName; /**< Name of collection of GFTracks used for input */
  };
}

#endif
