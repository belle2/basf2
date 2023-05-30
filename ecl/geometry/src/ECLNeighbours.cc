/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/geometry/ECLGeometryPar.h>

/* Basf2 headers. */
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <Math/Vector3D.h>
#include <Math/VectorUtil.h>
#include <TMath.h>

using namespace Belle2;
using namespace ECL;

// Constructor.
ECLNeighbours::ECLNeighbours(const std::string& neighbourDef, const double par, const bool sorted)
{
  // resize the vector
  std::vector<short int> fakeneighbours;
  fakeneighbours.push_back(0); // insert one fake to avoid the "0" entry
  m_neighbourMap.push_back(fakeneighbours);

  int parToInt = int(par);

  // fixed number of neighbours:
  if (neighbourDef == "N") {
    B2DEBUG(150, "ECLNeighbours::ECLNeighbours: initialize " << neighbourDef << ", n x n: " << parToInt * 2 + 1 << " x " << parToInt * 2
            + 1);
    if ((parToInt >= 0) and (parToInt < 11)) initializeN(parToInt, sorted);
    else B2FATAL("ECLNeighbours::ECLNeighbours: " << LogVar("parameter", parToInt) << "Invalid parameter (must be between 0 and 10)!");
  } else if (neighbourDef == "NC") {
    B2DEBUG(150, "ECLNeighbours::ECLNeighbours: initialize " << neighbourDef << ", n x n (minus corners): " << parToInt * 2 + 1 << " x "
            <<
            parToInt * 2 + 1);
    if ((parToInt >= 0) and (parToInt < 11)) initializeNC(parToInt);
    else B2FATAL("ECLNeighbours::ECLNeighbours: " << LogVar("parameter", parToInt) << "Invalid parameter (must be between 0 and 10)!");
  } else if (neighbourDef == "NLegacy") {
    B2DEBUG(150, "ECLNeighbours::ECLNeighbours: initialize " << neighbourDef << ", n x n: " << parToInt * 2 + 1 << " x " << parToInt * 2
            + 1);
    if ((parToInt >= 0) and (parToInt < 11)) initializeNLegacy(parToInt);
    else B2FATAL("ECLNeighbours::ECLNeighbours: " << LogVar("parameter", parToInt) << "Invalid parameter (must be between 0 and 10)!");
  } else if (neighbourDef == "NCLegacy") {
    B2DEBUG(150, "ECLNeighbours::ECLNeighbours: initialize " << neighbourDef << ", n x n (minus corners): " << parToInt * 2 + 1 << " x "
            <<
            parToInt * 2 + 1);
    if ((parToInt >= 0) and (parToInt < 11)) initializeNCLegacy(parToInt, 1);
    else B2FATAL("ECLNeighbours::ECLNeighbours: " << LogVar("parameter", parToInt) << "Invalid parameter (must be between 0 and 10)!");
  }
  // or neighbours depend on the distance:
  else if (neighbourDef == "R") {
    B2DEBUG(150, "ECLNeighbours::ECLNeighbours: initialize " << neighbourDef << ", radius: " << par << " cm");
    if ((par > 0.0) and (par < 30.0 * Belle2::Unit::cm)) initializeR(par);
    else B2FATAL("ECLNeighbours::ECLNeighbours: " << par << " is an invalid parameter (must be between 0 cm and 30 cm)!");
  }
  // or neighbours that form a cross, user defined coverage of up to 100% in neighbouring ring:
  else if (neighbourDef == "F") {
    double parChecked = par;
    if (parChecked > 1.0) parChecked = 1.0;
    else if (parChecked < 0.1) parChecked = 0.1;
    B2DEBUG(150, "ECLNeighbours::ECLNeighbours: initialize " << neighbourDef << ", fraction: " << parChecked);
    initializeF(parChecked);
  }
  // or wrong type:
  else {
    B2FATAL("ECLNeighbours::ECLNeighbours (constructor via std::string): Invalid option: " << neighbourDef <<
            " (valid: N(n), NC(n), NLegacy(n), NCLegacy(n), R ( with R<30 cm), F (with 0.1<f<1)");
  }

}

