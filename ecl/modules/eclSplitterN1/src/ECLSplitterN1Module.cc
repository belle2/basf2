/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclSplitterN1/ECLSplitterN1Module.h>

/* ECL header. */
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dataobjects/ECLLocalMaximum.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dbobjects/ECLnOptimal.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/utility/Position.h>

/* Basf2 headers. */
#include <framework/geometry/B2Vector3.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/EventLevelClusteringInfo.h>

/* C++ headers. */
#include <algorithm>
#include <string>
#include <utility>

// NAMESPACES
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module(s)
//-----------------------------------------------------------------
REG_MODULE(ECLSplitterN1);
REG_MODULE(ECLSplitterN1PureCsI);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLSplitterN1Module::ECLSplitterN1Module() : Module(),
  m_eclCalDigits(eclCalDigitArrayName()),
  m_eclConnectedRegions(eclConnectedRegionArrayName()),
  m_eclShowers(eclShowerArrayName()),
  m_eclLocalMaximums(eclLocalMaximumArrayName()),
  m_eventLevelClusteringInfo(eventLevelClusteringInfoName())
{
  // Set description.
  setDescription("ECLSplitterN1Module: Baseline reconstruction splitter code for the n photon hypothesis.");
  addParam("fullBkgdCount", m_fullBkgdCount,
           "Number of background digits at full background (as provided by EventLevelClusteringInfo).",
           182);

  // Set module parameters.

  // Splitter.
  addParam("threshold", m_threshold, "Threshold energy after splitting.", 7.5 * Belle2::Unit::MeV);
  addParam("expConstant", m_expConstant, "Constant a from exp(-a*dist/RM), typical: 1.5 to 3.5?", 2.5); // to be optimized!
  addParam("maxIterations", m_maxIterations, "Maximum number of iterations for centroid shifts.", 100);
  addParam("shiftTolerance", m_shiftTolerance, "Tolerance level for centroid shifts.", 1.0 * Belle2::Unit::mm);
  addParam("minimumSharedEnergy", m_minimumSharedEnergy, "Minimum shared energy.", 25.0 * Belle2::Unit::keV);
  addParam("maxSplits", m_maxSplits, "Maximum number of splits within one connected region.", 10);
  addParam("cutDigitEnergyForEnergy", m_cutDigitEnergyForEnergy,
           "Minimum digit energy to be included in the shower energy calculation. (NOT USED)", 0.5 * Belle2::Unit::MeV);
  addParam("cutDigitTimeResidualForEnergy", m_cutDigitTimeResidualForEnergy,
           "Maximum time residual to be included in the shower energy calculation. (NOT USED)", 5.0);

  // Neighbour definitions
  addParam("useOptimalNumberOfDigitsForEnergy", m_useOptimalNumberOfDigitsForEnergy,
           "Optimize the number of digits for energy calculations.", 1);

  // Position.
  addParam("positionMethod", m_positionMethod, "Position determination method.", std::string("lilo"));
  addParam("liloParameterA", m_liloParameterA, "Position determination linear-log. parameter A.", 4.0);
  addParam("liloParameterB", m_liloParameterB, "Position determination linear-log. parameter B.", 0.0);
  addParam("liloParameterC", m_liloParameterC, "Position determination linear-log. parameter C.", 0.0);

  // Set parallel processing flag.
  setPropertyFlags(c_ParallelProcessingCertified);
}

ECLSplitterN1Module::~ECLSplitterN1Module()
{
  // do not delete objects here, do it in terminate()!
}

void ECLSplitterN1Module::initialize()
{

  // Geometry instance.
  m_geom = ECLGeometryPar::Instance();

  // Check and format user input.
  m_liloParameters.resize(3);
  m_liloParameters.at(0) = m_liloParameterA;
  m_liloParameters.at(1) = m_liloParameterB;
  m_liloParameters.at(2) = m_liloParameterC;

  // ECL dataobjects.
  m_eclCalDigits.registerInDataStore(eclCalDigitArrayName());
  m_eclConnectedRegions.registerInDataStore(eclConnectedRegionArrayName());
  m_eclShowers.registerInDataStore(eclShowerArrayName());
  m_eclLocalMaximums.registerInDataStore(eclLocalMaximumArrayName());

  // mDST dataobjects.
  m_eventLevelClusteringInfo.isRequired(eventLevelClusteringInfoName());

  // Register relations (we probably dont need all, but keep them for now for debugging).
  m_eclShowers.registerRelationTo(m_eclConnectedRegions);
  m_eclShowers.registerRelationTo(m_eclCalDigits);
  m_eclShowers.registerRelationTo(m_eclLocalMaximums);
  m_eclLocalMaximums.registerRelationTo(m_eclCalDigits);

  // Initialize neighbour maps (we will optimize the endcaps later, there is more than just a certain energy containment to be considered)
  m_NeighbourMap9 = new ECLNeighbours("N", 1); // N: 3x3 = 9
  m_NeighbourMap21 = new ECLNeighbours("NC", 2); // NC: 5x5 excluding corners = 21

  // initialize the vector that gives the relation between cellid and store array position
  m_StoreArrPosition.resize(ECLElementNumbers::c_NCrystals + 1);
  m_StoreArrPositionLM.resize(ECLElementNumbers::c_NCrystals + 1);

}

