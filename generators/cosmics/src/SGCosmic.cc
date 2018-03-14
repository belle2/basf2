/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Based on the Belle single track generator by KUNIYA Toshio             *
 * Contributors: Sergey Yashchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <generators/cosmics/SGCosmic.h>

#include <cmath>
#include <limits>
#include <fstream>

#include <framework/dataobjects/Helix.h>
#include <framework/geometry/BFieldManager.h>
#include <TVector3.h>

using namespace std;
using namespace Belle2;

//#define DEBUG
#ifdef DEBUG
ofstream ofs_cosmic1g("cosmic1g.out");
ofstream ofs_cosmic1c("cosmic1c.out");
ofstream ofs_cosmic2g("cosmic2g.out");
ofstream ofs_cosmic2c("cosmic2c.out");
#endif

SGCosmic::SGCosmic()
{
  Parameters m_parameters;
  m_parameters.level = 1;
  m_parameters.ipRequirement = 0;
  m_parameters.ipdr = 3.; // Only relevant for ipRequirement = 1
  m_parameters.ipdz = 3.; // Only relevant for ipRequirement = 1
  m_parameters.ptmin = 0.7;
  m_parameters.cylindricalR = 125.0;
  setParameters(m_parameters);
}


bool SGCosmic::setParameters(const Parameters& p)
{
  // Sanity checks
  bool ok(true);

  // Check that we have correct generator level
  if (p.level != 1 && p.level != 2) {
    B2ERROR("Wrong generator flag");
    ok = false;
  }

  // If everything is ok, set the new parameters, else return false
  if (ok) {
    m_params = p;
    return true;
  }
  return false;
}


// Utility from the Belle cosmics version
double SGCosmic::findMax(const double* dim, const int num)
{
  double max = 0.;
  for (int i = 0; i < num; i++) {
    if (max < dim[i]) max = dim[i];
  }
  return max;
}

bool SGCosmic::generateEvent(MCParticleGraph& graph)
{
  int charge = 0; // Initialization of charge
  double dr, pt, phi, dz, tanl;

  while (true) {

    genCosmic(m_params.level, charge, dr, phi, pt, dz, tanl);

    MCParticleGraph::GraphParticle& p = graph.addParticle();
    p.setStatus(MCParticle::c_PrimaryParticle);

    if (1 == charge) {
      p.setPDG(-13);
    } else if (-1 == charge) {
      p.setPDG(13);
    } else {
      continue;
    }
    p.setMassFromPDG();
    p.setFirstDaughter(0);
    p.setLastDaughter(0);

#ifdef DEBUG
    if (1 == m_params.level) ofs_cosmic1g << charge << " " << dr << " " << phi << " "
                                            << pt << " " << dz << " " << tanl << endl;
    else if (2 == m_params.level) ofs_cosmic2g << charge << " " << dr << " " << phi << " "
                                                 << pt << " " << dz << " " << tanl << endl;
#endif

    // Simulate helix parameter at perigee
    float bz = BFieldManager::getField(0, 0, 0).Z() / Unit::T; // Magnetic field
    const float EPS = 0.0001; // Avoid using zero magnetic field
    if (fabs(bz) < EPS) bz = EPS; // Set the value of the magnetic field to a small number
    float d0, phi0, omega, z0, tanLambda; // The definition is the same as in the Helix class
    d0 = dr;
    phi0 = phi;
    omega = (double)charge / (pt * Helix::getAlpha(bz));
    z0 = dz;
    tanLambda = tanl;
    Helix CosmicMCHelix(d0, phi0, omega, z0, tanLambda);

    const float cylindricalR = m_params.cylindricalR; // radius (cm) of generation
    float arcLength;
    // Get arc length at ToP radius
    arcLength = CosmicMCHelix.getArcLength2DAtCylindricalR(cylindricalR);
    if (isnan(arcLength)) continue;

    // Calculate coordinates and momentum at ToP radius
    TVector3 vector;
    vector = CosmicMCHelix.getPositionAtArcLength2D(arcLength);
    double vx = vector[0];
    double vy = vector[1];
    double vz = vector[2];
    vector = CosmicMCHelix.getMomentumAtArcLength2D(arcLength, bz);
    double px = - vector[0];
    double py = - vector[1];
    double pz = - vector[2];
    double m = p.getMass();
    double e  = sqrt(px * px + py * py + pz * pz + m * m);

#ifdef DEBUG
    if (1 == m_params.level) ofs_cosmic1c << vx << " " << vy << " " << vz << " "
                                            << px << " " << py << " " << pz << endl;
    else if (2 == m_params.level) ofs_cosmic2c << vx << " " << vy << " " << vz << " "
                                                 << px << " " << py << " " << pz << endl;
#endif

    p.setMomentum(px, py, pz);
    p.setEnergy(e);
    p.setProductionVertex(vx, vy, vz);
    p.addStatus(MCParticle::c_StableInGenerator);

    return true;
  }
  return false;
}