ECLNeighbours::~ECLNeighbours()
{
  ;
}

void ECLNeighbours::initializeR(double radius)
{
  // resize the vector
  std::vector<short int> fakeneighbours;
  fakeneighbours.push_back(0); // insert one fake to avoid the "0" entry
  m_neighbourMapTemp.push_back(fakeneighbours);

  // distance calculations take a "long" time, so reduce the number of candidates first:
  initializeN(6);

  // ECL geometry
  ECLGeometryPar* geom = ECLGeometryPar::Instance();

  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {
    // get the central one
    ROOT::Math::XYZVector direction = geom->GetCrystalVec(i);
    ROOT::Math::XYZVector position  = geom->GetCrystalPos(i);

    // get all nearby crystals
    std::vector<short int> neighbours = getNeighbours(i + 1);
    std::vector<short int> neighboursTemp;

    // ... and calculate the shortest distance between the central one and all possible neighbours (of the reduced set)
    for (auto const& id : neighbours) {
      const ROOT::Math::XYZVector& directionNeighbour = geom->GetCrystalVec(id - 1);
      const double alpha = ROOT::Math::VectorUtil::Angle(
                             direction, directionNeighbour);
      const double R        = position.R();
      const double distance = getDistance(alpha, R);

      if (distance <= radius) neighboursTemp.push_back(id);
    }

    m_neighbourMapTemp.push_back(neighboursTemp);
  }

  // all done, reoplace the original map
  m_neighbourMap = m_neighbourMapTemp;

}

void ECLNeighbours::initializeF(double frac)
{
  // ECL geometry
  ECLGeometryPar* geom = ECLGeometryPar::Instance();

  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {
    // this vector will hold all neighbours for the i-th crystal
    std::vector<short int> neighbours;

    // phi and theta coordinates of the central crystal
    geom->Mapping(i);
    const short tid = geom->GetThetaID();
    const short pid = geom->GetPhiID();

    // add the two in the same theta ring
    const short int phiInc = increasePhiId(pid, tid, 1);
    const short int phiDec = decreasePhiId(pid, tid, 1);
    neighbours.push_back(geom->GetCellID(tid, pid) + 1);
    neighbours.push_back(geom->GetCellID(tid, phiInc) + 1);
    neighbours.push_back(geom->GetCellID(tid, phiDec) + 1);

    double fracPos = (pid + 0.5) / m_crystalsPerRing[tid];

    // find the two closest crystals in the inner theta ring
    short int tidinner = tid - 1;
    if (tidinner >= 0) {

      short int n1 = -1;
      double dist1 = 999.;
      short int pid1 = -1;
      short int n2 = -1;
      double dist2 = 999.;

      for (short int inner = 0; inner < m_crystalsPerRing[tidinner]; ++inner) {
        const double f = (inner + 0.5) / m_crystalsPerRing[tidinner];
        const double dist = fabs(fracPos - f);
        if (dist < dist1) {
          dist2 = dist1;
          dist1 = dist;
          n2 = n1;
          n1 = geom->GetCellID(tidinner, inner);
          pid1 = inner;
        } else if (dist < dist2) {
          dist2 = dist;
          n2 = geom->GetCellID(tidinner, inner);
        }
      }

      // check coverage
      double cov = TMath::Min(((double) pid + 1) / m_crystalsPerRing[tid],
                              ((double) pid1 + 1) / m_crystalsPerRing[tidinner]) - TMath::Max(((double) pid) / m_crystalsPerRing[tid],
                                  ((double) pid1) / m_crystalsPerRing[tidinner]);
      cov = cov / (1. / m_crystalsPerRing[tid]);

      neighbours.push_back(n1 + 1);
      if (cov < frac - 1e-4) neighbours.push_back(n2 + 1);
    }

    // find the two closest crystals in the outer theta ring
    short int tidouter = tid + 1;
    if (tidouter <= 68) {
      short int no1 = -1;
      double disto1 = 999.;
      short int pido1 = -1;
      short int no2 = -1;
      double disto2 = 999.;

      for (short int outer = 0; outer < m_crystalsPerRing[tidouter]; ++outer) {
        const double f = (outer + 0.5) / m_crystalsPerRing[tidouter];
        const double dist = fabs(fracPos - f);
        if (dist < disto1) {
          disto2 = disto1;
          disto1 = dist;
          no2 = no1;
          no1 = geom->GetCellID(tidouter, outer);
          pido1 = outer;
        } else if (dist < disto2) {
          disto2 = dist;
          no2 = geom->GetCellID(tidouter, outer);
        }
      }
      // check coverage
      double cov = TMath::Min(((double) pid + 1) / m_crystalsPerRing[tid],
                              ((double) pido1 + 1) / m_crystalsPerRing[tidouter]) - TMath::Max(((double) pid) / m_crystalsPerRing[tid],
                                  ((double) pido1) / m_crystalsPerRing[tidouter]);
      cov = cov / (1. / m_crystalsPerRing[tid]);

      neighbours.push_back(no1 + 1);
      if (cov < frac - 1e-4) {
        neighbours.push_back(no2 + 1);
      }
    }

    // push back the final vector of IDs
    m_neighbourMap.push_back(neighbours);
  }

}

