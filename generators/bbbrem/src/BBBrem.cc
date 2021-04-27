/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Hiroyuki Nakayama                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/bbbrem/BBBrem.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <TRandom3.h>
//#include <cmath>

using namespace std;
using namespace Belle2;


void BBBrem::init(double cmsEnergy, double minPhotonEFrac, bool unweighted, double maxWeight, int densitymode,
                  double densityparameter)
{
  m_cmsEnergy = cmsEnergy;
  m_photonEFrac = minPhotonEFrac;

  m_unweighted = unweighted;
  m_maxWeight = maxWeight;
  m_densityCorrectionMode = densitymode;
  m_densityCorrectionParameter = densityparameter;

  m_maxWeightDelivered = 0.0;
  m_sumWeightDelivered = 0.0;
  m_sumWeightDeliveredSqr = 0.0;
  m_weightCount = 0;

  if ((minPhotonEFrac <= 0.0) || (minPhotonEFrac >= 1.0)) {
    B2ERROR("BBBrem: The minimum photon energy fraction has to be in the range ]0,1[ !");
    return;
  }

  B2DEBUG(100, "BBBrem: Center of mass energy:     " << cmsEnergy);
  B2DEBUG(100, "BBBrem: Minimum photon energy:     " << minPhotonEFrac << " * beam energy");

  //Initialize the constants (in order to be consistent with the FORTRAN source code)
  alpha = Const::fineStrConst;
  rme = Const::electronMass;

  //Initialize the derived constants
  s     = cmsEnergy * cmsEnergy;
  rme2  = rme * rme;
  rme2s = rme2 / s;
  rls   = -log(rme2s);
  z0    =  minPhotonEFrac / (1.0 - minPhotonEFrac);

  //Approximate total cross section
  a1 = log((1.0 + z0) / z0);
//   a2 = (log(1.0 + z0)) / z0;
// Expression 'log(1 + x)' is replaced by 'log1p(x)' to avoid loss of precision.
  a2 = (log1p(z0)) / z0;
  ac = a1 / (a1 + a2);
  sigapp = 8.0 * (alpha * alpha * alpha) / rme2 * (-log(rme2s)) * (a1 + a2) * tomb;
  B2DEBUG(100, "BBBrem: Approximate cross section: " << sigapp << " millibarn");

  //Initial-state momenta
  eb     = 0.5 * cmsEnergy;
  pb     = sqrt(eb * eb - rme2);
  rin2pb = 0.5 / pb;
  p1[0] = 0.0;
  p1[1] = 0.0;
  p1[2] = -pb;
  p1[3] = eb;
  q1[0] = 0.0;
  q1[1] = 0.0;
  q1[2] = pb;
  q1[3] = eb;
}


double BBBrem::generateEvent(MCParticleGraph& mcGraph, TVector3 vertex, TLorentzRotation boost)
{

  if (m_unweighted) {
    double ran = 0.0;
    do {
      calcOutgoingLeptonsAndWeight();
      if (weight > m_maxWeightDelivered) m_maxWeightDelivered = weight;
      if (weight > m_maxWeight) {
        B2INFO("BBBrem: OVERWEIGHT, w=" << weight << ", nw=" << m_weightCountOver << ", sumw=" << m_sumWeightDeliveredOver);
        m_weightCountOver++;
        m_sumWeightDeliveredOver = m_sumWeightDeliveredOver + (weight - m_maxWeight);
        m_sumWeightDeliveredSqrOver = m_sumWeightDeliveredSqrOver + (weight - m_maxWeight) * (weight - m_maxWeight);
      }
      m_weightCount++;
      m_sumWeightDelivered += weight;
      m_sumWeightDeliveredSqr += weight * weight;

      ran = gRandom->Uniform();
    } while (weight <= ran * m_maxWeight);
  } else {
    calcOutgoingLeptonsAndWeight();
    if (weight > m_maxWeightDelivered) m_maxWeightDelivered = weight;
    m_weightCount++;
    m_sumWeightDelivered += weight;
    m_sumWeightDeliveredSqr += weight * weight;
  }

  //Store the incoming particles as virtual particles, the outgoing particles as real particles
  storeParticle(mcGraph, p1, 11, vertex, boost, false, true);
  storeParticle(mcGraph, q1, -11, vertex, boost, false, true);

  // BBBrem emits gammma only from electron(p1)
  // To emit gamma from positron we need to swap electron and positron here
  bool swapflag = (gRandom->Uniform() > 0.5) ? true : false;
  if (swapflag) {
    double tmp[4];
    for (int i = 0; i < 4; i++) tmp[i] = p2[i];
    for (int i = 0; i < 4; i++) p2[i]  = q2[i];
    for (int i = 0; i < 4; i++) q2[i] = tmp[i];
    p2[2] = -p2[2];
    q2[2] = -q2[2];
    qk[2] = -qk[2];
  }

  //Store the outgoing particles as real particles
  storeParticle(mcGraph, p2, 11, vertex, boost, false, false);
  storeParticle(mcGraph, q2, -11, vertex, boost, false, false);
  storeParticle(mcGraph, qk, 22, vertex, boost, false, false);

  if (!m_unweighted) return weight;
  return 1.0;
}


