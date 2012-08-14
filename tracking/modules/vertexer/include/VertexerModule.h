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

#include <TVector3.h>
#include <TMatrixD.h>


namespace Belle2 {

  /** Very simple module to use GFRave from basf2. It will take GFTrack Ojects from the dataStore and feed them GFRave
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
    bool m_useBeamSpot;  /**< flag to swtich on/off the usage of beam spot info for vertex fitting */
    GFRaveVertexFactory* m_gfRaveVertexFactoryPtr;   /**< pointer to the GFRaveVertexFactory that is the primary interface from Genfit to rave*/
    bool m_useGenfitPropagation; /**< flag to switch between Genfit and Rave propagation for vertex fitting */

    TVector3 m_beamSpotPos;
    TMatrixD m_beamSpotCov;

  };
}

#endif