void ECLNeighbours::initializeN(const int n, const bool sorted)
{
  // This is the "NxN-edges" case (in the barrel)
  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {

    // this vector will hold all neighbours for the i-th crystal
    std::vector<short int> neighbours;
    neighbours.push_back(i + 1);

    std::vector<short int> neighbours_temp;

    // iterate next, next-to-next, next-to-next-to-next, ...
    for (int idx = 0; idx < n; idx++) {
      EclNbr tmpNbr;
      for (const auto& nbr : neighbours) {
        const EclNbr& aNbr(tmpNbr.getNbr(nbr - 1)); // uses crystalid, but we store cellids
        for (std::vector<EclNbr::Identifier>::const_iterator newnbr = aNbr.nearBegin(); newnbr != aNbr.nearEnd(); ++newnbr) {
          neighbours_temp.push_back(((short) *newnbr) + 1); // store cellid, not crystalid
        }
      }

      // now sort and remove all duplicates from neighbours_temp
      sort(neighbours_temp.begin(), neighbours_temp.end());
      neighbours_temp.erase(unique(neighbours_temp.begin(), neighbours_temp.end()), neighbours_temp.end());
      neighbours.insert(neighbours.end(), neighbours_temp.begin(), neighbours_temp.end());
    }

    // push back the final vector of IDs, we have to erease the duplicate first ID
    sort(neighbours.begin(), neighbours.end());
    neighbours.erase(unique(neighbours.begin(), neighbours.end()), neighbours.end());

    //sort by theta and phi
    if (sorted == true) {

      // ECL geometry
      ECLGeometryPar* geom = ECLGeometryPar::Instance();

      // create a simple struct with cellid, thetaid, and phiid (the latter two will be used for sorting)
      struct crystal {
        int cellid;
        int phiid;
        int thetaid;
        int neighbourn; //needed since we can not access local variables in sort
      };

      // fill them all into a vector
      std::vector<crystal> crystals;
      for (const auto& nbr : neighbours) {
        geom->Mapping(nbr - 1);
        crystals.push_back({nbr, geom->GetPhiID(), geom->GetThetaID(), n});
      }

      //sort this vector using custom metric
      std::sort(crystals.begin(), crystals.end(), [](const auto & left, const auto & right) {
        //primary condition: thetaid
        if (left.thetaid < right.thetaid) return true;
        if (left.thetaid > right.thetaid) return false;

        // left.thetaid == right.thetaid for primary condition, go to secondary condition
        // first check if we are crossing a phi=0 boundary by checking if the  difference between phiids is larger than the neighbour size (2*N+1)
        // examples: left.phiid = 0, right.phiid=143 -> returns true (0 ">" 143)
        // examples: left.phiid = 0, right.phiid=1 -> returns false (1 ">" 0)
        // examples: left.phiid = 1, right.phiid=0 -> returns true (1 ">" 0)
        if (fabs(left.phiid - right.phiid) > (2 * left.neighbourn + 1)) {
          return right.phiid > left.phiid;
        } else {
          return left.phiid > right.phiid;
        }

        //we should never arrive here by definition
        return true;
      });

      //replace the neighbour vector with this newly sorted one
      for (int nbidx = 0; nbidx < int(neighbours.size()); nbidx++) {
        neighbours[nbidx] = crystals[nbidx].cellid;
      }
    }

    m_neighbourMap.push_back(neighbours);

  }
}