void BBBrem::term()
{
  if (m_weightCount > 0.0) {
    m_crossSection          = m_sumWeightDelivered / m_weightCount;
    m_crossSectionError     = sqrt(m_sumWeightDeliveredSqr - m_sumWeightDelivered * m_sumWeightDelivered / m_weightCount) /
                              m_weightCount;

    if (m_unweighted) {
      m_crossSectionOver      = m_sumWeightDeliveredOver / m_weightCount;
      m_crossSectionErrorOver = sqrt(abs(((m_sumWeightDeliveredSqrOver) / m_weightCount - m_crossSectionOver * m_crossSectionOver) /
                                         m_weightCount));
    }
  }
}


//=========================================================================
//                       Protected methods
//=========================================================================

void BBBrem::calcOutgoingLeptonsAndWeight()
{
  //Generate z
  double z;
  if (gRandom->Uniform() < ac) {
    double temp1 = gRandom->Uniform();
//     z = 1.0 / (temp1 * (exp(a1 * gRandom->Uniform()) - 1.0));
// Expression 'exp(x) - 1' is replaced by 'expm1(x)' to avoid loss of precision.
    z = 1.0 / (temp1 * (expm1(a1 * gRandom->Uniform())));
  } else {
    z = z0 / gRandom->Uniform();
  }

  //Bounds on t
  double y = rme2s * z;
  double q0 = eb * y;
  double temp1 = pb * pb - eb * q0;
  double temp2 = temp1 * temp1 - rme2 - q0 * q0;

  //If temp2<0 (very very rare): set weight to 0
  if (temp2 < 0.0) {
    B2WARNING("BBBrem: y too large: delta_t^2 = " << temp2 << " !!!");
    weight = 0.0;
  } else {
    double tmin = -2.0 * (temp1 + sqrt(temp2));
    double tmax = rme2 * s * y * y / tmin;

    //Generate t
    double sy = s * y;
    double w2 = sy + rme2;
    temp1 = sy + tmax;
    double rlamx = sqrt(temp1 * temp1 - 4.0 * w2 * tmax);

    if (temp1 <= 0.0) {
      temp1 = rlamx - temp1;
    } else {
      temp1 = -4.0 * w2 * tmax / (rlamx + temp1);
    }

    double t = 0.0;
    do {
      double b;
// b = exp(gRandom->Uniform() * log(1.0 + 2.0 * sy / temp1));
// Expression 'log(1 + x)' is replaced by 'log1p(x)' to avoid loss of precision.
      b = exp(gRandom->Uniform() * log1p(2.0 * sy / temp1));
      t = -b * z * z * rme2 / ((b - 1) * (b * z + b - 1));
    } while (t <= tmin);

    //Generate cgam
    double rlam = sqrt((sy - t) * (sy - t) - 4 * rme2 * t);
    double eps = 4 * rme2 * w2 / (rlam * (rlam + w2 + rme2 - t));
    double rl = log((2 + eps) / eps);
//     double vgam = eps * (exp(gRandom->Uniform() * rl) - 1.0);
// Expression 'exp(x) - 1' is replaced by 'expm1(x)' to avoid loss of precision.
    double vgam = eps * (expm1(gRandom->Uniform() * rl));
    double cgam = 1.0 - vgam;
    double sgam = sqrt(vgam * (2 - vgam));

    //Generate azimuthal angles
    double phi  = twopi * gRandom->Uniform();
    double phig = twopi * gRandom->Uniform();

    //Construct momentum transfer q(mu)
    double ql = (2.0 * eb * q0 - t) * rin2pb;
    double qt = sqrt((tmax - t) * (t - tmin)) * rin2pb;
    double q[4];
    q[0] = qt * sin(phi);
    q[1] = qt * cos(phi);
    q[2] = ql;
    q[3] = q0;

    //Construct momentum of outgoing positron in lab frame
    q2[0] = q1[0] - q[0];
    q2[1] = q1[1] - q[1];
    q2[2] = q1[2] - q[2];
    q2[3] = q1[3] - q[3];

    //Find euler angles of p1(mu) in cm frame
    double r0 = eb + q0;
    double w = sqrt(w2);
    double rin2w = 0.5 / w;
    double rinr0w = 1.0 / (r0 + w);
    double eta = -(sy + 2 * w * q0 + t) * rin2w * rinr0w;
    double phat1 = -q[0] * (1 + eta);
    double phat2 = -q[1] * (1 + eta);
    double phat3 = pb * eta - ql * (1 + eta);
    double phatl = rlam * rin2w;
    double phatt = sqrt(phat1 * phat1 + phat2 * phat2);
    double sfhat = phat1 / phatt;
    double cfhat = phat2 / phatt;
    double sthat = phatt / phatl;
    double vthat;
    if (phat3 > 0.0) {
      vthat = sthat * sthat / (1 - sqrt(1 - sthat * sthat));
    } else {
      vthat = sthat * sthat / (1 + sqrt(1 - sthat * sthat));
    }
    double cthat = vthat - 1.0;

    //Rotate using these euler angles to get the qk direction in the cm
    double sfg = sin(phig);
    double cfg = cos(phig);
    temp1 = sgam * sfg;
    temp2 = cthat * sgam * cfg + sthat * cgam;
    double veg = vthat + vgam - vthat * vgam - sthat * sgam * cfg;
    double qkhat[4];
    qkhat[3] = sy * rin2w;
    qkhat[0] = qkhat[3] * (cfhat * temp1 + sfhat * temp2);
    qkhat[1] = qkhat[3] * (-sfhat * temp1 + cfhat * temp2);
    qkhat[2] = qkhat[3] * (veg - 1.0);

    //Boost the photon momentum to the lab frame
    temp1 = pb * qkhat[2];
    if (temp1 > 0.0) {
      temp2 = (rme2 * qkhat[3] * qkhat[3] + pb * pb * (qkhat[0] * qkhat[0] + qkhat[1] * qkhat[1])) / (eb * qkhat[3] + temp1);
    } else {
      temp2 = eb * qkhat[3] - temp1;
    }

    qk[3] = (temp2 + qkhat[3] * q[3] + qkhat[0] * q[0] + qkhat[1] * q[1] + qkhat[2] * q[2]) / w;
    temp1 = (qk[3] + qkhat[3]) * rinr0w;
    qk[0] = qkhat[0] + temp1 * q[0];
    qk[1] = qkhat[1] + temp1 * q[1];
    qk[2] = qkhat[2] + temp1 * (-pb + ql);

    //Construct p2 by momentum conservation
    p2[0] = -q2[0] - qk[0];
    p2[1] = -q2[1] - qk[1];
    p2[2] = -q2[2] - qk[2];
    p2[3] = -q2[3] - qk[3] + m_cmsEnergy;

    //Impose cut on the photon energy: qk[3]>eb*rk0
    if (qk[3] < eb * m_photonEFrac) {
      weight = 0.0;
    } else {
      //The event is now accepted: compute matrix element and weight
      //Compute fudge factor c1
//       double c1 = log(1 + z) / log((2 + eps) / eps);
//       Expression 'log(1 + x)' is replaced by 'log1p(x)' to avoid loss of precision.
      double c1 = log1p(z) / log((2.0 + eps) / eps);

      //Compute fudge factor c2
      temp1 = sy - tmax;
      double vnumx = sqrt(temp1 * temp1 - 4.0 * rme2 * tmax) + temp1;
      temp1 = sy + tmax;

      double vdenx;
      if (temp1 < 0.0) {
        vdenx = sqrt(temp1 * temp1 - 4.0 * w2 * tmax) - temp1;
      } else {
        vdenx = -4.0 * w2 * tmax / (sqrt(temp1 * temp1 - 4.0 * w2 * tmax) + temp1);
      }
      temp1 = sy - tmin;
      double vnumn = sqrt(temp1 * temp1 - 4.0 * rme2 * tmin) + temp1;
      temp1 = sy + tmin;

      double vdenn;
      if (temp1 < 0.0) {
        vdenn = sqrt(temp1 * temp1 - 4.0 * w2 * tmin) - temp1;
      } else {
        vdenn = -4.0 * w2 * tmin / (sqrt(temp1 * temp1 - 4.0 * w2 * tmin) + temp1);
      }
      double c2 = 2.0 * rls / log((vnumx * vdenn) / (vdenx * vnumn));

      //Compute vector (small) r in cm frame, and (big) z
      double rlabl = (t - 2.0 * rme2 * y) * rin2pb;
      double rhat4 = -(2.0 * rme2 * y + (1 - y) * t) * rin2w;
      double etar = rhat4 * rinr0w;
      double rhat1 = -q[0] * (1 + etar);
      double rhat2 = -q[1] * (1 + etar);
      double rhat3 = rlabl + (pb - ql) * etar;
      double zz = s * (rhat4 * qkhat[3] - rhat1 * qkhat[0] - rhat2 * qkhat[1] - rhat3 * qkhat[2]);

      //The other invariants
      double s1 = 4.0 * eb * (eb - qk[3]);
      double d1 = sy * rlam * (eps + vgam) * rin2w * rin2w;
      double d2 = 0.5 * sy;

      //The exact matrix element
      //Kleiss-burkhardt cross section multiplied by t^2
      double rind1 = 1.0 / d1;
      double rind12 = rind1 * rind1;
      double rind2 = 1.0 / d2;
      double rind22 = rind2 * rind2;
      temp1 = s + t - 2 * d2;
      temp2 = s1 + t + 2 * d1;
      double aa0 = (s * s + s1 * s1 + temp1 * temp1 + temp2 * temp2) * rind1 * rind2 * (-t);
      double aa1 = -4.0 * rme2 * zz * zz * rind12 * rind22;
      double aa2 = -8.0 * rme2 * (d1 * d1 + d2 * d2) * rind1 * rind2;
      double aa3 = 16.0 * rme2 * rme2 * (d1 - d2) * zz * rind12 * rind22;
      double aa4 = -16.0 * rme2 * rme2 * rme2 * (d1 - d2) * (d1 - d2) * rind12 * rind22;
      double rmex = aa0 + aa1 + aa2 + aa3 + aa4;

      //The approximate matrix element without c1,2, multiplied by t^2
      double rmap = 4.0 * s * s * rind1 * rind2 * (-t) * c1 * c2;

      //The weight
      weight = rmex / rmap * sigapp;

      //Isnan check (not sure if this is ok)
      if (std::isnan(weight)) {
        B2WARNING("BBBrem: Weight is nan! Setting the weight to zero.");
        weight = 0.0;
      }


      //========================================================
      // beam size effect (or density effect)
      // ref: arxiv:hep-ph/9401333

      // tc = (hbarc/simga_y)^2
//       double tc = 1.68e-17;  //SuperKEKB LER (sigma_y*=48nm)
      //double tc = 9.81e-18;  //SuperKEKB HER (sigma_y*=63nm)
      double tc = m_densityCorrectionParameter;
//       int cutflag = 0 ;  // 0: no cut, 1: hard cut, 2: soft cut
      if (m_densityCorrectionMode == 1) {
        if (abs(t) < tc) weight = 0.0;
      } else if (m_densityCorrectionMode == 2) {
        if (t != tc) weight *= t * t / (t - tc) / (t - tc); // t<0<tc, always t!=tc
      }
      //========================================================

    }
  }
}