void SGCosmic::genCosmic(const int level, int& charge,
                         double& dr,
                         double& phi,
                         double& pt,
                         double& dz,
                         double& tanl)
{
  // generator version 1
  if (level == 1) {
    double muPluse = 361917;
    double muMinus = 285667;
    double fraction = muPluse / (muPluse + muMinus);

    // mu+ or mu- --> fraction = f(mu+)
    charge = muChargeFlag(fraction);

    // make distribution
    double parameters[5]; // dr,phi,pt(not Kappa),dz,tanl
    mkdist_v1(charge, parameters);
    dr = parameters[0];
    phi = parameters[1];
    pt = parameters[2]; // not Kappa
    dz = parameters[3];
    tanl = parameters[4];
  }

  // generator version 2
  if (level == 2) {
    double muPluse = 143391;
    double muMinus = 143302;
    double fraction = muPluse / (muPluse + muMinus);

    // mu+ or mu- --> fraction = f(mu+)
    charge = muChargeFlag(fraction);

    // make distribution
    double parameters[5]; // dr,phi,pt(not Kappa),dz,tanl
    mkdist_v2(charge, parameters);
    dr = parameters[0];
    phi = parameters[1];
    pt = parameters[2]; // not Kappa
    dz = parameters[3];
    tanl = parameters[4];
  }

  return;
}

int SGCosmic::muChargeFlag(const double fraction)
{
  double rand1;
  rand1 = gRandom->Uniform(0., 1.);
  if ((double)rand1 > fraction) return -1;
  else return 1;
}

// -------- version 1 --------
void SGCosmic::mkdist_v1(const int charge, double* parameters)
{
  double rand1;
  double rand2;

  for (int i = 0; i < 5; i++) {
    int success;
    success = 0;
    do {
      rand1 = gRandom->Uniform(0., 1.);
      rand2 = gRandom->Uniform(0., 1.);
      if (i == 0) { // dr
        if (m_params.ipRequirement == 0) {
          parameters[0] = rand1 * 40. - 20.; // (-20, 20)
        } else {
          // IP requirement
          double widthdr = m_params.ipdr * 2;
          parameters[0] = rand1 * widthdr - m_params.ipdr; // (-ipdr, ipdr)
        }
        if (charge == 1)
          success = mkDr_pos_v1(parameters[0], rand2);
        else
          success = mkDr_neg_v1(parameters[0], rand2);
      }
      if (i == 1) { // phi
        parameters[1] = rand1 * M_PI; // (0, pi)
        if (charge == 1)
          success = mkPhi_pos_v1(parameters[1], rand2);
        else
          success = mkPhi_neg_v1(parameters[1], rand2);
      }
      if (i == 2) { // pt
        parameters[2] = (30.0 - m_params.ptmin) * rand1 + m_params.ptmin; // (ptmin, 30)
        if (charge == 1)
          success = mkPt_pos_v1(parameters[2], rand2);
        else
          success = mkPt_neg_v1(parameters[2], rand2);
      }
      if (i == 3) { // dz
        if (m_params.ipRequirement == 0) {
          parameters[3] = rand1 * 150. - 50.; //(-50, 100)
        } else {
          // IP requirement
          double widthdz = m_params.ipdz * 2;
          parameters[3] = rand1 * widthdz - m_params.ipdz; //(-ipdz, ipdz)
        }
        if (charge == 1)
          success = mkDz_pos_v1(parameters[3], rand2);
        else
          success = mkDz_neg_v1(parameters[3], rand2);
      }
      if (i == 4) { // tanl
        parameters[4] = (rand1 - 0.5) * 6; // (-3, 3)
        if (charge == 1)
          success = mkTanl_pos_v1(parameters[4], rand2);
        else
          success = mkTanl_neg_v1(parameters[4], rand2);
      }
    } while (success == 0);
  }
  return;
}