void ECLSplitterN1Module::beginRun()
{
  //..Read in nOptimal crystal payload from database
  if (m_eclNOptimal.hasChanged()) {

    //..Vectors of energy boundaries for each region
    std::vector<float> eBoundariesFwd = m_eclNOptimal->getUpperBoundariesFwd();
    std::vector<float> eBoundariesBrl = m_eclNOptimal->getUpperBoundariesBrl();
    std::vector<float> eBoundariesBwd = m_eclNOptimal->getUpperBoundariesBwd();

    //..Adjust the size of the vector of boundaries to match the number in the payload
    m_nEnergyBins = eBoundariesBrl.size();
    B2INFO("ECLSplitterN1 beginRun: number of nOptimal payload energies = " << m_nEnergyBins);
    m_eBoundaries.resize(m_nLeakReg, std::vector<float>(m_nEnergyBins, 0.));

    //..Copy values to m_eBoundaries
    for (int ie = 0; ie < m_nEnergyBins; ie++) {
      m_eBoundaries[0][ie] = eBoundariesFwd[ie];
      m_eBoundaries[1][ie] = eBoundariesBrl[ie];
      m_eBoundaries[2][ie] = eBoundariesBwd[ie];
      B2INFO(" upper boundaries for energy point " << ie << " " << m_eBoundaries[0][ie] << " " << m_eBoundaries[1][ie] << " " <<
             m_eBoundaries[2][ie]);
    }

    //..Group number of each crystal
    m_groupNumber = m_eclNOptimal->getGroupNumber();

    //..2D histogram of nOptimal for each group and energy point
    m_nOptimal2D = m_eclNOptimal->getNOptimal();
  }
}

void ECLSplitterN1Module::event()
{
  B2DEBUG(175, "ECLCRSplitterModule::event()");

  // Fill a vector that can be used to map cellid -> store array position for eclCalDigits.
  memset(&m_StoreArrPosition[0], -1, m_StoreArrPosition.size() * sizeof m_StoreArrPosition[0]);
  for (int i = 0; i < m_eclCalDigits.getEntries(); i++) {
    m_StoreArrPosition[m_eclCalDigits[i]->getCellId()] = i;
  }

  // Fill a vector that can be used to map cellid -> store array position for eclLocalMaximums.
  memset(&m_StoreArrPositionLM[0], -1, m_StoreArrPositionLM.size() * sizeof m_StoreArrPositionLM[0]);
  for (int i = 0; i < m_eclLocalMaximums.getEntries(); i++) {
    m_StoreArrPositionLM[m_eclLocalMaximums[i]->getCellId()] = i;
  }

  // Loop over all connected regions
  for (auto& aCR : m_eclConnectedRegions) {
    // list theat will hold all cellids in this connected region
    m_cellIdInCR.clear();

    const unsigned int entries = (aCR.getRelationsWith<ECLCalDigit>(eclCalDigitArrayName())).size();

    m_cellIdInCR.resize(entries);

    // Fill all calDigits ids in this CR into a vector to make them 'find'-able.
    int i = 0;
    for (const auto& caldigit : aCR.getRelationsWith<ECLCalDigit>(eclCalDigitArrayName())) {
      m_cellIdInCR[i] = caldigit.getCellId();
      ++i;
    }

    // Split and reconstruct the showers in this connected regions.
    splitConnectedRegion(aCR);

  } // end auto& aCR

}


void ECLSplitterN1Module::endRun()
{
//  if (m_tg2OptimalNumberOfDigitsForEnergy) delete m_tg2OptimalNumberOfDigitsForEnergy;
}


void ECLSplitterN1Module::terminate()
{
  if (m_NeighbourMap9) delete m_NeighbourMap9;
  if (m_NeighbourMap21) delete m_NeighbourMap21;
}