void BBBrem::storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, TVector3 vertex, TLorentzRotation boost,
                           bool isVirtual, bool isInitial)
{

  // RG 6/25/14 Add new flag for ISR "c_Initial"
  MCParticleGraph::GraphParticle& part = mcGraph.addParticle();
  if (isVirtual) {
    part.setStatus(MCParticle::c_IsVirtual);
  } else if (isInitial) {
    part.setStatus(MCParticle::c_Initial);
  }

  //all particles from a generator are primary
  part.addStatus(MCParticle::c_PrimaryParticle);

  //all non virtual photons from BBBREM are ISR or FSR
  if (pdg == 22 && !isVirtual) {
    part.addStatus(MCParticle::c_IsISRPhoton);
    part.addStatus(MCParticle::c_IsFSRPhoton);
  }

  part.setPDG(pdg);
  part.setFirstDaughter(0);
  part.setLastDaughter(0);
  part.setMomentum(TVector3(mom[0], mom[1], -mom[2])); //Switch direction, because electrons fly into the +z direction at Belle II
  part.setEnergy(mom[3]);
  part.setMassFromPDG();
//   part.setDecayVertex(0.0, 0.0, 0.0);

  //set the production vertex of non initial particles
  if (!isInitial) {
    TVector3 v3 = part.getProductionVertex();
    v3 = v3 + vertex;
    part.setProductionVertex(v3);
    part.setValidVertex(true);
  }

  //If boosting is enable boost the particles to the lab frame
  TLorentzVector p4 = part.get4Vector();
  p4 = boost * p4;
  part.set4Vector(p4);
}