void ECLNeighbours::initializeNC(const int n)
{
  // get the normal neighbours
  initializeN(n);

  // ECL geometry
  ECLGeometryPar* geom = ECLGeometryPar::Instance();

  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {
    std::vector<short int> neighbours = m_neighbourMap.at(i + 1);
    std::vector<short int> neighbours_temp;

    geom->Mapping(i);
    const int centerthetaid = geom->GetThetaID();

    for (const auto& nbr : neighbours) {
      geom->Mapping(nbr - 1);
      const int thetaid = geom->GetThetaID();

      if (abs(thetaid - centerthetaid) == n) {
        const short int phiInc = increasePhiId(geom->GetPhiID(), geom->GetThetaID(), 1);
        const short int phiDec = decreasePhiId(geom->GetPhiID(), geom->GetThetaID(), 1);
        const int cid1 = geom->GetCellID(thetaid, phiInc) + 1;
        const int cid2 = geom->GetCellID(thetaid, phiDec) + 1;

        // if that crystal has two neighbours in the same theta-ring, it will not be removed
        if (!((std::find(neighbours.begin(), neighbours.end(), cid1) != neighbours.end()) and
              (std::find(neighbours.begin(), neighbours.end(), cid2) != neighbours.end()))) {
          continue;
        }
      }
      neighbours_temp.push_back(nbr);

    }

    m_neighbourMap[i + 1] = neighbours_temp;

  } // end 8736 loop

}

void ECLNeighbours::initializeNLegacy(int n)
{
  // ECL geometry
  ECLGeometryPar* geom = ECLGeometryPar::Instance();

  // This is the "NxN-edges" case (in the barrel)
  // in the endcaps we project the neighbours to the outer and inner rings.
  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {

    // this vector will hold all neighbours for the i-th crystal
    std::vector<short int> neighbours;

    // phi and theta coordinates of the central crystal
    geom->Mapping(i);
    const short tid = geom->GetThetaID();
    const short pid = geom->GetPhiID();

    // 'left' and 'right' extremal neighbours in the same theta ring
    const short int phiInc = increasePhiId(pid, tid, n);
    const short int phiDec = decreasePhiId(pid, tid, n);
    const double fractionalPhiInc = static_cast < double >(phiInc) / m_crystalsPerRing [ tid ];
    const double fractionalPhiDec = static_cast < double >(phiDec) / m_crystalsPerRing [ tid ];

    // loop over all possible theta rings and add neighbours
    for (int theta = tid - n; theta <= tid + n; theta++) {
      if ((theta > 68) || (theta < 0)) continue;     // valid theta ids are 0..68 (69 in total)

      short int thisPhiInc = std::lround(fractionalPhiInc * m_crystalsPerRing [ theta ]);

      short int thisPhiDec = std::lround(fractionalPhiDec * m_crystalsPerRing [ theta ]);
      if (thisPhiDec == m_crystalsPerRing [ theta ]) thisPhiDec = 0;

      const std::vector<short int> phiList = getPhiIdsInBetween(thisPhiInc, thisPhiDec, theta);

      for (unsigned int k = 0; k < phiList.size(); ++k) {
        neighbours.push_back(geom->GetCellID(theta, phiList.at(k)) + 1);
      }
    }

    // push back the final vector of IDs
    m_neighbourMap.push_back(neighbours);

  }
}