void ECLSplitterN1Module::splitConnectedRegion(ECLConnectedRegion& aCR)
{

  // Get the event background level
  const int bkgdcount = m_eventLevelClusteringInfo->getNECLCalDigitsOutOfTime();
  double backgroundLevel = 0.0; // from out of time digit counting
  if (m_fullBkgdCount > 0) {
    backgroundLevel = static_cast<double>(bkgdcount) / static_cast<double>(m_fullBkgdCount);
  }

  // Get the number of LMs in this CR
  const int nLocalMaximums = aCR.getRelationsWith<ECLLocalMaximum>(eclLocalMaximumArrayName()).size();

  B2DEBUG(170, "ECLCRSplitterModule::splitConnectedRegion: nLocalMaximums = " << nLocalMaximums);

  // Three cases:
  // 1) There is no local maximum (can only happen if the CR seed is not a LM itself).
  // 2) There is exactly one local maximum.
  // 3) There are more than one, typically two or three, local maxima and we have to share energy between them.
  //    If there are more than m_maxSplits local maxima, the m_maxSplits highest energy local maxima will be used.

  // ---------------------------------------------------------------------
  if (nLocalMaximums == 1) {

    // Create a shower.
    const auto aECLShower = m_eclShowers.appendNew();

    // Add relation to the CR.
    aECLShower->addRelationTo(&aCR);

    // Find the highest energetic crystal in this CR or use the LM.
    double weightSum = 0.0;

    // Add relation to the LM.
    RelationVector<ECLLocalMaximum> locmaxvector = aCR.getRelationsWith<ECLLocalMaximum>(eclLocalMaximumArrayName());
    aECLShower->addRelationTo(locmaxvector[0]);

    const int locmaxcellid = locmaxvector[0]->getCellId();
    const int pos = m_StoreArrPosition[locmaxcellid];
    double highestEnergyID             = (m_eclCalDigits[pos])->getCellId();
    double highestEnergy               = (m_eclCalDigits[pos])->getEnergy();
    double highestEnergyTime           = (m_eclCalDigits[pos])->getTime();
    double highestEnergyTimeResolution = (m_eclCalDigits[pos])->getTimeResolution();

    // Get a first estimation of the energy using 3x3 neighbours.
    const double energyEstimation = estimateEnergy(highestEnergyID);

    // Check if 21 would be better in the present background conditions:
    ECLNeighbours* neighbourMap; // FIXME pointer needed?
    int nNeighbours = getNeighbourMap(energyEstimation, backgroundLevel);
    if (nNeighbours == 9 and !m_useOptimalNumberOfDigitsForEnergy) neighbourMap = m_NeighbourMap9;
    else neighbourMap = m_NeighbourMap21;

    // Add neighbours and weights for the shower.
    std::vector<ECLCalDigit> digits;
    std::vector<double> weights;
    for (auto& neighbourId : neighbourMap->getNeighbours(highestEnergyID)) {
      const auto it = std::find(m_cellIdInCR.begin(), m_cellIdInCR.end(),
                                neighbourId); // check if the neighbour is in the list for this CR
      if (it == m_cellIdInCR.end()) continue; // not in this CR

      const int neighbourpos = m_StoreArrPosition[neighbourId];
      digits.push_back(*m_eclCalDigits[neighbourpos]); // list of digits for position reconstruction
      weights.push_back(1.0); // list of weights (all 1 in this case for now)
      weightSum += 1.0;

      aECLShower->addRelationTo(m_eclCalDigits[neighbourpos], 1.0); // add digits to this shower, weight = 1
    }

    // Get position.
    const B2Vector3D& showerposition = Belle2::ECL::computePositionLiLo(digits, weights, m_liloParameters);
    aECLShower->setTheta(showerposition.Theta());
    aECLShower->setPhi(showerposition.Phi());
    aECLShower->setR(showerposition.Mag());

    // Get Energy, if requested, set some weights to zero for energy calculation.
    double showerEnergy = 0.0;
    if (m_useOptimalNumberOfDigitsForEnergy) {

      // Get the optimal number of neighbours for this crystal and energy
      std::vector<int> nOptimalVec = getOptimalNumberOfDigits(highestEnergyID, energyEstimation);
      const unsigned int nOptimal = static_cast<unsigned int>(nOptimalVec[0]);
      aECLShower->setNominalNumberOfCrystalsForEnergy(static_cast<double>(nOptimal));

      // Store the indices used; will be needed later for energy corrections.
      // Also store energy, used along with cellID to find leakage corrections.
      aECLShower->setNOptimalGroupIndex(nOptimalVec[1]);
      aECLShower->setNOptimalEnergyBin(nOptimalVec[2]);
      aECLShower->setNOptimalEnergy(energyEstimation);

      // Get the list of crystals used for the energy calculation
      std::vector< std::pair<unsigned int, double>> listCrystalPairs; // cell id and weighted reconstructed energy
      listCrystalPairs.resize(digits.size()); //resize to number of all crystals in cluster

      std::vector < std::pair<double, double> > weighteddigits;
      weighteddigits.resize(digits.size());
      for (unsigned int i = 0; i < digits.size(); ++i) {
        weighteddigits.at(i) = std::make_pair((digits.at(i)).getEnergy(), weights.at(i));
        listCrystalPairs.at(i) = std::make_pair((digits.at(i)).getCellId(), weights.at(i) * (digits.at(i)).getEnergy());
      }

      // sort the listCrystals and keep the n highest in descending order
      std::sort(listCrystalPairs.begin(), listCrystalPairs.end(), [](const auto & left, const auto & right) {
        return left.second > right.second;
      });
      std::vector< unsigned int> listCrystals; //cell id

      for (unsigned int i = 0; i < digits.size(); ++i) {
        if (i < nOptimal) {
          listCrystals.push_back(listCrystalPairs[i].first);
        }
      }

      aECLShower->setNumberOfCrystalsForEnergy(static_cast<double>(listCrystals.size()));
      aECLShower->setListOfCrystalsForEnergy(listCrystals);

      showerEnergy = getEnergySum(weighteddigits, nOptimal);
      B2DEBUG(175, "Shower Energy (1): " << showerEnergy);

    } else {
      showerEnergy = Belle2::ECL::computeEnergySum(digits, weights);
    }

    aECLShower->setEnergy(showerEnergy);
    aECLShower->setEnergyRaw(showerEnergy);
    aECLShower->setEnergyHighestCrystal(highestEnergy);
    aECLShower->setTime(highestEnergyTime);
    aECLShower->setDeltaTime99(highestEnergyTimeResolution);
    aECLShower->setNumberOfCrystals(weightSum);
    aECLShower->setCentralCellId(highestEnergyID);

    B2DEBUG(175, "theta           = " << showerposition.Theta());
    B2DEBUG(175, "phi             = " << showerposition.Phi());
    B2DEBUG(175, "R               = " << showerposition.Mag());
    B2DEBUG(175, "energy          = " << showerEnergy);
    B2DEBUG(175, "time            = " << highestEnergyTime);
    B2DEBUG(175, "time resolution = " << highestEnergyTimeResolution);
    B2DEBUG(175, "neighbours      = " << nNeighbours);
    B2DEBUG(175, "backgroundLevel = " << backgroundLevel);

    // Fill shower Ids
    aECLShower->setShowerId(1); // always one (only this single shower in the CR)
    aECLShower->setHypothesisId(Belle2::ECLShower::c_nPhotons);
    aECLShower->setConnectedRegionId(aCR.getCRId());

    // Add relations of all CalDigits of the CR to the local maximum (here: all weights = 1).
    const int posLM = m_StoreArrPositionLM[locmaxcellid];
    for (const auto& aDigit : aCR.getRelationsWith<ECLCalDigit>()) {
      const int posDigit = m_StoreArrPosition[aDigit.getCellId()];
      m_eclLocalMaximums[posLM]->addRelationTo(m_eclCalDigits[posDigit], 1.0);
    }

  } // end case with one LM
  else { // More than one LM, energy must be split. This algorithm is inspired by BaBar code.

    // check if we have too many local maximums. if yes: limit to user set maximum
    // create a vector with all local maximums and its crystal energies
    std::vector<std::pair<ECLLocalMaximum, double>> lm_energy_vector;
    for (auto& aLocalMaximum :  aCR.getRelationsWith<ECLLocalMaximum>(eclLocalMaximumArrayName())) {
      const int cellid = aLocalMaximum.getCellId();
      const int pos = m_StoreArrPosition[cellid];
      const double digitenergy = m_eclCalDigits[pos]->getEnergy();
      lm_energy_vector.push_back(std::pair<ECLLocalMaximum, double>(aLocalMaximum, digitenergy));
    };

    // sort this vector in descending order and keep only up to m_maxSplits entries
    if (lm_energy_vector.size() >= static_cast<size_t>(m_maxSplits)) {
      std::sort(lm_energy_vector.begin(), lm_energy_vector.end(), [](const std::pair<ECLLocalMaximum, double>& x,
      const std::pair<ECLLocalMaximum, double>& y) {
        return x.second > y.second;
      });

      lm_energy_vector.resize(m_maxSplits);
    }

    std::vector<ECLCalDigit> digits;
    std::vector<double> weights;
    std::map<int, B2Vector3D> centroidList; // key = cellid, value = centroid position
    std::map<int, double> centroidEnergyList; // key = cellid, value = centroid position
    std::map<int, B2Vector3D> allPoints; // key = cellid, value = digit position
    std::map<int, std::vector < double > > weightMap; // key = locmaxid, value = vector of weights
    std::vector < ECLCalDigit > digitVector; // the order of weights in weightMap must be the same

    // Fill the maxima positions in a map
    std::map<int, B2Vector3D> localMaximumsPoints; // key = locmaxid, value = maximum position
    std::map<int, B2Vector3D> centroidPoints; // key = locmaxid (as index), value = centroid position

    for (auto& aLocalMaximum : lm_energy_vector) {

      int cellid = aLocalMaximum.first.getCellId();

      // Get the position of this crystal and fill it in two maps.
      B2Vector3D vectorPosition = m_geom->GetCrystalPos(cellid - 1);
      localMaximumsPoints.insert(std::map<int, B2Vector3D>::value_type(cellid, vectorPosition));
      centroidPoints.insert(std::map<int, B2Vector3D>::value_type(cellid, vectorPosition));
    }

    // The following will be done iteratively. Empty clusters after splitting will be removed, and the procedure will be repeated.
    bool iterateclusters = true;
    do {
      digits.clear();
      weights.clear();
      centroidList.clear();
      centroidEnergyList.clear();
      allPoints.clear();
      weightMap.clear();
      digitVector.clear();

      // Fill all digits from this CR in a map
      for (auto& aCalDigit : aCR.getRelationsWith<ECLCalDigit>(eclCalDigitArrayName())) {
        const int cellid = aCalDigit.getCellId();
        // get the position of this crystal and fill them in a map
        B2Vector3D vectorPosition = m_geom->GetCrystalPos(cellid - 1);
        allPoints.insert(std::map<int, B2Vector3D>::value_type(cellid, vectorPosition));
        digits.push_back(aCalDigit);
      }

      for (const auto& digitpoint : allPoints) {
        const int cellid = digitpoint.first;
        const int pos = m_StoreArrPosition[cellid];
        digitVector.push_back(*m_eclCalDigits[pos]);
      }


      // -----------------------------------------------------------------------------------------
      // The 'heart' of the splitter
      // Start with each local maximum and set it to the first centroid position. Then iterate over all ECLCalDigits
      // in this CR and calculate the weighted distances to the local maximum
      int nIterations = 0;
      double centroidShiftAverage = 0.0;
      //    double lastcentroidShiftAverage = 0.0;

      do {
        B2DEBUG(175, "Iteration: #" << nIterations << " (of max. " << m_maxIterations << ")");

        centroidShiftAverage = 0.0;
        if (nIterations == 0) {
          centroidList.clear();
          centroidEnergyList.clear();
          weightMap.clear();
        }

        // Loop over all local maximums points, each one will become a shower!
        for (const auto& locmaxpoint : localMaximumsPoints) {

          // cell id of this local maximum
          const int locmaxcellid = locmaxpoint.first;

          // clear weights vector
          weights.clear();

          // if this is the first iteration the shower energy is not know, take the local maximum energy * 1.5.
          if (nIterations == 0) {
            const int pos = m_StoreArrPosition[locmaxcellid];
            const double locmaxenergy = m_eclCalDigits[pos]->getEnergy();
            centroidEnergyList[locmaxcellid] = 1.5 * locmaxenergy;
          }

          B2DEBUG(175, "local maximum cellid: " << locmaxcellid);

          //-------------------------------------------------------------------
          // Loop over all digits. They get a weight using the distance to the respective centroid.
          for (const auto& digitpoint : allPoints) {

            // cellid and position of this digit
            const int digitcellid = digitpoint.first;
            B2Vector3D digitpos = digitpoint.second;

            const int pos = m_StoreArrPosition[digitcellid];
            const double digitenergy = m_eclCalDigits[pos]->getEnergy();

            double weight            = 0.0;
            double energy            = 0.0;
            double distance          = 0.0;
            double distanceEnergySum = 0.0;

            // Loop over all centroids
            for (const auto& centroidpoint : centroidPoints) {

              // cell id and position of this centroid
              const int centroidcellid = centroidpoint.first;
              B2Vector3D centroidpos = centroidpoint.second;

              double thisdistance = 0.;

              // in the first iteration, this distance is really zero, avoid floating point problems
              if (nIterations == 0 and digitcellid == centroidcellid) {
                thisdistance = 0.0;
              } else {
                B2Vector3D vectorDistance = ((centroidpos) - (digitpos));
                thisdistance = vectorDistance.Mag();
              }

              // energy of the centroid aka locmax
              const int thispos = m_StoreArrPosition[centroidcellid];
              const double thisenergy = m_eclCalDigits[thispos]->getEnergy();

              // Not  the most efficienct way to get this information, but not worth the thinking yet:
              if (locmaxcellid == centroidcellid) {
                distance = thisdistance;
                energy = thisenergy;
              }

              // Get the product of distance and energy.
              const double expfactor = exp(-m_expConstant * thisdistance / c_molierRadius);
              distanceEnergySum += (thisenergy * expfactor);

            } // end centroidPoints

            // Calculate the weight for this digits for this local maximum.
            if (distanceEnergySum > 0.0) {
              const double expfactor = exp(-m_expConstant * distance / c_molierRadius);
              weight = energy * expfactor / distanceEnergySum;
            } else {
              weight = 0.0;
            }

            // Check if the weighted energy is above threshold
            if ((digitenergy * weight) < m_minimumSharedEnergy) {
              weight = 0.0;
            }

            // Check for rounding problems larger than unity
            if (weight > 1.0) {
              B2WARNING("ECLCRSplitterModule::splitConnectedRegion: Floating point glitch, weight for this digit " << weight <<
                        ", resetting it to 1.0.");
              weight = 1.0;
            }

            // Fill the weight for this digits and this local maximum.
            B2DEBUG(175, "   cellid: " << digitcellid << ", energy: " << digitenergy << ", weight: " << weight << ", distance: " << distance);
            weights.push_back(weight);

          } // end allPoints

          // Get the old centroid position.
          B2Vector3D oldCentroidPos = (centroidPoints.find(locmaxcellid))->second;

          // Calculate the new centroid position.
          B2Vector3D newCentroidPos = Belle2::ECL::computePositionLiLo(digits, weights, m_liloParameters);

          // Calculate new energy
          const double newEnergy = Belle2::ECL::computeEnergySum(digits, weights);

          // Calculate the shift of the centroid position for this local maximum.
          const B2Vector3D centroidShift = (oldCentroidPos - newCentroidPos);

          // Save the new centroid position (but dont update yet!), also save the weights and energy.
          centroidList[locmaxcellid] = newCentroidPos;
          weightMap[locmaxcellid] = weights;

          B2DEBUG(175, "--> inserting new energy: " << newEnergy << " for local maximum " << locmaxcellid);
          centroidEnergyList[locmaxcellid] = newEnergy;
          double showerenergy = (*centroidEnergyList.find(locmaxcellid)).second / Belle2::Unit::MeV;
          B2DEBUG(175, "--> new energy = " << showerenergy << " MeV");

          // Add this to the average centroid shift.
          centroidShiftAverage += centroidShift.Mag();

          // Debugging output
          B2DEBUG(175, "   old centroid: " << oldCentroidPos.X() << " cm, " <<  oldCentroidPos.Y() << " cm, " <<  oldCentroidPos.Z() <<
                  "cm");
          B2DEBUG(175, "   new centroid: " << newCentroidPos.X() << " cm, " <<  newCentroidPos.Y() << " cm, " <<  newCentroidPos.Z() <<
                  "cm");
          B2DEBUG(175, "   centroid shift: " << centroidShift.Mag() << " cm");

        } // end localMaximumsPoints

        // Get the average centroid shift.
        centroidShiftAverage /= static_cast<double>(nLocalMaximums);
        //      lastcentroidShiftAverage = centroidShiftAverage;
        B2DEBUG(175, "--> average centroid shift: " << centroidShiftAverage << " cm (tolerance is " << m_shiftTolerance << " cm)");

        // Update centroid positions for the next round
        for (const auto& locmaxpoint : localMaximumsPoints) {
          centroidPoints[locmaxpoint.first] = (centroidList.find(locmaxpoint.first))->second;
        }

        ++nIterations;

      } while (nIterations < m_maxIterations and centroidShiftAverage > m_shiftTolerance);
      // DONE!

      // check that local maxima are still local maxima
      std::vector<int> markfordeletion;
      iterateclusters = false;
      for (const auto& locmaxpoint : localMaximumsPoints) {

        // Get locmax cellid
        const int locmaxcellid = locmaxpoint.first;
        const int pos = m_StoreArrPosition[locmaxcellid];

        B2DEBUG(175, "locmaxcellid: " << locmaxcellid);
        const double lmenergy = m_eclCalDigits[pos]->getEnergy();
        B2DEBUG(175, "ok: ");

        // Get the weight vector.
        std::vector < double > myWeights = (*weightMap.find(locmaxcellid)).second;

        for (unsigned int i = 0; i < digitVector.size(); ++i) {

          const ECLCalDigit dig = digitVector[i];
          const double weight = myWeights[i];
          const int cellid = dig.getCellId();
          const double energy = dig.getEnergy();

          // two ways to fail:
          // 1) another cell has more energy: energy*weight > lmenergy and cellid != locmaxcellid
          // 2) local maximum has cell has less than threshold energy left: energy*weight < m_threshold and cellid == locmaxcellid
          if ((energy * weight > lmenergy and cellid != locmaxcellid) or (energy * weight < m_threshold and cellid == locmaxcellid)) {
            markfordeletion.push_back(locmaxcellid);
            iterateclusters = true;
            continue;
          }
        }
      }

      // delete LMs
      for (const auto lmid : markfordeletion) {
        localMaximumsPoints.erase(lmid);
        centroidPoints.erase(lmid);
      }

    } while (iterateclusters);

    // Create the ECLShower objects, one per LocalMaximumPoints
    unsigned int iShower = 1;
    for (const auto& locmaxpoint : localMaximumsPoints) {

      const int locmaxcellid = locmaxpoint.first;
      const int posLM = m_StoreArrPositionLM[locmaxcellid];

      // Create a shower
      const auto aECLShower = m_eclShowers.appendNew();

      // Use the same method for the estimate (3x3).
      const double energyEstimation = estimateEnergy(locmaxcellid);

      // Get the neighbour list.
      ECLNeighbours* neighbourMap; // FIXME need pointer?
      int nNeighbours = getNeighbourMap(energyEstimation, backgroundLevel);
      if (nNeighbours == 9 and !m_useOptimalNumberOfDigitsForEnergy) neighbourMap = m_NeighbourMap9;
      else neighbourMap = m_NeighbourMap21;

      // Get the neighbour list.
      std::vector<short int> neighbourlist = neighbourMap->getNeighbours(locmaxcellid);

      // Get the weight vector.
      std::vector < double > myWeights = (*weightMap.find(locmaxcellid)).second;

      // Loop over all digits.
      std::vector<ECLCalDigit> newdigits;
      std::vector<double> newweights;
      double highestEnergy = 0.;
      double highestEnergyTime = 0.;
      double highestEnergyTimeResolution = 0.;
      double weightSum = 0.0;

      for (unsigned int i = 0; i < digitVector.size(); ++i) {

        const ECLCalDigit dig = digitVector[i];
        const double weight = myWeights[i];

        const int cellid = dig.getCellId();
        const int pos = m_StoreArrPosition[cellid];

        // Add weighted relations of all CalDigits to the local maximum.
        m_eclLocalMaximums[posLM]->addRelationTo(m_eclCalDigits[pos], weight);

        // Positive weight and in allowed neighbour list?
        if (weight > 0.0) {
          if (std::find(neighbourlist.begin(), neighbourlist.end(), cellid) != neighbourlist.end()) {

            aECLShower->addRelationTo(m_eclCalDigits[pos], weight);

            newdigits.push_back(dig);
            newweights.push_back(weight);

            weightSum += weight;

            const double energy = dig.getEnergy();

            if (energy * weight > highestEnergy) {
              highestEnergy = energy * weight;
              highestEnergyTime = dig.getTime();
              highestEnergyTimeResolution = dig.getTimeResolution();
            }
          }
        }
      }

      // Old position:
      B2Vector3D* oldshowerposition = new B2Vector3D((centroidList.find(locmaxcellid))->second);

      B2DEBUG(175, "old theta: " << oldshowerposition->Theta());
      B2DEBUG(175, "old phi: " << oldshowerposition->Phi());
      B2DEBUG(175, "old R: " << oldshowerposition->Mag());
      B2DEBUG(175, "old energy: " << energyEstimation);
      delete oldshowerposition;

      // New position (with reduced number of neighbours)
      // There are some cases where high backgrounds fake local maxima and the splitted centroid position is far
      // away from the original LM cell... this will throw a (non fatal) error, and create a cluster with zero energy now).
      B2Vector3D* showerposition = new B2Vector3D(Belle2::ECL::computePositionLiLo(newdigits, newweights, m_liloParameters));
      aECLShower->setTheta(showerposition->Theta());
      aECLShower->setPhi(showerposition->Phi());
      aECLShower->setR(showerposition->Mag());

      B2DEBUG(175, "new theta: " << showerposition->Theta());
      B2DEBUG(175, "new phi: " << showerposition->Phi());
      B2DEBUG(175, "new R: " << showerposition->Mag());
      delete showerposition;

      // Get Energy, if requested, set weights to zero for energy calculation.
      double showerEnergy = 0.0;
      if (m_useOptimalNumberOfDigitsForEnergy) {


        // Get the optimal number of neighbours for this crystal and energy
        std::vector<int> nOptimalVec = getOptimalNumberOfDigits(locmaxcellid, energyEstimation);
        const unsigned int nOptimal = static_cast<unsigned int>(nOptimalVec[0]);
        aECLShower->setNominalNumberOfCrystalsForEnergy(static_cast<double>(nOptimal));

        // Store the indices used; will be needed later for energy corrections.
        // Also store energy, used along with cellID to find leakage corrections.
        aECLShower->setNOptimalGroupIndex(nOptimalVec[1]);
        aECLShower->setNOptimalEnergyBin(nOptimalVec[2]);
        aECLShower->setNOptimalEnergy(energyEstimation);

        // Get the list of crystals used for the energy calculation
        std::vector< std::pair<unsigned int, double>> listCrystalPairs; // cell id and weighted reconstructed energy
        listCrystalPairs.resize(newdigits.size()); //resize to number of all crystals in cluster

        std::vector < std::pair<double, double> > weighteddigits;
        weighteddigits.resize(newdigits.size());
        for (unsigned int i = 0; i < newdigits.size(); ++i) {
          weighteddigits.at(i) = std::make_pair((newdigits.at(i)).getEnergy(), newweights.at(i));
          listCrystalPairs.at(i) = std::make_pair((newdigits.at(i)).getCellId(), newweights.at(i) * (newdigits.at(i)).getEnergy());
        }

        // sort the listCrystals and keep the n highest in descending order
        std::sort(listCrystalPairs.begin(), listCrystalPairs.end(), [](const auto & left, const auto & right) {
          return left.second > right.second;
        });

        std::vector< unsigned int> listCrystals; //cell id

        for (unsigned int i = 0; i < newdigits.size(); ++i) {
          if (i < nOptimal) {
            listCrystals.push_back(listCrystalPairs[i].first);
          }
        }

        aECLShower->setNumberOfCrystalsForEnergy(static_cast<double>(listCrystals.size()));
        aECLShower->setListOfCrystalsForEnergy(listCrystals);

        showerEnergy = getEnergySum(weighteddigits, nOptimal);
        B2DEBUG(175, "Shower Energy (2): " << showerEnergy);

      } else {
        showerEnergy = Belle2::ECL::computeEnergySum(newdigits, newweights);
      }

      aECLShower->setEnergy(showerEnergy);
      aECLShower->setEnergyRaw(showerEnergy);
      aECLShower->setEnergyHighestCrystal(highestEnergy);
      aECLShower->setTime(highestEnergyTime);
      aECLShower->setDeltaTime99(highestEnergyTimeResolution);
      aECLShower->setNumberOfCrystals(weightSum);
      aECLShower->setCentralCellId(locmaxcellid);

      B2DEBUG(175, "new energy: " << showerEnergy);

      // Get unique ID
      aECLShower->setShowerId(iShower);
      ++iShower;
      aECLShower->setHypothesisId(Belle2::ECLShower::c_nPhotons);
      aECLShower->setConnectedRegionId(aCR.getCRId());

      // Add relation to the CR.
      aECLShower->addRelationTo(&aCR);

      // Add relation to the LM.
      aECLShower->addRelationTo(m_eclLocalMaximums[posLM]);
    }
  }
}

