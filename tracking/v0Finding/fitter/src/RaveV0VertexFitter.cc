/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/v0Finding/fitter/RaveV0VertexFitter.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/IOIntercept.h>
#include <tracking/v0Finding/dataobjects/VertexVector.h>

#include <genfit/Track.h>
#include <genfit/GFRaveVertexFactory.h>
#include <genfit/GFRaveVertex.h>

#include <vector>

using namespace Belle2;

bool RaveV0VertexFitter::fit(genfit::Track& trackPlus, genfit::Track& trackMinus, const int, const int,
                             genfit::GFRaveVertex& vertex)
{
  VertexVector vertexVector;
  std::vector<genfit::Track*> trackPair {&trackPlus, &trackMinus};

  try {
    IOIntercept::OutputToLogMessages
    logCapture("V0Fitter GFRaveVertexFactory", LogConfig::c_Debug, LogConfig::c_Debug);
    logCapture.start();

    genfit::GFRaveVertexFactory vertexFactory;
    vertexFactory.findVertices(&vertexVector.v, trackPair);
  } catch (...) {
    B2ERROR("Exception during vertex fit.");
    return false;
  }

  if (vertexVector.size() != 1) {
    B2DEBUG(21, "Vertex fit failed. Size of vertexVector not 1, but: " << vertexVector.size());
    return false;
  }

  if ((*vertexVector[0]).getNTracks() != 2) {
    B2DEBUG(20, "Wrong number of tracks in vertex.");
    return false;
  }

  vertex = *vertexVector[0];
  return true;
}