void ECLNeighbours::initializeNCLegacy(const int n, const int corners)
{
  // ECL geometry
  ECLGeometryPar* geom = ECLGeometryPar::Instance();

  // This is the "NxN-edges" minus edges case (in the barrel)
  // in the endcaps we project the neighbours to the outer and inner rings.
  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {

    // this vector will hold all neighbours for the i-th crystal
    std::vector<short int> neighbours;

    // phi and theta coordinates of the central crystal
    geom->Mapping(i);
    const short tid = geom->GetThetaID();
    const short pid = geom->GetPhiID();

    // 'left' and 'right' extremal neighbours in the same theta ring
    const short int phiInc = increasePhiId(pid, tid, n);
    const short int phiDec = decreasePhiId(pid, tid, n);
    const double fractionalPhiInc = static_cast < double >(phiInc) / m_crystalsPerRing [ tid ];
    const double fractionalPhiDec = static_cast < double >(phiDec) / m_crystalsPerRing [ tid ];

    // loop over all possible theta rings and add neighbours
    for (int theta = tid - n; theta <= tid + n; theta++) {
      if ((theta > 68) || (theta < 0)) continue;     // valid theta ids are 0..68 (69 in total)

      short int thisPhiInc = std::lround(fractionalPhiInc * m_crystalsPerRing [ theta ]);

      short int thisPhiDec = std::lround(fractionalPhiDec * m_crystalsPerRing [ theta ]);
      if (thisPhiDec == m_crystalsPerRing [ theta ]) thisPhiDec = 0;

      std::vector<short int> phiList;
      if ((theta == tid - n) or (theta == tid + n)) phiList = getPhiIdsInBetweenC(thisPhiInc, thisPhiDec, theta, corners);
      else if (corners == 2 and ((theta == tid - n + 1)
                                 or (theta == tid + n - 1))) phiList = getPhiIdsInBetweenC(thisPhiInc, thisPhiDec, theta, 1);
      else phiList = getPhiIdsInBetween(thisPhiInc, thisPhiDec, theta);

      for (unsigned int k = 0; k < phiList.size(); ++k) {
        neighbours.push_back(geom->GetCellID(theta, phiList.at(k)) + 1);
      }
    }

    // push back the final vector of IDs
    m_neighbourMap.push_back(neighbours);

  }
}

const std::vector<short int>& ECLNeighbours::getNeighbours(const short int cid) const
{
  return m_neighbourMap.at(cid);
}

// decrease the phi id by "n" integers numbers (valid ids range from 0 to m_crystalsPerRing[thetaid] - 1)
short int ECLNeighbours::decreasePhiId(const short int phiid, const short int thetaid, const short int n)
{
  short int previousPhiId = ((phiid - n < 0) ? m_crystalsPerRing[thetaid] + phiid - n : phiid - n);     // "underflow"
  return previousPhiId;
}

// increase the phi id by "n" integers numbers (valid ids range from 0 to m_crystalsPerRing[thetaid] - 1)
short int ECLNeighbours::increasePhiId(const short int phiid, const short int thetaid, const short int n)
{
  short int nextPhiId = (((phiid + n) > (m_crystalsPerRing[thetaid] - 1)) ? phiid + n - m_crystalsPerRing[thetaid] : phiid +
                         n);       // "overflow"
  return nextPhiId;
}

std::vector<short int> ECLNeighbours::getPhiIdsInBetween(const short int phi0, const short int phi1, const short int theta)
{
  std::vector<short int> phiList;
  short int loop = phi0;

  while (1) {
    phiList.push_back(loop);
    if (loop == phi1) break;
    loop = decreasePhiId(loop, theta, 1);
  }

  return phiList;
}

std::vector<short int> ECLNeighbours::getPhiIdsInBetweenC(const short int phi0, const short int phi1, const short int theta,
                                                          const int corners)
{
  std::vector<short int> phiList;
  if (phi0 == phi1) return phiList; // could be that there is only one crystal in this theta ring definition

  short int loop = decreasePhiId(phi0, theta, corners); // start at -n corners
  if (loop == phi1) return phiList; // there will be no neighbours left in this phi ring after removing the first corners

  short int stop = increasePhiId(phi1, theta, corners); //stop at +n corners

  while (1) {
    phiList.push_back(loop);
    if (loop == stop) break;
    loop = decreasePhiId(loop, theta, 1);
  }

  return phiList;
}

double ECLNeighbours::getDistance(const double alpha, const double R)
{
  return 2.0 * R * TMath::Sin(alpha / 2.0);
}

