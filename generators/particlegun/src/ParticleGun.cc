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

namespace {
  /** Function to generate a random variable according to a polyline.
   * params is a list of x and y coordinates, where the x coordinates are
   * sorted in ascending order and the y coordinates are all >=0. This function
   * generates an x variable so that x is distributed according to the line
   * which connects all points using an acceptance-rejection method.
   *
   * We use a step function as envelope for the polyline in the
   * acceptance-rejection method to achieve a worst case efficiency of 50%.
   *
   * @param n number of points in the polyline
   * @param x pointer to the x coordinates
   * @param y pointer to the y coordinates
   * @return value within x0 and xn, distributed according to the line
   *         connecting (xi,yi)
   */
  double randomPolyline(size_t n, const double* x, const double* y)
  {
    //Calculate the area of the step function which envelops the polyline and
    //the area in each segment.
    std::vector<double> weights(n - 1);
    double sumw(0);
    for (size_t i = 0; i < n - 1; ++i) {
      weights[i] = (x[i + 1] - x[i]) * max(y[i], y[i + 1]);
      sumw += weights[i];
    }
    while (true) {
      //Generate x distributed according to the step function. This can be done
      //with 100% efficiency
      double weight = gRandom->Uniform(0, sumw);
      size_t segment(0);
      for (; segment < n - 1; ++segment) {
        weight -= weights[segment];
        if (weight <= 0) break;
      }
      const double x1 = x[segment];
      const double x2 = x[segment + 1];
      const double x = gRandom->Uniform(x1, x2);
      //Now calculate y(x) using line between (x1,y1) and (x2,y2)
      const double y1 = y[segment];
      const double y2 = y[segment + 1];
      const double y = (y2 == y1) ? y1 : y1 + (x - x1) / (x2 - x1) * (y2 - y1);
      //Generate a random y inside the step function
      const double randY = gRandom->Uniform(0, max(y1, y2));
      //And accept the x value if randY lies below the line
      if (randY < y) return x;
      //Otherwise repeat generation of x and y
    }
  }
}


double ParticleGun::generateValue(Distribution dist, const vector<double>& params)
{
  double rand(0);
  switch (dist) {
    case fixedValue:
      return params[0];
    case uniformDistribution:
    case uniformPtDistribution:
      return gRandom->Uniform(params[0], params[1]);
    case inversePtDistribution:
      return 1. / gRandom->Uniform(1. / params[1], 1. / params[0]);
    case uniformCosDistribution:
      return acos(gRandom->Uniform(cos(params[0]), cos(params[1])));
    case normalDistribution:
    case normalPtDistribution:
      return gRandom->Gaus(params[0], params[1]);
    case normalCosDistribution:
      return acos(gRandom->Gaus(params[0], params[1]));
    case discreteSpectrum:
      //Create weighted discrete distribution
      //First we sum all the weights (second half of the array)
      for (size_t i = params.size() / 2; i < params.size(); ++i) rand += params[i];
      //Then generate a random variable between 0 and sum of weights
      rand = gRandom->Uniform(0, rand);
      //Go over the weights again and subtract from generated value
      for (size_t i = params.size() / 2; i < params.size(); ++i) {
        rand -= params[i];
        //If the value is negative we found the correct bin, return that value
        if (rand <= 0) return params[i - (params.size() / 2)];
      }
      B2FATAL("Something wrong with picking fixed spectra values");
    case polylineDistribution:
    case polylinePtDistribution:
      return randomPolyline(params.size() / 2, params.data(), params.data() + params.size() / 2);
    case polylineCosDistribution:
      return acos(randomPolyline(params.size() / 2, params.data(), params.data() + params.size() / 2));
    default:
      B2FATAL("Unknown distribution");
  }
  return 0;
}

bool ParticleGun::generateEvent(MCParticleGraph& graph)
{
  //generate the event vertex (possible the same for all particles in event)
  double vx = generateValue(m_params.xVertexDist, m_params.xVertexParams);
  double vy = generateValue(m_params.yVertexDist, m_params.yVertexParams);
  double vz = generateValue(m_params.zVertexDist, m_params.zVertexParams);

  //Determine number of tracks
  int nTracks = static_cast<int>(m_params.nTracks);
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
    } else if (m_params.nTracks <= 0) {
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
    if (m_params.momentumDist == uniformPtDistribution || m_params.momentumDist == normalPtDistribution ||
        m_params.momentumDist == inversePtDistribution || m_params.momentumDist == polylinePtDistribution) {
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
    //Particle is stable in generator. We could use MCParticleGraph options to
    //do this automatically but setting it here makes the particle correct
    //independent of the options
    p.setDecayTime(numeric_limits<double>::infinity());

    if (m_params.independentVertices) {
      //If we have independent vertices, generate new vertex for next particle
      vx = generateValue(m_params.xVertexDist, m_params.xVertexParams);
      vy = generateValue(m_params.yVertexDist, m_params.yVertexParams);
      vz = generateValue(m_params.zVertexDist, m_params.zVertexParams);
    }
  }// end loop over particles in event

  return true;
}

