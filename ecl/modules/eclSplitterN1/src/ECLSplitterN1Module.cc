/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Main reconstruction splitter code for the N1 hypothesis.               *
 * Based on a connected region (CR) we look for local maxima and          *
 * create one shower for each local maximum (LM). In case of multiple     *
 * LM in one CR the energy is shared between the showers based on         *
 * their exponentially weighted distance in an iterative procedure. If    *
 * a CR has no LM the highest energetic digit in the CR is taken as LM.   *
 * The position is reconstructed using logarithmic weights for not too    *
 * small shower and linear weights otherwise ('lilo').                    *
 *                                                                        *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclSplitterN1/ECLSplitterN1Module.h>

// FRAMEWORK
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

// ECL
#include <ecl/utility/Position.h>

// OTHER
#include <string>

// NAMESPACES
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module(s)
//-----------------------------------------------------------------
REG_MODULE(ECLSplitterN1)
REG_MODULE(ECLSplitterN1PureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLSplitterN1Module::ECLSplitterN1Module() : Module(), m_eclCalDigits(eclCalDigitArrayName()),
  m_eclConnectedRegions(eclConnectedRegionArrayName()),
  m_eclShowers(eclShowerArrayName()), m_eclLocalMaximums(eclLocalMaximumArrayName()), m_eclEventInformation(eclEventInformationName())
{
  // Set description.
  setDescription("ECLSplitterN1Module: Baseline reconstruction splitter code for the all photon hypothesis (N1).");

  // Set module parameters.
  // Local Maximum.
  addParam("lmEnergyCut", m_lmEnergyCut, "Local maximum energy cut.", 17.5 * Belle2::Unit::MeV); // to be optimized!
  addParam("lmEnergyNeighbourCut", m_lmEnergyNeighbourCut, "Neighbours of maximum energy cut.",
           0.5 * Belle2::Unit::MeV); // to be optimized!
  addParam("lmMethod", m_lmMethod, "Method to find local maximum", std::string("BaBar"));

  // Local Maximum, Method specific parameters: BaBar method
  addParam("lmCutOffset", m_lmCutOffset, "Cut offset used to select Local Maxima", 2.5); // to be optimized!
  addParam("lmCutSlope", m_lmCutSlope, "Cut slope used to select Local Maxima", 0.5); // to be optimized!
  addParam("lmRatioCorrection", m_lmRatioCorrection, "Correction factor for ratio of emaxneighbour / emax", 0.0);

  // Local Maximum, Method specific parameters: BDT (coming soon)
  // ...

  // Splitter.
  addParam("expConstant", m_expConstant, "Constant a from exp(-a*dist/RM), typical: 1.5 to 3.5?", 2.5); // to be optimized!
  addParam("maxIterations", m_maxIterations, "Maximum number of iterations for centroid shifts.", 100);
  addParam("shiftTolerance", m_shiftTolerance, "Tolerance level for centroid shifts.", 1.0 * Belle2::Unit::mm);
  addParam("minimumSharedEnergy", m_minimumSharedEnergy, "Minimum shared energy.", 25.0 * Belle2::Unit::keV);
  addParam("maxSplits", m_maxSplits, "Maximum number of splits within one connected region.", 10);

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

  // Check user input.
  m_liloParameters.resize(3);
  m_liloParameters.at(0) = m_liloParameterA;
  m_liloParameters.at(1) = m_liloParameterB;
  m_liloParameters.at(2) = m_liloParameterC;

  if (m_lmMethod == "BaBar" and m_lmRatioCorrection >= m_lmEnergyNeighbourCut) {
    B2ERROR("ECLCRSplitterModule::initialize(): Choose a energy neighbour cut value of greater than the ratio correction.");
  }

  if (m_lmEnergyNeighbourCut >= m_lmEnergyCut) {
    B2ERROR("ECLCRSplitterModule::initialize(): Choose a energy neighbour cut value of greater than the central energy cut.");
  }

  // ECL dataobjects.
  m_eclCalDigits.registerInDataStore(eclCalDigitArrayName());
  m_eclConnectedRegions.registerInDataStore(eclConnectedRegionArrayName());
  m_eclShowers.registerInDataStore(eclShowerArrayName());
  m_eclLocalMaximums.registerInDataStore(eclLocalMaximumArrayName());
  m_eclEventInformation.registerInDataStore(eclEventInformationName());

  // Register relations (we probably dont need all, but keep them for now for debugging).
  m_eclConnectedRegions.registerRelationTo(m_eclCalDigits);
  m_eclConnectedRegions.registerRelationTo(m_eclLocalMaximums);
  m_eclLocalMaximums.registerRelationTo(m_eclCalDigits);
  m_eclShowers.registerRelationTo(m_eclConnectedRegions);
  m_eclShowers.registerRelationTo(m_eclCalDigits);
  m_eclShowers.registerRelationTo(m_eclLocalMaximums);

  // Initialize neighbour maps (we will optimize the endcaps later, there is more than just a certain energy containment to be considered)
  m_NeighbourMap9 = new ECLNeighbours("N", 1); // 3x3 = 9
  m_NeighbourMap21 = new ECLNeighbours("NC", 2); // 5x5 excluding corners = 21
}

void ECLSplitterN1Module::beginRun()
{
  ;
}

void ECLSplitterN1Module::event()
{
  // Loop over all connected regions in this event to find possible significant local maxima.
  // They will probably have the same or higher energy cut than the seed cell cut of the CRFinder.
  std::map < int, ECLCalDigit* > CellIdToDigitPointerMap;

  // Loop over all connected regions
  for (auto& aCR : m_eclConnectedRegions) {

    CellIdToDigitPointerMap.clear();

    // Fill all calDigits in this CR into a map to make them accesible by cellid.
    for (auto& aCalDigit : aCR.getRelationsTo<ECLCalDigit>()) {
      const int cellid = aCalDigit.getCellId();
      CellIdToDigitPointerMap[cellid] = &aCalDigit;
    }

    // Make the local maximums.
    makeLocalMaximums(aCR, CellIdToDigitPointerMap);

    // Split and reconstruct the showers in this connected regions.
    splitConnectedRegion(aCR, CellIdToDigitPointerMap);

  } // end auto& aCR

}


void ECLSplitterN1Module::endRun()
{
  ;
}

void ECLSplitterN1Module::terminate()
{
  ;
}

void ECLSplitterN1Module::makeLocalMaximums(ECLConnectedRegion& aCR, std::map < int, ECLCalDigit* >& myCellIdToDigitPointerMap)
{
  B2DEBUG(175, "ECLCRSplitterModule::makeLocalMaximums(): START!");

  // Output Array
  StoreArray<ECLLocalMaximum> eclLocalMaximums(eclLocalMaximumArrayName());

  std::vector < double > energyVector;
  int nNeighbours = 0;
  int lmidentifier = 0;
//  double energy = 0.0;

  // Loop over this map and check neighbours.
  for (auto const& entry : myCellIdToDigitPointerMap) {
    energyVector.clear();
    nNeighbours = 0;
    const double energy = (entry.second)->getEnergy();

    // Energy cut.
    if (energy < m_lmEnergyCut) continue;

    // Local maximum selection.
    for (auto& neighbourId : m_NeighbourMap9->getNeighbours(entry.first)) {

      if (neighbourId == entry.first) continue; // skip the center cell

      const auto it = myCellIdToDigitPointerMap.find(neighbourId); // check if the neighbour is in the list
      if (it == myCellIdToDigitPointerMap.end()) continue; // not in this CR

      const double energyNeighbour = ((*it).second)->getEnergy();

      // only neighbours with E < Elocmaxcand
      if (energyNeighbour < energy) {
        energyVector.push_back(energyNeighbour); // append this cell energy
      } else { // neighbour has higher energy, we can abort here and now!
        energyVector.clear();
        nNeighbours = -1;
        break;
      }

      // all neighbours
      ++nNeighbours;

    } // end auto& neighbourId

    // This cell could be a local maximum (single high energetic crystals (e.g. MIPs) will survive this).
    const unsigned int sizeEnergyVector = energyVector.size();

    B2DEBUG(175, "ECLCRSplitterModule::makeLocalMaximums(): end auto& neighbourId! " << sizeEnergyVector << " " << nNeighbours);

    if ((sizeEnergyVector > 0 && nNeighbours > 0) || nNeighbours == 0) {

      // Maximum neighbour energy.
      double maxNeighbourEnergy = 0.0;
      if (sizeEnergyVector > 0) {
        auto max = std::max_element(std::begin(energyVector), std::end(energyVector));
        maxNeighbourEnergy = *max;
      }

      // Cut
      if (m_lmCutSlope * (nNeighbours - m_lmCutOffset) >= (maxNeighbourEnergy - m_lmRatioCorrection) / (energy - m_lmRatioCorrection)) {

        // Set the local maximum dataobject.
        if (!eclLocalMaximums) eclLocalMaximums.create();
        const auto aLocalMaximum = eclLocalMaximums.appendNew();

        B2DEBUG(175, "ECLCRSplitterModule::makeLocalMaximums(): local max is cellid: " << entry.first);

        // Set the id of this local maximum and increment the counter.
        aLocalMaximum->setLMId(lmidentifier);
        ++lmidentifier;

        // Set the number of neighbours (equal to the number of entries in the vector).
        aLocalMaximum->setNNeighbours(energyVector.size());

        // Set the maximum energy.
        aLocalMaximum->setMaximumEnergyNeighbours(maxNeighbourEnergy);

        // Set the list of neighbours (for monitoring/debugging).
        aLocalMaximum->setNeighbourEnergiesList(energyVector);

        // Add relations to ECLConnectedRegion.
        aCR.addRelationTo(aLocalMaximum);

        // Add relation to the underlying ECLCalDigit.
        const auto it = myCellIdToDigitPointerMap.find(entry.first);
        aLocalMaximum->addRelationTo(((*it).second));
      }

    }

  } // end auto const& entry

  B2DEBUG(175, "ECLCRSplitterModule::makeLocalMaximums(): DONE!");

}


void ECLSplitterN1Module::splitConnectedRegion(ECLConnectedRegion& aCR, std::map < int, ECLCalDigit* >& myCellIdToDigitPointerMap)
{

  // Get the event background level
  const int bkgdcount = m_eclEventInformation->getBackgroundECL();
  double backgroundLevel = 0.0;
  if (c_fullBkgdCount > 0) backgroundLevel = static_cast<double>(bkgdcount) / static_cast<double>(c_fullBkgdCount);

  // Get the number of LMs in this CR
  const int nLocalMaximums = aCR.getRelationsTo<ECLLocalMaximum>().size();

  B2INFO("ECLCRSplitterModule::splitConnectedRegion: nLocalMaximums = " << nLocalMaximums);

  // Three cases:
  // 1) There is no local maximum (most likely in presence of high background) or there are too many:
  //    Make one photon around the highest largest energy deposition in this CR.
  // 2) There is exactly one local maximum, this is the easiest (and most likely for true photons) case.
  // 3) There are more than one, typically two or three, local maxima and we have to share energy between them.

  // ---------------------------------------------------------------------
  if (nLocalMaximums <= 1 or nLocalMaximums >= m_maxSplits) { // case 1 or 2

    // Create a shower.
    if (!m_eclShowers) m_eclShowers.create();
    const auto aECLShower = m_eclShowers.appendNew();

    // Add relation to the CR.
    aECLShower->addRelationTo(&aCR);

    // Find the highest energetic crystal in this CR or use the LM.
    unsigned int highestEnergyID = 0;
    if (nLocalMaximums != 1) {
      double highestEnergy = 0.0;

      for (auto const& aECLCalDigit : aCR.getRelationsTo<ECLCalDigit>()) {
        if (aECLCalDigit.getEnergy() > highestEnergy) {
          highestEnergy = aECLCalDigit.getEnergy();
          highestEnergyID = aECLCalDigit.getCellId();
        }
      }
    } else {
      RelationVector<ECLLocalMaximum> locmaxvector = aCR.getRelationsTo<ECLLocalMaximum>();
      aECLShower->addRelationTo(locmaxvector[0]);

      RelationVector<ECLCalDigit> caldigitvector = locmaxvector[0]->getRelationsTo<ECLCalDigit>();
      highestEnergyID = (caldigitvector[0])->getCellId();
    }

    // Get a first estimation of the energy using 3x3 neighbours.
    std::map<int, ECLCalDigit*>::iterator it; // iterator for the CR digit map
    const double energyEstimation = estimateEnergy(highestEnergyID, myCellIdToDigitPointerMap);

    // Check if 21 would be better in the present background conditions:
    ECLNeighbours* neighbourMap;
    int nNeighbours = getNeighbourMap(energyEstimation, backgroundLevel);
    if (nNeighbours == 9) neighbourMap = m_NeighbourMap9;
    else neighbourMap = m_NeighbourMap21;

    // Add neighbours etc (this is either a very low energetic photon or garbage anyhow...).
    std::vector<ECLCalDigit> digits;
    std::vector<double> weights;
    for (auto& neighbourId : neighbourMap->getNeighbours(highestEnergyID)) {

      // Check if this neighbouts is in this CR
      it = myCellIdToDigitPointerMap.find(neighbourId);
      if (it != myCellIdToDigitPointerMap.end()) {
        digits.push_back(*(*it).second); // list of digits for position reconstruction
        weights.push_back(1.0); // list of weights (all 1 in this case for now)
        aECLShower->addRelationTo((*it).second, 1.0); // add digits to this shower
      }
    }

    // Get position.
    const TVector3& showerposition = Belle2::ECL::computePositionLiLo(digits, weights, m_liloParameters);
    aECLShower->setTheta(showerposition.Theta());
    aECLShower->setPhi(showerposition.Phi());
    aECLShower->setR(showerposition.Mag());

    // Get Energy.
    const double energy = Belle2::ECL::computeEnergySum(digits, weights);
    aECLShower->setEnergy(energy);

    // Time and time resolution
    // ...

    B2DEBUG(175, "theta           = " << showerposition.Theta());
    B2DEBUG(175, "phi             = " << showerposition.Phi());
    B2DEBUG(175, "R               = " << showerposition.Mag());
    B2DEBUG(175, "energy          = " << energy);
    B2DEBUG(175, "neighbours      = " << nNeighbours);
    B2DEBUG(175, "backgroundLevel = " << backgroundLevel);

    // Get unique ID
    const int showerId = m_SUtility.getShowerId(aCR.getCRId(), c_Hypothesis,
                                                0); // since there is only one shower, we give it the id 0 (or 1?)
    aECLShower->setShowerId(showerId);

  } else { // case 3
    B2DEBUG(175, "case 3");

    std::vector<ECLCalDigit> digits;
    std::vector<double> weights;
    std::map<int, TVector3*> centroidList; // key = cellid, value = centroid position
    std::map<int, double> centroidEnergyList; // key = cellid, value = centroid position
    std::map<int, TVector3*> centroidPoints; // key = locmaxid (as index), value = centroid position
    std::map<int, TVector3*> localMaximumsPoints; // key = locmaxid, value = maximum position
    std::map<int, TVector3*> allPoints; // key = cellid, value = digit position
    std::map<int, std::vector < double > > weightMap; // key = locmaxid, value = vector of weights
    std::vector < ECLCalDigit* >
    digitVector; // the same for all local maxima, the order of weights in weightMap must be the same though

    // Fill the maxima positions in a map
    for (auto& aLocalMaximum : aCR.getRelationsTo<ECLLocalMaximum>()) {
      // get the underlying digit
      int cellid = -1;
      for (auto& aCalDigit : aLocalMaximum.getRelationsTo<ECLCalDigit>()) {
        if (cellid == -1) cellid = aCalDigit.getCellId();
        else B2FATAL("Multiple ECLCalDigit relations from one Local Maximum?"); // TF: Can that happen?
      }

      // get the position of this crystal and fill them in a maps
      TVector3* vectorPosition = new TVector3(m_geom->GetCrystalPos(cellid - 1));
      localMaximumsPoints.insert(std::map<int, TVector3*>::value_type(cellid, vectorPosition));
      centroidPoints.insert(std::map<int, TVector3*>::value_type(cellid, vectorPosition));
    }

    // Fill all digits in a map
    for (auto& aCalDigit : aCR.getRelationsTo<ECLCalDigit>()) {
      const int cellid = aCalDigit.getCellId();
      // get the position of this crystal and fill them in a map
      TVector3* vectorPosition = new TVector3(m_geom->GetCrystalPos(cellid - 1));
      allPoints.insert(std::map<int, TVector3*>::value_type(cellid, vectorPosition));

      digits.push_back(aCalDigit);
    }

    for (const auto& digitpoint : allPoints) {
      digitVector.push_back((*myCellIdToDigitPointerMap.find(digitpoint.first)).second);
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

        // if this is the first iteration the shower energy is not know, take the local maximum energy * 2.
        if (nIterations == 0) {
          const double locmaxenergy = ((*myCellIdToDigitPointerMap.find(locmaxcellid)).second)->getEnergy();
          centroidEnergyList[locmaxcellid] = 2.*locmaxenergy;
        }

        B2DEBUG(175, "local maximum cellid: " << locmaxcellid);

        //-------------------------------------------------------------------
        // Loop over all digits. They get a weight using the distance to the respective centroid.
        int nDigit = 0;
        for (const auto& digitpoint : allPoints) {

          // cellid and position of this digit
          const int digitcellid = digitpoint.first;
          TVector3* digitpos = digitpoint.second;
          const double digitenergy = ((*myCellIdToDigitPointerMap.find(digitcellid)).second)->getEnergy();

          double weight            = 0.0;
          double energy            = 0.0;
          double distance          = 0.0;
          double distanceEnergySum = 0.0;

          // Loop over all centroids
          for (const auto& centroidpoint : centroidPoints) {

            // cell id and position of this centroid
            const int centroidcellid = centroidpoint.first;
            TVector3* centroidpos = centroidpoint.second;

            double thisdistance = 0.;

            // in the first iteration, this distance is really zero, avoid floating point problems
            if (nIterations == 0 and digitcellid == centroidcellid) {
              thisdistance = 0.0;
            } else {
              TVector3 vectorDistance = ((*centroidpos) - (*digitpos));
              thisdistance = vectorDistance.Mag();
            }

            // energy of the centroid aka locmax
            const double thisenergy = ((*myCellIdToDigitPointerMap.find(centroidcellid)).second)->getEnergy();

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

          // Check if the weighted energy is above threshold or too far away
          if ((digitenergy * weight) < m_minimumSharedEnergy) {
            weight = 0.0;
          }

          // Check for rounding problems larger than unity
          if (weight > 1.0) {
            B2WARNING("ECLCRSplitterModule::splitConnectedRegion: weight for this digit " << weight << ", resetting it to 1.0.");
            weight = 1.0;
          }

          // Fill the weight for this digits and this local maximum.
          B2DEBUG(175, "   cellid: " << digitcellid << ", energy: " << digitenergy << ", weight: " << weight << ", distance: " << distance);
          weights.push_back(weight);
          ++nDigit;

        } // end allPoints

        // Get the old centroid position.
        TVector3* oldCentroidPos = (*centroidPoints.find(locmaxcellid)).second;

        // Calculate the new centroid position.
        TVector3* newCentroidPos = new TVector3(Belle2::ECL::computePositionLiLo(digits, weights, m_liloParameters));

        // Calculate new energy
        const double newEnergy = Belle2::ECL::computeEnergySum(digits, weights);

        // Calculate the shift of the centroid position for this local maximum.
        const TVector3 centroidShift = (*oldCentroidPos - *newCentroidPos);

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
        B2DEBUG(175, "   old centroid: " << oldCentroidPos->x() << " cm, " <<  oldCentroidPos->y() << " cm, " <<  oldCentroidPos->z() <<
                "cm");
        B2DEBUG(175, "   new centroid: " << newCentroidPos->x() << " cm, " <<  newCentroidPos->y() << " cm, " <<  newCentroidPos->z() <<
                "cm");
        B2DEBUG(175, "   centroid shift: " << centroidShift.Mag() << " cm");

      } // end localMaximumsPoints

      // Get the average centroid shift.
      centroidShiftAverage /= static_cast<double>(nLocalMaximums);
//      lastcentroidShiftAverage = centroidShiftAverage;
      B2DEBUG(175, "--> average centroid shift: " << centroidShiftAverage << " cm (tolerance is " << m_shiftTolerance << " cm)");

      // Update centroid positions for the next round
      for (const auto& locmaxpoint : localMaximumsPoints) {
        centroidPoints[locmaxpoint.first] = (*centroidList.find(locmaxpoint.first)).second;
      }

      ++nIterations;

    } while (nIterations < m_maxIterations and centroidShiftAverage > m_shiftTolerance);
    // DONE!

    // Create the ECLShower objects, one per LocalMaximum
    unsigned int iShower = 0;
    for (const auto& locmaxpoint : localMaximumsPoints) {

      const int locmaxcellid = locmaxpoint.first;

      // Create a shower
      if (!m_eclShowers) m_eclShowers.create();
      const auto aECLShower = m_eclShowers.appendNew();

      // We can use the existing weighted energy estimate as starting point.
      const double energyEstimation = (*centroidEnergyList.find(locmaxcellid)).second;

      // Get the neighbour list.
      ECLNeighbours* neighbourMap;
      int nNeighbours = getNeighbourMap(energyEstimation, backgroundLevel);
      if (nNeighbours == 9) neighbourMap = m_NeighbourMap9;
      else neighbourMap = m_NeighbourMap21;

      // Get the neighbour list.
      std::vector<short int> neighbourlist = neighbourMap->getNeighbours(locmaxcellid);

      // Get the weight vector.
      std::vector < double > myWeights = (*weightMap.find(locmaxcellid)).second;

      // Loop over all digits.
      std::vector<ECLCalDigit> digits;
      std::vector<double> weights;

      for (unsigned int i = 0; i < digitVector.size(); ++i) {
        const ECLCalDigit* digit = digitVector[i];
        const double weight = myWeights[i];
        const int cellid = digit->getCellId();

        // Positive weight and in allowed neighbour list?
        if (weight > 0) {
          if (std::find(neighbourlist.begin(), neighbourlist.end(), cellid) != neighbourlist.end()) {
            aECLShower->addRelationTo(digit, weight);

            digits.push_back(*digit);
            weights.push_back(weight);
          }
        }
      }

      // Old position:
      TVector3* oldshowerposition = (*centroidList.find(locmaxcellid)).second;

      B2DEBUG(175, "old theta: " << oldshowerposition->Theta());
      B2DEBUG(175, "old phi: " << oldshowerposition->Phi());
      B2DEBUG(175, "old R: " << oldshowerposition->Mag());
      B2DEBUG(175, "old energy: " << energyEstimation);

      // New position (with reduced number of neighbours)
      // There are some cases where high backgrounds fake local maxima and the splitted centroid position is far
      // away from the original LM cell... this will throw a (non fatal) error, and create a cluster with zero energy now).
      TVector3* showerposition = new TVector3(Belle2::ECL::computePositionLiLo(digits, weights, m_liloParameters));
      aECLShower->setTheta(showerposition->Theta());
      aECLShower->setPhi(showerposition->Phi());
      aECLShower->setR(showerposition->Mag());
      B2DEBUG(175, "new theta: " << showerposition->Theta());
      B2DEBUG(175, "new phi: " << showerposition->Phi());
      B2DEBUG(175, "new R: " << showerposition->Mag());

      // New energy
      const double showerenergy = Belle2::ECL::computeEnergySum(digits, weights);
      aECLShower->setEnergy(showerenergy);
      B2DEBUG(175, "new energy: " << showerenergy);

      // Time and time resolution
      // ...

      // Get unique ID
      const int showerId = m_SUtility.getShowerId(aCR.getCRId(), c_Hypothesis, iShower);
      ++iShower;
      aECLShower->setShowerId(showerId);

      // Add relation to the CR.
      aECLShower->addRelationTo(&aCR);

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


int ECLSplitterN1Module::estimateEnergy(const int centerid, std::map < int, ECLCalDigit* >& cellIdToDigitPointerMap)
{

  double energyEstimation = 0.0;
  std::map<int, ECLCalDigit*>::iterator it; // iterator for the CR digit map

  for (auto& neighbourId : m_NeighbourMap9->getNeighbours(centerid)) {

    // Check if this neighbour is in this CR
    it = cellIdToDigitPointerMap.find(neighbourId);
    if (it != cellIdToDigitPointerMap.end()) {
      energyEstimation += (*it).second->getEnergy();
    }
  }

  return energyEstimation;
}