int ECLSplitterN1Module::getNeighbourMap(const double energy, const double background)
{
  if (background <= 0.1) return 21;
  else {
    if (energy > 0.06 + 0.4 * background) return 21; // based on preliminary study TF, valid in barrel only (TF).
    else return 9;
  }
}

std::vector<int> ECLSplitterN1Module::getOptimalNumberOfDigits(const int cellid, const double energy)
{

  //..nOptimal depends on the energy bin, which in turn depends on ECL region
  int iRegion = 1; // barrel
  if (ECLElementNumbers::isForward(cellid)) {iRegion = 0;}
  if (ECLElementNumbers::isBackward(cellid)) {iRegion = 2;}

  //..Find the energy bin
  int iEnergy = 0;
  while (energy > m_eBoundaries[iRegion][iEnergy] and iEnergy < m_nEnergyBins - 1) {iEnergy++;}

  //..Group number just depends on the cellID
  int iGroup = m_groupNumber[cellid - 1];

  //..Optimal number of crystals from energy and group numbers
  int nOptimalNeighbours = (int)(0.5 + m_nOptimal2D.GetBinContent(iGroup + 1, iEnergy + 1));

  //..Store these in a vector to return
  std::vector<int> nOptimalVector;
  nOptimalVector.push_back(nOptimalNeighbours);
  nOptimalVector.push_back(iGroup);
  nOptimalVector.push_back(iEnergy);

  B2DEBUG(175, "ECLSplitterN1Module::getOptimalNumberOfDigits: cellID: " << cellid << " energy: " << energy << " iG: " << iGroup <<
          " iE: " << iEnergy << " nOpt: " << nOptimalNeighbours);

  return nOptimalVector;

}