int SGCosmic::mkDr_pos_v1(const double dr, const float rndm)
{
  const int nch = 100;
  const int width = 40;
  double dr_pos[nch] = {0.0,      0.0,      0.0,      0.0,      0.0,
                        0.0,      0.0,      0.0,      0.0,      0.0,
                        0.0,      0.0,      0.0,      0.0,      0.0,
                        0.0,      0.0,      0.0,      0.0,      0.0,
                        0.0,      0.0,      0.0,      0.0,      0.0,
                        0.0,      0.0,      0.0,      0.0,      0.0,
                        3284.0,   4110.0,   4545.0,   5140.0,   5601.0,
                        6304.0,   6840.0,   7142.0,   7714.0,   8166.0,
                        8795.0,   9309.0,   9600.0,   9759.0,   9892.0,
                        9995.0,  10198.0,  10379.0,  10428.0,  10646.0,
                        10791.0,  11002.0,  11172.0,  11332.0,  11334.0,
                        11381.0,  11226.0,  11241.0,  10958.0,  10840.0,
                        10619.0,  10246.0,  10135.0,   9640.0,   9477.0,
                        9215.0,   8689.0,   8610.0,   8150.0,   7679.0,
                        359.0,      2.0,      0.0,      0.0,      0.0,
                        0.0,      0.0,      0.0,      0.0,      0.0,
                        0.0,      0.0,      0.0,      0.0,      0.0,
                        0.0,      0.0,      0.0,      0.0,      0.0,
                        0.0,      0.0,      0.0,      0.0,      0.0,
                        0.0,      0.0,      0.0,      0.0,      0.0
                       };
  // normalize by peak value --> max = 1
  double max = findMax(dr_pos, nch);
  for (int i = 0; i < nch; i++) dr_pos[i] /= max;
  int index = (int)((double)nch / (double)width * (dr + 20.));

  if (dr_pos[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkDr_neg_v1(const double dr, const float rndm)
{
  const int nch = 100;
  const int width = 40;
  double dr_neg[nch] = { 0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         2735.0,   3081.0,   3568.0,   3837.0,   4445.0,
                         4938.0,   5338.0,   5701.0,   5985.0,   6404.0,
                         7022.0,   7375.0,   7438.0,   7662.0,   7833.0,
                         7951.0,   8166.0,   8186.0,   8101.0,   8159.0,
                         8541.0,   8687.0,   8855.0,   8908.0,   8998.0,
                         9014.0,   8962.0,   8915.0,   8833.0,   8698.0,
                         8399.0,   8285.0,   7875.0,   7726.0,   7346.0,
                         7058.0,   7074.0,   6887.0,   6546.0,   5910.0,
                         263.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0
                       };
  // normalize by peak value --> max = 1
  double max = findMax(dr_neg, nch);
  for (int i = 0; i < nch; i++) dr_neg[i] /= max;
  int index = (int)((double)nch / (double)width * (dr + 20.));

  if (dr_neg[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkPhi_pos_v1(const double phi, const float rndm)
{
  const int nch = 100;
  const double width = M_PI;
  double phi_pos[nch] = { 95.0,    194.0,    287.0,    433.0,    636.0,
                          839.0,   1190.0,   1486.0,   1795.0,   2090.0,
                          2192.0,   2437.0,   2831.0,   3114.0,   3407.0,
                          3589.0,   3706.0,   3796.0,   4125.0,   4292.0,
                          4650.0,   4898.0,   4877.0,   5003.0,   5037.0,
                          5409.0,   5488.0,   5704.0,   5733.0,   5547.0,
                          5704.0,   5769.0,   5919.0,   5993.0,   5976.0,
                          5965.0,   6011.0,   6036.0,   6069.0,   6450.0,
                          6260.0,   6103.0,   6252.0,   6277.0,   6219.0,
                          6364.0,   6287.0,   6298.0,   6033.0,   5894.0,
                          5883.0,   6036.0,   6032.0,   5894.0,   5748.0,
                          5229.0,   5382.0,   5374.0,   5504.0,   5283.0,
                          5074.0,   4700.0,   4651.0,   4847.0,   4698.0,
                          4446.0,   4410.0,   4104.0,   3870.0,   3790.0,
                          3826.0,   3750.0,   3488.0,   3220.0,   2973.0,
                          2862.0,   2718.0,   2616.0,   2400.0,   2168.0,
                          1927.0,   1860.0,   1708.0,   1667.0,   1334.0,
                          1162.0,    965.0,    787.0,    629.0,    554.0,
                          384.0,    333.0,    243.0,    159.0,    139.0,
                          88.0,     59.0,     50.0,     27.0,      9.0
                        };
  // normalize by peak value --> max = 1
  double max = findMax(phi_pos, nch);
  for (int i = 0; i < nch; i++) phi_pos[i] /= max;
  int index = (int)((double)nch / width * phi);

  if (phi_pos[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkPhi_neg_v1(const double phi, const float rndm)
{
  const int nch = 100;
  const double width = M_PI;
  double phi_neg[nch] = { 351.0,    483.0,    520.0,    506.0,    541.0,
                          497.0,    475.0,    503.0,    495.0,    532.0,
                          594.0,    638.0,    787.0,    904.0,   1094.0,
                          1186.0,   1250.0,   1349.0,   1494.0,   1658.0,
                          1807.0,   1929.0,   2031.0,   2077.0,   2292.0,
                          2402.0,   2592.0,   2790.0,   2988.0,   2908.0,
                          2968.0,   3107.0,   3264.0,   3534.0,   3505.0,
                          3582.0,   3608.0,   3649.0,   3772.0,   3827.0,
                          4023.0,   3960.0,   3956.0,   3965.0,   4095.0,
                          4392.0,   4504.0,   4428.0,   4364.0,   4510.0,
                          4633.0,   4767.0,   4968.0,   4866.0,   4889.0,
                          4753.0,   4862.0,   4928.0,   5015.0,   4958.0,
                          4960.0,   4921.0,   4763.0,   4838.0,   4841.0,
                          4779.0,   4627.0,   4679.0,   4478.0,   4528.0,
                          4470.0,   4474.0,   4199.0,   4145.0,   3896.0,
                          3882.0,   3812.0,   3839.0,   3466.0,   3274.0,
                          3067.0,   3043.0,   2914.0,   2776.0,   2677.0,
                          2425.0,   2195.0,   1892.0,   1777.0,   1685.0,
                          1413.0,   1296.0,   1123.0,   1017.0,    795.0,
                          739.0,    665.0,    493.0,    395.0,    251.0
                        };
  // normalize by peak value --> max = 1
  double max = findMax(phi_neg, nch);
  for (int i = 0; i < nch; i++) phi_neg[i] /= max;
  int index = (int)((double)nch / width * phi);

  if (phi_neg[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkPt_pos_v1(const double pt, const float rndm)
{
  const int nch = 50;
  const int width = 30;
  double pt_pos[nch] = { 47.0,  22547.0,  28678.0,  26159.0,  25145.0,
                         23668.0,  21581.0,  19197.0,  16847.0,  14902.0,
                         13020.0,  11836.0,  10397.0,   9414.0,   8356.0,
                         7492.0,   6640.0,   6173.0,   5426.0,   5023.0,
                         4612.0,   4340.0,   3860.0,   3656.0,   3222.0,
                         3042.0,   2846.0,   2655.0,   2422.0,   2288.0,
                         2172.0,   1984.0,   1841.0,   1745.0,   1599.0,
                         1495.0,   1494.0,   1366.0,   1305.0,   1186.0,
                         1153.0,   1063.0,   1051.0,    996.0,    952.0,
                         906.0,    883.0,    783.0,    733.0,    682.0
                       };
  // normalize by peak value --> max = 1
  double max = findMax(pt_pos, nch);
  for (int i = 0; i < nch; i++) pt_pos[i] /= max;
  int index = (int)((double)nch / width * pt);

  if (pt_pos[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkPt_neg_v1(const double pt, const float rndm)
{
  const int nch = 50;
  const int width = 30;
  double pt_neg[nch] = { 538.0,    614.0,    600.0,    626.0,    648.0,
                         729.0,    716.0,    760.0,    828.0,    882.0,
                         914.0,    922.0,   1009.0,   1105.0,   1113.0,
                         1246.0,   1305.0,   1345.0,   1523.0,   1578.0,
                         1649.0,   1856.0,   1968.0,   2126.0,   2319.0,
                         2526.0,   2716.0,   3005.0,   3317.0,   3427.0,
                         3869.0,   4287.0,   4727.0,   5184.0,   5882.0,
                         6299.0,   7191.0,   8074.0,   9017.0,   9968.0,
                         11425.0,  12821.0,  14601.0,  16351.0,  17885.0,
                         19838.0,  20766.0,  23697.0,  22861.0,    116.0
                       };
  // normalize by peak value --> max = 1
  double max = findMax(pt_neg, nch);
  for (int i = 0; i < nch; i++) pt_neg[i] /= max;
  int index = (int)((double)nch / width * (width - pt));

  if (pt_neg[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkDz_pos_v1(const double dz, const float rndm)
{
  const int nch = 100;
  const int width = 250;
  double dz_pos[nch] = { 0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         1294.0,   1796.0,   2018.0,   2146.0,   2280.0,
                         2422.0,   2601.0,   2913.0,   3076.0,   3564.0,
                         5147.0,   6284.0,   6958.0,   7133.0,   7465.0,
                         7481.0,   7878.0,   7947.0,   8297.0,   8589.0,
                         8366.0,   8540.0,   8717.0,   8825.0,   8884.0,
                         8985.0,   9131.0,   9105.0,   9279.0,   9333.0,
                         9329.0,   9501.0,   9442.0,   9484.0,   9471.0,
                         9322.0,   9201.0,   8344.0,   6855.0,   5879.0,
                         5095.0,   5012.0,   5044.0,   4967.0,   4717.0,
                         4928.0,   4917.0,   4762.0,   4579.0,   4556.0,
                         4461.0,   4366.0,   4324.0,   4185.0,   4185.0,
                         4085.0,   4019.0,   3974.0,   3748.0,   2739.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0
                       };
  // normalize by peak value --> max = 1
  double max = findMax(dz_pos, nch);
  for (int i = 0; i < nch; i++) dz_pos[i] /= max;
  int index = (int)((double)nch / (double)width * (dz + 100.));

  if (dz_pos[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkDz_neg_v1(const double dz, const float rndm)
{
  const int nch = 100;
  const int width = 250;
  double dz_neg[nch] = { 0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         967.0,   1357.0,   1507.0,   1618.0,   1736.0,
                         1926.0,   2090.0,   2182.0,   2390.0,   2748.0,
                         4261.0,   5004.0,   5605.0,   5819.0,   5932.0,
                         5946.0,   6214.0,   6281.0,   6521.0,   6626.0,
                         6743.0,   6830.0,   6912.0,   7217.0,   7089.0,
                         7194.0,   7340.0,   7283.0,   7343.0,   7357.0,
                         7141.0,   7545.0,   7476.0,   7608.0,   7415.0,
                         7369.0,   7365.0,   6827.0,   5364.0,   4520.0,
                         4052.0,   3994.0,   3885.0,   3820.0,   3743.0,
                         3747.0,   3543.0,   3604.0,   3580.0,   3479.0,
                         3467.0,   3395.0,   3409.0,   3365.0,   3231.0,
                         3247.0,   3192.0,   3074.0,   2995.0,   2215.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0,
                         0.0,      0.0,      0.0,      0.0,      0.0
                       };
  // normalize by peak value --> max = 1
  double max = findMax(dz_neg, nch);
  for (int i = 0; i < nch; i++) dz_neg[i] /= max;
  int index = (int)((double)nch / (double)width * (dz + 100.));

  if (dz_neg[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkTanl_pos_v1(const double tanl, const float rndm)
{
  const int nch = 100;
  const int width = 6;
  double tanl_pos[nch] = { 0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      1.0,
                           1.0,      1.0,      2.0,      6.0,     15.0,
                           32.0,     30.0,    103.0,    375.0,    906.0,
                           1465.0,   2088.0,   2858.0,   4007.0,   5324.0,
                           7025.0,   8801.0,  11256.0,  13767.0,  16230.0,
                           18402.0,  20313.0,  21764.0,  22369.0,  23219.0,
                           22975.0,  23058.0,  22121.0,  20633.0,  18402.0,
                           16315.0,  14098.0,  11450.0,   8844.0,   6793.0,
                           5207.0,   3937.0,   2850.0,   1990.0,   1464.0,
                           921.0,    384.0,     83.0,     23.0,     18.0,
                           6.0,      3.0,      2.0,      2.0,      4.0,
                           1.0,      0.0,      1.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0
                         };
  // normalize by peak value --> max = 1
  double max = findMax(tanl_pos, nch);
  for (int i = 0; i < nch; i++) tanl_pos[i] /= max;
  int index = (int)((double)nch / (double)width * (tanl + 3.));

  if (tanl_pos[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkTanl_neg_v1(const double tanl, const float rndm)
{
  const int nch = 100;
  const int width = 6;
  double tanl_neg[nch] = { 0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           1.0,      1.0,      0.0,      7.0,     10.0,
                           11.0,     24.0,     94.0,    335.0,    760.0,
                           1210.0,   1713.0,   2233.0,   3242.0,   4236.0,
                           5530.0,   7004.0,   8768.0,  10923.0,  12809.0,
                           14457.0,  16043.0,  17321.0,  18064.0,  18237.0,
                           18129.0,  17823.0,  17077.0,  16272.0,  14539.0,
                           13015.0,  11031.0,   9000.0,   7219.0,   5411.0,
                           4163.0,   3047.0,   2120.0,   1551.0,   1215.0,
                           672.0,    266.0,     73.0,     23.0,      5.0,
                           4.0,      8.0,      5.0,      3.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0,
                           0.0,      0.0,      0.0,      0.0,      0.0
                         };
  // normalize by peak value --> max = 1
  double max = findMax(tanl_neg, nch);
  for (int i = 0; i < nch; i++) tanl_neg[i] /= max;
  int index = (int)((double)nch / (double)width * (tanl + 3.));

  if (tanl_neg[index] > rndm) return 1;
  return 0;
}

// -------- version 2 --------
void SGCosmic::mkdist_v2(const int charge, double* parameters)
{
  double rand1;
  double rand2;

  for (int i = 0; i < 5; i++) {
    int success;
    success = 0;
    do {
      rand1 = gRandom->Uniform(0., 1.);
      rand2 = gRandom->Uniform(0., 1.);
      if (i == 0) { // dr
        parameters[0] = rand1 * 6. - 3.; // (-3, 3)
        if (charge == 1)
          success = mkDr_pos_v2(parameters[0], rand2);
        else
          success = mkDr_neg_v2(parameters[0], rand2);
      }
      if (i == 1) { // phi
        parameters[1] = rand1 * M_PI; // (0, pi)
        if (charge == 1)
          success = mkPhi_pos_v2(parameters[1], rand2);
        else
          success = mkPhi_neg_v2(parameters[1], rand2);
      }
      if (i == 2) { // pt
        parameters[2] = 50. * rand1; // (0., 50)
        if (charge == 1)
          success = mkPt_pos_v2(parameters[2], rand2);
        else
          success = mkPt_neg_v2(parameters[2], rand2);
      }
      if (i == 3) { // dz
        parameters[3] = rand1 * 80. - 30.; // (-30, 50)
        if (charge == 1)
          success = mkDz_pos_v2(parameters[3], rand2);
        else
          success = mkDz_neg_v2(parameters[3], rand2);
      }
      if (i == 4) { // tanl
        parameters[4] = rand1 * 4 - 2; // (-2, 2)
        if (charge == 1)
          success = mkTanl_pos_v2(parameters[4], rand2);
        else
          success = mkTanl_neg_v2(parameters[4], rand2);
      }
    } while (success == 0);
  }
  return;
}

int SGCosmic::mkDr_pos_v2(const double dr, const float rndm)
{
  const int nch = 50;
  const int width = 6;
  double dr_pos[nch] = {0., 0., 0., 0., 0.,
                        0., 2403.000, 3562.000, 3685.000, 3619.000,
                        3667.000, 3724.000, 3823.000, 3824.000, 3881.000,
                        3764.000, 3867.000, 3918.000, 3964.000, 3916.000,
                        4002.000, 4001.000, 3980.000, 3848.000, 4220.000,
                        4206.000, 3856.000, 3920.000, 3857.000, 3751.000,
                        3753.000, 3898.000, 3898.000, 3822.000, 3872.000,
                        3905.000, 3896.000, 3874.000, 3767.000, 3734.000,
                        3744.000, 3666.000, 3757.000, 2458.000, 0.,
                        0., 0., 0., 0., 0
                       };
  // normalize by peak value --> max = 1
  double max = findMax(dr_pos, nch);
  for (int i = 0; i < nch; i++) dr_pos[i] /= max;
  int index = (int)((double)nch / (double)width * (dr + 3.));

  if (dr_pos[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkDr_neg_v2(const double dr, const float rndm)
{
  const int nch = 50;
  const int width = 6;
  double dr_neg[nch] = { 0., 0., 0., 0., 0.,
                         0., 2508.000, 3752.000, 3713.000, 3695.000,
                         3758.000, 3727.000, 3847.000, 3887.000, 3896.000,
                         3862.000, 3858.000, 3905.000, 3894.000, 3806.000,
                         3736.000, 3797.000, 3921.000, 3922.000, 4203.000,
                         4167.000, 3927.000, 3952.000, 3983.000, 3982.000,
                         3914.000, 3962.000, 3983.000, 3817.000, 3831.000,
                         3811.000, 3837.000, 3852.000, 3728.000, 3684.000,
                         3629.000, 3663.000, 3588.000, 2394.000, 0.,
                         0., 0., 0., 0., 0.
                       };
  // normalize by peak value --> max = 1
  double max = findMax(dr_neg, nch);
  for (int i = 0; i < nch; i++) dr_neg[i] /= max;
  int index = (int)((double)nch / (double)width * (dr + 3.));

  if (dr_neg[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkPhi_pos_v2(const double phi, const float rndm)
{
  const int nch = 50;
  const double width = 8;
  double phi_pos[nch] = { 0., 0., 0., 0., 0.,
                          1073.000, 1911.000, 2905.000, 4165.000, 5205.000,
                          6062.000, 6955.000, 7382.000, 7262.000, 7142.000,
                          6548.000, 5759.000, 5015.000, 4399.000, 3392.000,
                          2365.000, 1533.000, 801.0000, 245.0000, 25.00000,
                          39.00000, 119.0000, 398.0000, 841.0000, 1487.000,
                          2331.000, 3138.000, 3726.000, 4430.000, 5106.000,
                          5548.000, 5766.000, 5699.000, 5408.000, 5011.000,
                          4439.000, 3732.000, 2883.000, 2034.000, 1023.000,
                          0., 0., 0., 0., 0.
                        };
  // normalize by peak value --> max = 1
  double max = findMax(phi_pos, nch);
  for (int i = 0; i < nch; i++) phi_pos[i] /= max;
  int index = (int)(nch / width * (phi + 4.));

  if (phi_pos[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkPhi_neg_v2(const double phi, const float rndm)
{
  const int nch = 50;
  const double width = 8;
  double phi_neg[nch] = { 0., 0., 0., 0., 0.,
                          16.00000, 73.00000, 262.0000, 677.0000, 1252.000,
                          1973.000, 2896.000, 3520.000, 4173.000, 4876.000,
                          5389.000, 5730.000, 5798.000, 5553.000, 5120.000,
                          4676.000, 4036.000, 3185.000, 2320.000, 1688.000,
                          1741.000, 2164.000, 3325.000, 4588.000, 5522.000,
                          6427.000, 7159.000, 7385.000, 7282.000, 6980.000,
                          6157.000, 5496.000, 4854.000, 4066.000, 3042.000,
                          2032.000, 1236.000, 586.0000, 123.0000, 13.00000,
                          0., 0., 0., 0., 0.
                        };
  // normalize by peak value --> max = 1
  double max = findMax(phi_neg, nch);
  for (int i = 0; i < nch; i++) phi_neg[i] /= max;
  int index = (int)(nch / width * (phi + 4.));

  if (phi_neg[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkPt_pos_v2(const double pt, const float rndm)
{
  const int nch = 100;
  const int width = 50;
  double pt_pos[nch] = { 3813.000, 14309.00, 14985.00, 13103.00, 10807.00,
                         9118.000, 7793.000, 6718.000, 5788.000, 5177.000,
                         4458.000, 3889.000, 3480.000, 3191.000, 2798.000,
                         2484.000, 2245.000, 2053.000, 1883.000, 1691.000,
                         1481.000, 1416.000, 1311.000, 1207.000, 1171.000,
                         1017.000, 953.0000, 886.0000, 790.0000, 816.0000,
                         712.0000, 685.0000, 627.0000, 577.0000, 526.0000,
                         502.0000, 442.0000, 453.0000, 415.0000, 351.0000,
                         340.0000, 358.0000, 305.0000, 330.0000, 278.0000,
                         272.0000, 250.0000, 273.0000, 243.0000, 209.0000,
                         203.0000, 176.0000, 212.0000, 182.0000, 153.0000,
                         168.0000, 155.0000, 170.0000, 129.0000, 117.0000,
                         120.0000, 114.0000, 126.0000, 123.0000, 106.0000,
                         90.00000, 97.00000, 103.0000, 82.00000, 87.00000,
                         76.00000, 104.0000, 75.00000, 82.00000, 67.00000,
                         64.00000, 63.00000, 75.00000, 65.00000, 62.00000,
                         54.00000, 86.00000, 44.00000, 44.00000, 53.00000,
                         54.00000, 53.00000, 51.00000, 49.00000, 39.00000,
                         37.00000, 42.00000, 40.00000, 40.00000, 24.00000,
                         38.00000, 39.00000, 30.00000, 28.00000, 32.00000
                       };
  // normalize by peak value --> max = 1
  double max = findMax(pt_pos, nch);
  for (int i = 0; i < nch; i++) pt_pos[i] /= max;
  int index = (int)(nch / width * pt);

  if (pt_pos[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkPt_neg_v2(const double pt, const float rndm)
{
  const int nch = 100;
  const int width = 50;
  double pt_neg[nch] = { 3794.000, 14288.00, 15015.00, 13120.00, 10865.00,
                         9167.000, 7826.000, 6733.000, 5760.000, 5253.000,
                         4506.000, 3796.000, 3512.000, 3197.000, 2848.000,
                         2463.000, 2232.000, 2083.000, 1827.000, 1679.000,
                         1556.000, 1414.000, 1299.000, 1163.000, 1116.000,
                         1069.000, 964.0000, 922.0000, 818.0000, 764.0000,
                         723.0000, 658.0000, 614.0000, 541.0000, 552.0000,
                         520.0000, 491.0000, 382.0000, 380.0000, 392.0000,
                         346.0000, 352.0000, 321.0000, 298.0000, 272.0000,
                         277.0000, 246.0000, 238.0000, 237.0000, 241.0000,
                         189.0000, 208.0000, 196.0000, 194.0000, 169.0000,
                         149.0000, 143.0000, 139.0000, 130.0000, 134.0000,
                         125.0000, 116.0000, 110.0000, 104.0000, 122.0000,
                         110.0000, 99.00000, 95.00000, 91.00000, 87.00000,
                         91.00000, 75.00000, 87.00000, 76.00000, 81.00000,
                         81.00000, 67.00000, 64.00000, 55.00000, 59.00000,
                         52.00000, 53.00000, 53.00000, 51.00000, 46.00000,
                         40.00000, 49.00000, 41.00000, 37.00000, 44.00000,
                         44.00000, 42.00000, 47.00000, 30.00000, 30.00000,
                         42.00000, 28.00000, 28.00000, 32.00000, 26.00000
                       };
  // normalize by peak value --> max = 1
  double max = findMax(pt_neg, nch);
  for (int i = 0; i < nch; i++) pt_neg[i] /= max;
  int index = (int)(nch / width * pt);

  if (pt_neg[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkDz_pos_v2(const double dz, const float rndm)
{
  const int nch = 200;
  const int width = 100;
  double dz_pos[nch] = { 0., 0., 0., 1.000000, 0.,
                         0., 0., 0., 0., 0.,
                         0., 0., 0., 0., 0.,
                         0., 0., 0., 0., 0.,
                         0., 0., 0., 0., 0.,
                         0., 0., 0., 0., 0.,
                         0., 0., 0., 0., 0.,
                         0., 0., 0., 1.000000, 0.,
                         1.000000, 0., 0., 4.000000, 1.000000,
                         1.000000, 2.000000, 1.000000, 3.000000, 6.000000,
                         20.00000, 11.00000, 30.00000, 45.00000, 64.00000,
                         139.0000, 243.0000, 391.0000, 460.0000, 586.0000,
                         720.0000, 790.0000, 931.0000, 967.0000, 1087.000,
                         1204.000, 1231.000, 1272.000, 1251.000, 1355.000,
                         1453.000, 1447.000, 1370.000, 1364.000, 1460.000,
                         1405.000, 1545.000, 1454.000, 1536.000, 1499.000,
                         1547.000, 1500.000, 1465.000, 1512.000, 1515.000,
                         1547.000, 1548.000, 1554.000, 1579.000, 1599.000,
                         1590.000, 1609.000, 1618.000, 1710.000, 1599.000,
                         1746.000, 1669.000, 1709.000, 1814.000, 1904.000,
                         1896.000, 1634.000, 1598.000, 1680.000, 1727.000,
                         1714.000, 1702.000, 1737.000, 1693.000, 1743.000,
                         1713.000, 1663.000, 1639.000, 1569.000, 1695.000,
                         1697.000, 1745.000, 1708.000, 1682.000, 1686.000,
                         1676.000, 1638.000, 1660.000, 1655.000, 1592.000,
                         1617.000, 1581.000, 1514.000, 1601.000, 1553.000,
                         1451.000, 1528.000, 1456.000, 1489.000, 1465.000,
                         1455.000, 1489.000, 1375.000, 1387.000, 1330.000,
                         1298.000, 1304.000, 1237.000, 1140.000, 1111.000,
                         1038.000, 942.0000, 959.0000, 867.0000, 822.0000,
                         773.0000, 698.0000, 649.0000, 591.0000, 556.0000,
                         513.0000, 474.0000, 424.0000, 408.0000, 414.0000,
                         324.0000, 296.0000, 265.0000, 259.0000, 188.0000,
                         170.0000, 195.0000, 163.0000, 140.0000, 131.0000,
                         122.0000, 113.0000, 98.00000, 92.00000, 68.00000,
                         61.00000, 51.00000, 44.00000, 46.00000, 30.00000,
                         32.00000, 25.00000, 20.00000, 13.00000, 11.00000,
                         3.000000, 2.000000, 1.000000, 1.000000, 0.,
                         0., 0., 0., 0., 0.,
                         0., 0., 0., 0., 0.
                       };
  // normalize by peak value --> max = 1
  double max = findMax(dz_pos, nch);
  for (int i = 0; i < nch; i++) dz_pos[i] /= max;
  int index = (int)((double)nch / (double)width * (dz + 50.));

  if (dz_pos[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkDz_neg_v2(const double dz, const float rndm)
{
  const int nch = 100;
  const int width = 100;
  double dz_neg[nch] = { 0., 1.000000, 0., 0., 0.,
                         0., 0., 0., 0., 0.,
                         0., 0., 0., 0., 0.,
                         0., 0., 0., 0., 1.000000,
                         2.000000, 1.000000, 2.000000, 5.000000, 12.00000,
                         25.00000, 66.00000, 244.0000, 609.0000, 1092.000,
                         1592.000, 1849.000, 2264.000, 2537.000, 2692.000,
                         2889.000, 2711.000, 2901.000, 3015.000, 2976.000,
                         3061.000, 3001.000, 3066.000, 3058.000, 3146.000,
                         3198.000, 3308.000, 3349.000, 3391.000, 3720.000,
                         3552.000, 3291.000, 3404.000, 3459.000, 3447.000,
                         3318.000, 3248.000, 3416.000, 3408.000, 3408.000,
                         3334.000, 3345.000, 3251.000, 3127.000, 2999.000,
                         3038.000, 2944.000, 2970.000, 2921.000, 2603.000,
                         2633.000, 2304.000, 2209.000, 1866.000, 1660.000,
                         1511.000, 1269.000, 1065.000, 893.0000, 765.0000,
                         628.0000, 456.0000, 416.0000, 378.0000, 273.0000,
                         221.0000, 166.0000, 134.0000, 116.0000, 62.00000,
                         46.00000, 35.00000, 11.00000, 5.000000, 0.,
                         0., 0., 0., 1.000000, 0.
                       };
  // normalize by peak value --> max = 1
  double max = findMax(dz_neg, nch);
  for (int i = 0; i < nch; i++) dz_neg[i] /= max;
  int index = (int)((double)nch / (double)width * (dz + 50.));

  if (dz_neg[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkTanl_pos_v2(const double tanl, const float rndm)
{
  const int nch = 100;
  const int width = 5;
  double tanl_pos[nch] = { 0., 0., 0., 0., 0.,
                           1.000000, 0., 2.000000, 0., 7.000000,
                           2.000000, 6.000000, 4.000000, 17.00000, 20.00000,
                           21.00000, 18.00000, 34.00000, 36.00000, 47.00000,
                           64.00000, 85.00000, 126.0000, 116.0000, 163.0000,
                           214.0000, 262.0000, 330.0000, 415.0000, 477.0000,
                           556.0000, 641.0000, 852.0000, 956.0000, 1243.000,
                           1370.000, 1699.000, 1995.000, 2415.000, 3040.000,
                           3352.000, 3889.000, 4390.000, 4823.000, 5399.000,
                           5862.000, 6201.000, 6506.000, 6748.000, 6788.000,
                           6767.000, 6575.000, 6479.000, 6112.000, 5940.000,
                           5459.000, 4986.000, 4504.000, 3879.000, 3460.000,
                           2981.000, 2502.000, 2121.000, 1803.000, 1477.000,
                           1248.000, 1041.000, 927.0000, 708.0000, 605.0000,
                           461.0000, 424.0000, 341.0000, 257.0000, 230.0000,
                           167.0000, 141.0000, 119.0000, 84.00000, 80.00000,
                           46.00000, 44.00000, 39.00000, 30.00000, 17.00000,
                           18.00000, 13.00000, 6.000000, 6.000000, 5.000000,
                           2.000000, 2.000000, 2.000000, 0., 0.,
                           0., 0., 0., 0., 1.000000
                         };
  // normalize by peak value --> max = 1
  double max = findMax(tanl_pos, nch);
  for (int i = 0; i < nch; i++) tanl_pos[i] /= max;
  int index = (int)((double)nch / (double)width * (tanl + 2.5));

  if (tanl_pos[index] > rndm) return 1;
  return 0;
}

int SGCosmic::mkTanl_neg_v2(const double tanl, const float rndm)
{
  const int nch = 100;
  const int width = 5;
  double tanl_neg[nch] = { 0., 0., 0., 0., 0.,
                           0., 0., 1.000000, 4.000000, 0.,
                           3.000000, 4.000000, 9.000000, 8.000000, 14.00000,
                           21.00000, 25.00000, 41.00000, 44.00000, 49.00000,
                           73.00000, 84.00000, 107.0000, 139.0000, 166.0000,
                           242.0000, 268.0000, 316.0000, 412.0000, 469.0000,
                           595.0000, 727.0000, 900.0000, 1002.000, 1267.000,
                           1454.000, 1793.000, 2126.000, 2500.000, 2966.000,
                           3492.000, 3829.000, 4505.000, 4980.000, 5518.000,
                           5864.000, 6157.000, 6440.000, 6664.000, 6747.000,
                           6841.000, 6757.000, 6454.000, 6224.000, 5776.000,
                           5458.000, 4855.000, 4399.000, 3887.000, 3323.000,
                           3098.000, 2445.000, 1993.000, 1695.000, 1381.000,
                           1200.000, 993.0000, 844.0000, 640.0000, 590.0000,
                           484.0000, 412.0000, 352.0000, 255.0000, 207.0000,
                           160.0000, 121.0000, 119.0000, 103.0000, 57.00000,
                           53.00000, 37.00000, 27.00000, 26.00000, 25.00000,
                           23.00000, 8.000000, 7.000000, 9.000000, 3.000000,
                           5.000000, 4.000000, 2.000000, 3.000000, 3.000000,
                           0., 0., 0., 0., 0.
                         };
  // normalize by peak value --> max = 1
  double max = findMax(tanl_neg, nch);
  for (int i = 0; i < nch; i++) tanl_neg[i] /= max;
  int index = (int)((double)nch / (double)width * (tanl + 2.5));

  if (tanl_neg[index] > rndm) return 1;
  return 0;
}
