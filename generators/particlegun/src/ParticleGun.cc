/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <generators/particlegun/ParticleGun.h>

#include <cmath>
#include <limits>

using namespace std;
using namespace Belle2;

double ParticleGun::generateValue(Distribution dist, const vector<double> params)
{
  double rand(0);
  switch (dist) {
    case fixedValue:
      return params[0];
    case uniformDistribution:
    case uniformPtDistribution:
      return gRandom->Uniform(params[0], params[1]);
    case uniformCosinusDistribution:
      return acos(gRandom->Uniform(cos(params[0]), cos(params[1])));
    case normalDistribution:
    case normalPtDistribution:
      return gRandom->Gaus(params[0], params[1]);
    case discreteSpectrum:
      for (size_t i = 0; i < params.size() / 2; ++i) rand += params[2 * i];
      rand = gRandom->Uniform(0, rand);
      for (size_t i = 0; i < params.size() / 2; ++i) {
        rand -= params[2 * i];
        if (rand <= 0) return params[2 * i + 1];
      }
      B2FATAL("Something wrong with picking fixed spectra values");
    default:
      B2FATAL("Unknown distribution");
  }
  return 0;
}

bool ParticleGun::generateEvent(MCParticleGraph& graph)
{
  //generate the event vertex (possible the same for all particles in event)
  double vx = generateValue(m_params.vertexDist, m_params.xVertexParams);
  double vy = generateValue(m_params.vertexDist, m_params.yVertexParams);
  double vz = generateValue(m_params.vertexDist, m_params.zVertexParams);

  //Determine number of tracks
  int nTracks = m_params.nTracks;
  if (m_params.nTracks <= 0) {
    nTracks = m_params.pdgCodes.size();
  } else if (m_params.varyNumberOfTracks) {
    nTracks = gRandom->Poisson(m_params.nTracks);
  }

  //Make list of particles
  for (int i = 0; i < nTracks; ++i) {
    MCParticleGraph::GraphParticle& p = graph.addParticle();
    p.setStatus(MCParticle::c_PrimaryParticle);
    if (m_params.pdgCodes.size() == 1) {
      //only one PDGcode available, always take this one
      p.setPDG(m_params.pdgCodes[0]);
    } else if (nTracks <= 0) {
      //0 or negative nTracks, take the ids sequentially
      p.setPDG(m_params.pdgCodes[i]);
    } else {
      //else choose randomly one of the available codes
      int index = static_cast<int>(gRandom->Uniform(m_params.pdgCodes.size()));
      p.setPDG(m_params.pdgCodes[index]);
    }
    p.setMassFromPDG();
    p.setChargeFromPDG();
    p.setFirstDaughter(0);
    p.setLastDaughter(0);

    //lets generate the momentum vector:
    double momentum = generateValue(m_params.momentumDist, m_params.momentumParams);
    double phi      = generateValue(m_params.phiDist, m_params.phiParams);
    double theta    = generateValue(m_params.thetaDist, m_params.thetaParams);

    double pt = momentum * sin(theta);
    if (m_params.momentumDist == uniformPtDistribution || m_params.momentumDist == normalPtDistribution) {
      //this means we are actually generating the Pt and not the P, so exchange values
      pt = momentum;
      momentum = (sin(theta) > 0) ? (pt / sin(theta)) : numeric_limits<double>::max();
    }

    double pz = momentum * cos(theta);
    double px = pt * cos(phi);
    double py = pt * sin(phi);
    double m  = p.getMass();
    double e  = sqrt(momentum * momentum + m * m);

    p.setMomentum(px, py, pz);
    p.setEnergy(e);
    p.setProductionVertex(vx, vy, vz);
    p.addStatus(MCParticle::c_StableInGenerator);

    if (m_params.independentVertices) {
      //If we have independent vertices, generate new vertex for next particle
      vx = generateValue(m_params.vertexDist, m_params.xVertexParams);
      vy = generateValue(m_params.vertexDist, m_params.yVertexParams);
      vz = generateValue(m_params.vertexDist, m_params.zVertexParams);
    }
  }// end loop over particles in event

  return true;
}

//Helper macro to check if all distributions make sense
#define CHECK_DIST(var,dist) \
  if(p.var##Dist == dist){ \
    B2ERROR(#dist << " does not make sense for " << #var << " generation"); \
    ok = false; \
  }

//Helper macro to check that the number of parameters supplied for a given distribution is correct
#define CHECK_DIST_NPARAMS(var,dist,params,num) \
  if(p.var##Dist == dist && p.params##Params.size()<num) { \
    B2ERROR(#var << " generation: " << #dist << " requires at least " \
            << num << " parameters, " \
            << p.params##Params.size() << " given for " << #params); \
    ok = false; \
  }

//Helper macro to check that a distribution has the correct number of parameters
#define CHECK_NPARAMS(var,params) \
  CHECK_DIST_NPARAMS(var,fixedValue,params,1) \
  CHECK_DIST_NPARAMS(var,uniformDistribution,params,2) \
  CHECK_DIST_NPARAMS(var,uniformPtDistribution,params,2) \
  CHECK_DIST_NPARAMS(var,uniformCosinusDistribution,params,2) \
  CHECK_DIST_NPARAMS(var,normalDistribution,params,2) \
  CHECK_DIST_NPARAMS(var,normalPtDistribution,params,2)\
  CHECK_DIST_NPARAMS(var,discreteSpectrum,params,2)\
  if(p.var##Dist == discreteSpectrum && p.params##Params.size() % 2 != 0){\
    B2ERROR(#var << " generation: discreteSpectrum requires an even number of parameters");\
    ok = false; \
  }

bool ParticleGun::setParameters(const Parameters& p)
{
  //Sanity checks
  bool ok(true);

  //Check that we have some particle ids to generate
  if (p.pdgCodes.empty()) {
    B2ERROR("No pdgCodes specified for generation");
    ok = false;
  }
  //Check that the distributions make sense, uniformPt for example only makes sense for momentum generation
  CHECK_DIST(momentum, uniformCosinusDistribution);
  CHECK_DIST(vertex  , uniformCosinusDistribution);
  CHECK_DIST(vertex  , uniformPtDistribution);
  CHECK_DIST(phi     , uniformPtDistribution);
  CHECK_DIST(theta   , uniformPtDistribution);
  CHECK_DIST(vertex  , normalPtDistribution);
  CHECK_DIST(phi     , normalPtDistribution);
  CHECK_DIST(theta   , normalPtDistribution);
  if (p.phiDist == uniformCosinusDistribution) {
    B2WARNING("uniformCosinus distribution not intended for phi, please make sure this is set correctly");
  }
  //Check that the amount of parameters supplied is correct
  CHECK_NPARAMS(momentum, momentum)
  CHECK_NPARAMS(phi,      phi)
  CHECK_NPARAMS(theta,    theta)
  CHECK_NPARAMS(vertex,   xVertex)
  CHECK_NPARAMS(vertex,   yVertex)
  CHECK_NPARAMS(vertex,   zVertex)

  //If everything is ok, set the new parameters, else return false
  if (ok) {
    m_params = p;
    return true;
  }
  return false;
}