double ECLSplitterN1Module::getEnergySum(std::vector < std::pair<double, double> >& weighteddigits, const unsigned int n)
{

  double energysum = 0.;

  std::sort(weighteddigits.begin(), weighteddigits.end(), [](const auto & left, const auto & right) {
    return left.first * left.second > right.first * right.second;
  });


  unsigned int min = n;
  if (weighteddigits.size() < n) min = weighteddigits.size();

  for (unsigned int i = 0; i < min; ++i) {
    B2DEBUG(175, "getEnergySum: " << weighteddigits.at(i).first << " " << weighteddigits.at(i).second);
    energysum += (weighteddigits.at(i).first * weighteddigits.at(i).second);
  }
  B2DEBUG(175, "getEnergySum: energysum=" << energysum);

  return energysum;
}


double ECLSplitterN1Module::estimateEnergy(const int centerid)
{

  double energyEstimation = 0.0;

  for (auto& neighbourId : m_NeighbourMap9->getNeighbours(centerid)) {

    // Check if this neighbour is in this CR
    const auto it = std::find(m_cellIdInCR.begin(), m_cellIdInCR.end(),
                              neighbourId); // check if the neighbour is in the list for this CR
    if (it == m_cellIdInCR.end()) continue; // not in this CR

    const int pos = m_StoreArrPosition[neighbourId];
    const double energyNeighbour = m_eclCalDigits[pos]->getEnergy();

    energyEstimation += energyNeighbour;
  }

  return energyEstimation;
}