bool ParticleGun::setParameters(const Parameters& p)
{
  //checking that all parameters are useful is the most complex part of the
  //whole particle gun. We need to disallow distributions for certain variables
  //and check the number of parameters and their validity

  //As long as this is true, no error has been found. We could just return but
  //it's so much nicer to display all possible errors at once so the user does
  //not need to iterate over this all the time
  bool ok(true);

  //Make an enum -> name mapping for nice error messages
  std::map<Distribution, std::string> distributionNames = {
    {fixedValue,              "fixedValue"},
    {uniformDistribution,     "uniform"},
    {uniformPtDistribution,   "uniformPt"},
    {uniformCosDistribution,  "uniformCos"},
    {normalDistribution,      "normal"},
    {normalPtDistribution,    "normalPt"},
    {normalCosDistribution,   "normalCos"},
    {inversePtDistribution,   "inversePt"},
    {polylineDistribution,    "polyline"},
    {polylinePtDistribution,  "polylinePt"},
    {polylineCosDistribution, "polylineCos"},
    {discreteSpectrum,        "discrete"},
  };
  //Small helper lambda to get the distribution by name
  auto getDist = [&p](const std::string & dist) -> Distribution {
    if (dist == "momentum") return p.momentumDist;
    if (dist == "xVertex")  return p.xVertexDist;
    if (dist == "yVertex")  return p.yVertexDist;
    if (dist == "zVertex")  return p.zVertexDist;
    if (dist == "theta")    return p.thetaDist;
    if (dist == "phi")      return p.phiDist;
    throw std::runtime_error("wrong parameter");
  };
  //Small helper lambda to get the parameters by name
  auto getPars = [&p](const std::string & dist) -> const std::vector<double>& {
    if (dist == "momentum") return p.momentumParams;
    if (dist == "xVertex")  return p.xVertexParams;
    if (dist == "yVertex")  return p.yVertexParams;
    if (dist == "zVertex")  return p.zVertexParams;
    if (dist == "theta")    return p.thetaParams;
    if (dist == "phi")      return p.phiParams;
    throw std::runtime_error("wrong parameter");
  };
  //Small helper lambda to produce a nice error message and set the error flag
  //if the distribution is excluded.
  auto excludeDistribution = [&](const std::string & dist, Distribution excluded) {
    if (getDist(dist) == excluded) {
      B2ERROR(distributionNames[excluded] << " is not allowed for " << dist << " generation");
      ok = false;
    }
  };

  //Exclude some distributions
  for (auto dist : {"momentum", "xVertex", "yVertex", "zVertex"}) {
    excludeDistribution(dist, uniformCosDistribution);
    excludeDistribution(dist, normalCosDistribution);
    excludeDistribution(dist, polylineCosDistribution);
  }
  for (auto dist : {"xVertex", "yVertex", "zVertex", "phi", "theta"}) {
    excludeDistribution(dist, uniformPtDistribution);
    excludeDistribution(dist, normalPtDistribution);
    excludeDistribution(dist, inversePtDistribution);
    excludeDistribution(dist, polylinePtDistribution);
  }

  //Check that we have some particle ids to generate
  if (p.pdgCodes.empty()) {
    B2ERROR("No pdgCodes specified for generation");
    ok = false;
  }

  //Check minimum numbers of parameters
  for (auto par : {"momentum", "xVertex", "yVertex", "zVertex", "theta", "phi"}) {
    const Distribution dist = getDist(par);
    size_t minParams = (dist == fixedValue) ? 1 : 2;
    if (dist == polylineDistribution || dist == polylinePtDistribution || dist == polylineCosDistribution) minParams = 4;
    const size_t hasParams = getPars(par).size();
    if (hasParams < minParams) {
      B2ERROR(par << " generation: " << distributionNames[dist]
              << " distribution requires at least " << minParams
              << " parameters, " << hasParams << " given");
      ok = false;
    }
    //Check even number of parameters for discrete or polyline distributions
    if (dist == discreteSpectrum || dist == polylineDistribution || dist == polylinePtDistribution || dist == polylineCosDistribution) {
      if ((hasParams % 2) != 0) {
        B2ERROR(par << " generation: " << distributionNames[dist] << " requires an even number of parameters");
        ok = false;
      } else if (dist == discreteSpectrum || hasParams >= 4) {
        //Check wellformedness of polyline pdf: ascending x coordinates and positive y coordinates with at least one nonzero value
        //Discrete spectrum only requires positive weights, no sorting needed
        const std::vector<double>& p = getPars(par);
        const std::string parname = (dist == discreteSpectrum) ? "weight" : "y coordinate";
        //Check for sorting for polylines
        if (dist != discreteSpectrum) {
          for (size_t i = 0; i < (hasParams / 2) - 1; ++i) {
            if (p[i] > p[i + 1]) {
              B2ERROR(par << " generation: " << distributionNames[dist] << " requires x coordinates in ascending order");
              ok = false;
            }
          }
        }
        //Check that values are non-negative and at least one value is positive
        auto minmax = std::minmax_element(p.begin() + hasParams / 2, p.end());
        if (*minmax.first < 0) {
          B2ERROR(par << " generation: " << distributionNames[dist] << " requires "
                  << parname << "s to be non-negative");
          ok = false;
        }
        if (*minmax.second <= 0) {
          B2ERROR(par << " generation: " << distributionNames[dist] << " requires at least one "
                  << parname << " to be positive");
          ok = false;
        }
      }
    }
  }

  //Finally check that we do not have any problems with the inverse
  if (p.momentumDist == inversePtDistribution && p.momentumParams.size() >= 2) {
    if (p.momentumParams[0] == 0 || p.momentumParams[1] == 0) {
      B2ERROR("inversePt distribution does not allow zero momentum");
      ok = false;
    }
  }

  //If everything is ok, set the new parameters, else return false
  if (ok) { m_params = p; }
  return ok;
}
