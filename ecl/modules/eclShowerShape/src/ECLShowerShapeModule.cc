/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates shower shape variables.                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Alon Hershenhorn   (hershen@phas.ubc.ca)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclShowerShape/ECLShowerShapeModule.h>

// FRAMEWORK
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>

#include <framework/logging/Logger.h>
#include <framework/geometry/B2Vector3.h>

// ECL
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/geometry/ECLGeometryPar.h>

// MDST
#include <mdst/dataobjects/ECLCluster.h>

// MVA package
#include <mva/interface/Interface.h>

// ROOT
#include <TMath.h>

//BOOST
#include <boost/algorithm/string/predicate.hpp>
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLShowerShape)
REG_MODULE(ECLShowerShapePureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLShowerShapeModule::ECLShowerShapeModule() : Module(), m_secondMomentCorrectionArray("ecl_shower_shape_second_moment_corrections")
{
  // Set description
  setDescription("ECLShowerShapeModule: Calculate ECL shower shape variables (e.g. E9oE21)");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("zernike_n1_rho0", m_zernike_n1_rho0,
           "Scaling factor for radial distances in a plane perpendicular to direction to shower for the n photon hypothesis in Zernike moment calculation.",
           10.0 * Unit::cm);

  addParam("zernike_n2_rho0", m_zernike_n2_rho0,
           "Scaling factor for radial distances in a plane perpendicular to direction to shower for the neutral hadron hypothesis in Zernike moment calculation. ",
           20.0 * Unit::cm);

  addParam("zernike_useFarCrystals", m_zernike_useFarCrystals,
           "Determines if Digits with rho > rho0 are used for the zernike moment calculation. If they are, their radial distance will be set to rho0.",
           true);

  addParam("zernike_MVAidentifier_FWD", m_zernike_MVAidentifier_FWD, "The Zernike moment MVA database identifier for forward endcap.",
           std::string{"ecl_showershape_zernike_mva_fwd"});
  addParam("zernike_MVAidentifier_BRL", m_zernike_MVAidentifier_BRL, "The Zernike moment MVA database identifier for barrel.",
           std::string{"ecl_showershape_zernike_mva_brl"});
  addParam("zernike_MVAidentifier_BWD", m_zernike_MVAidentifier_BWD,
           "The Zernike moment MVA database identifier for backward endcap.", std::string{"ecl_showershape_zernike_mva_bwd"});

  addParam("avgCrystalDimension", m_avgCrystalDimension,
           "Average crystal dimension used in lateral energy calculation.",
           5.0 * Unit::cm);

}

ECLShowerShapeModule::~ECLShowerShapeModule()
{
}

void ECLShowerShapeModule::initializeMVAweightFiles(const std::string& identifier,
                                                    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>& weightFileRepresentation)
{
  if (not(boost::ends_with(identifier, ".root") or boost::ends_with(identifier, ".xml"))) {
    weightFileRepresentation = std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>(new
                               DBObjPtr<DatabaseRepresentationOfWeightfile>(identifier));
  }
  MVA::AbstractInterface::initSupportedInterfaces();
}

void ECLShowerShapeModule::initialize()
{
  // Initialize neighbour maps.
  m_neighbourMap9 = new ECLNeighbours("N", 1);
  m_neighbourMap21 = new ECLNeighbours("NC", 2);

  initializeMVAweightFiles(m_zernike_MVAidentifier_FWD, m_weightfile_representation_FWD);
  initializeMVAweightFiles(m_zernike_MVAidentifier_BRL, m_weightfile_representation_BRL);
  initializeMVAweightFiles(m_zernike_MVAidentifier_BWD, m_weightfile_representation_BWD);

  //Add callback to fill m_secondMomentCorrections when m_secondMomentCorrectionArray changes
  //21-Oct-2016 - The callback doesn't seem to be called at the begining of the run so I commented it out and added a call to prepareSecondMomentCorrectionsCallback in the beginRun
//   m_secondMomentCorrectionArray.addCallback(this, &ECLShowerShapeModule::prepareSecondMomentCorrectionsCallback);
//   prepareSecondMomentCorrectionsCallback();

}

void ECLShowerShapeModule::initializeMVA(const std::string& identifier,
                                         std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>& weightFileRepresentation, std::unique_ptr<MVA::Expert>& expert)
{
  MVA::Weightfile  weightfile;
  //Load MVA weight file
  if (weightFileRepresentation) {

    //If multiple sources of conditions DB have been configured (the regular case), then the IOV of each payload will be "artificially" set to the current run only.
    //This is so that in the next run, the payload can be taken from a different DB source.
    //For example, payload of current run will be taken from central DB and payload of next run will be taken from local DB, if it appears there.
    //In this case weightFileRepresentation->hasChanged() will be true at the beginning of each run, even though the IOV of the payload is greater than a single run.
    //This is true as of 2017-06-01, functionality of hasChanged() might be changed in future.

    if (weightFileRepresentation->hasChanged()) {
      std::stringstream ss((*weightFileRepresentation)->m_data);
      weightfile = MVA::Weightfile::loadFromStream(ss);
    } else
      return;
  } else {
    weightfile = MVA::Weightfile::loadFromFile(identifier);
  }

  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
  MVA::GeneralOptions general_options;
  weightfile.getOptions(general_options);

  //Check number of variables in weight file
  if (m_numZernikeMVAvariables != general_options.m_variables.size())
    B2FATAL("Expecting " << m_numZernikeMVAvariables << " varibales, found " << general_options.m_variables.size());

  expert = supported_interfaces[general_options.m_method]->getExpert();
  expert->load(weightfile);

  //create new dataset, if this is the barrel MVA (assumes FWD and BWD datasets are same size)
  if (weightFileRepresentation == m_weightfile_representation_BRL) {
    std::vector<float> dummy(general_options.m_variables.size(), 0);
    m_dataset = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(general_options, dummy, 0));
  }
}

void ECLShowerShapeModule::beginRun()
{
  initializeMVA(m_zernike_MVAidentifier_FWD, m_weightfile_representation_FWD, m_expert_FWD);
  initializeMVA(m_zernike_MVAidentifier_BRL, m_weightfile_representation_BRL, m_expert_BRL);
  initializeMVA(m_zernike_MVAidentifier_BWD, m_weightfile_representation_BWD, m_expert_BWD);

  //This is a hack because the callback doesn't seem to be called at the begining of the run
  if (m_secondMomentCorrectionArray.hasChanged()) {
    if (m_secondMomentCorrectionArray) prepareSecondMomentCorrectionsCallback();
    else B2ERROR("ECLShowerShapeModule::beginRun - Couldn't find second moment correction for current run");
  }
}


void ECLShowerShapeModule::setShowerShapeVariables(ECLShower* eclShower, const bool calculateZernikeMVA) const
{
  //Project the digits on the plane perpendicular to the shower direction
  std::vector<ProjectedECLDigit> projectedECLDigits = projectECLDigits(*eclShower);

  const double showerEnergy = eclShower->getEnergy();
  const double showerTheta = eclShower->getTheta();
  const double showerPhi = eclShower->getPhi();

  //sum crystal energies
  double sumEnergies = 0.0;
  for (const auto& projectedECLDigit : projectedECLDigits) sumEnergies += projectedECLDigit.energy;

  //Choose rho0 according to shower hypothesis
  double rho0 = 0.0;
  const int hypothesisID = eclShower->getHypothesisId();
  if (hypothesisID == ECLCluster::c_nPhotons) rho0 = m_zernike_n1_rho0;
  else if (hypothesisID == ECLCluster::c_neutralHadron) rho0 = m_zernike_n2_rho0;

  const double absZernike40 = computeAbsZernikeMoment(projectedECLDigits, sumEnergies, 4, 0, rho0);
  const double absZernike51 = computeAbsZernikeMoment(projectedECLDigits, sumEnergies, 5, 1, rho0);

  const double secondMomentCorrection = getSecondMomentCorrection(showerTheta, showerPhi, hypothesisID);
  B2DEBUG(175, "Second moment angular correction: " << secondMomentCorrection << " (theta(rad)=" << showerTheta << ", phi(rad)=" <<
          showerPhi << ",hypothesisId=" << hypothesisID <<
          ")");
  const double secondMoment = computeSecondMoment(projectedECLDigits, showerEnergy) * secondMomentCorrection;
  B2DEBUG(175, "Second moment after correction: " << secondMoment);

  const double LATenergy    = computeLateralEnergy(projectedECLDigits, m_avgCrystalDimension);

  // Set shower shape variables.
  eclShower->setAbsZernike40(absZernike40);
  eclShower->setAbsZernike51(absZernike51);
  eclShower->setSecondMoment(secondMoment);
  eclShower->setLateralEnergy(LATenergy);
  eclShower->setE1oE9(computeE1oE9(*eclShower));
  if (eclShower->getE9oE21() < 1e-9) eclShower->setE9oE21(computeE9oE21(*eclShower));

  if (calculateZernikeMVA) {
    //Set Zernike moments that will be used in MVA calculation
    // m_dataset holds 22 entries, 11 Zernike moments of N2 shower, followed by 11 Zernike moments of N1 shower
    int indexOffset = 0;//Offset entries depending on hypothesis type
    if (hypothesisID == ECLCluster::c_nPhotons) indexOffset = (m_numZernikeMVAvariables / 2);
    else if (hypothesisID == ECLCluster::c_neutralHadron) indexOffset = 0;

    m_dataset->m_input[0 + indexOffset] = computeAbsZernikeMoment(projectedECLDigits, sumEnergies, 1, 1, rho0);
    m_dataset->m_input[1 + indexOffset] = computeAbsZernikeMoment(projectedECLDigits, sumEnergies, 2, 0, rho0);
    m_dataset->m_input[2 + indexOffset] = computeAbsZernikeMoment(projectedECLDigits, sumEnergies, 2, 2, rho0);
    m_dataset->m_input[3 + indexOffset] = computeAbsZernikeMoment(projectedECLDigits, sumEnergies, 3, 1, rho0);
    m_dataset->m_input[4 + indexOffset] = computeAbsZernikeMoment(projectedECLDigits, sumEnergies, 3, 3, rho0);
    m_dataset->m_input[5 + indexOffset] = absZernike40;
    m_dataset->m_input[6 + indexOffset] = computeAbsZernikeMoment(projectedECLDigits, sumEnergies, 4, 2, rho0);
    m_dataset->m_input[7 + indexOffset] = computeAbsZernikeMoment(projectedECLDigits, sumEnergies, 4, 4, rho0);
    m_dataset->m_input[8 + indexOffset] = absZernike51;
    m_dataset->m_input[9 + indexOffset] = computeAbsZernikeMoment(projectedECLDigits, sumEnergies, 5, 3, rho0);
    m_dataset->m_input[10 + indexOffset] = computeAbsZernikeMoment(projectedECLDigits, sumEnergies, 5, 5, rho0);
    //Set zernikeMVA for N1 showers
    //This assumes that the N2 zernike moments have already been set in m_dataset!!!!
    if (hypothesisID == ECLCluster::c_nPhotons) {
      //FWD
      if (eclShower->getTheta() < m_BRLthetaMin) eclShower->setZernikeMVA(m_expert_FWD->apply(*m_dataset)[0]);
      //BWD
      else if (eclShower->getTheta() > m_BRLthetaMax) eclShower->setZernikeMVA(m_expert_BWD->apply(*m_dataset)[0]);
      //BRL
      else eclShower->setZernikeMVA(m_expert_BRL->apply(*m_dataset)[0]);
    }
  } else eclShower->setZernikeMVA(0.0);
}

void ECLShowerShapeModule::event()
{
  StoreArray<ECLConnectedRegion> eclConnectedRegions(eclConnectedRegionArrayName());

  for (auto& eclCR : eclConnectedRegions) {

    //Start by finding the N2 shower and calculating it's shower shape variables
    //Assumes that there is only 1 N2 Shower per CR!!!!!!
    ECLShower* N2shower = nullptr;
    for (auto& eclShower : eclCR.getRelationsWith<ECLShower>(eclShowerArrayName())) {
      if (eclShower.getHypothesisId() == ECLCluster::c_neutralHadron) {
        N2shower = &eclShower;
        setShowerShapeVariables(N2shower, true);
        break;
      }
    }

    //If couldn't find N2 shower, don't calculate zernikeMVA
    bool found_N2shower = true;
    if (N2shower == nullptr) found_N2shower = false;

    double prodN1zernikeMVAs = 1.0;
    //Calculate shower shape variables for the rest of the showers
    for (auto& eclShower : eclCR.getRelationsWith<ECLShower>(eclShowerArrayName())) {
      if (eclShower.getHypothesisId() == ECLCluster::c_neutralHadron)
        continue; //shower shape variables already calculated for neutral hadrons

      bool calculateZernikeMVA = true;
      if (!found_N2shower || eclShower.getHypothesisId() != ECLCluster::c_nPhotons) calculateZernikeMVA = false;

      setShowerShapeVariables(&eclShower, calculateZernikeMVA);

      if (eclShower.getHypothesisId() == ECLCluster::c_nPhotons) prodN1zernikeMVAs *= eclShower.getZernikeMVA();
    }

    //Set zernikeMVA for the N2 shower
    if (N2shower) N2shower->setZernikeMVA(1.0 - prodN1zernikeMVAs);
  }
}

std::vector<ECLShowerShapeModule::ProjectedECLDigit> ECLShowerShapeModule::projectECLDigits(const ECLShower& shower) const
{
  std::vector<ProjectedECLDigit> tmpProjectedECLDigits; //Will be returned at the end of the function
  auto showerDigitRelations = shower.getRelationsTo<ECLCalDigit>(eclCalDigitArrayName());
//   tmpProjectedECLDigits.resize( showerDigitRelations.size() );
  //---------------------------------------------------------------------
  // Get shower parameters.
  //---------------------------------------------------------------------
  const double showerR = shower.getR();
  const double showerTheta = shower.getTheta();
  const double showerPhi = shower.getPhi();

  B2Vector3D showerPosition;
  showerPosition.SetMagThetaPhi(showerR, showerTheta, showerPhi);

  // Unit vector pointing in shower direction.
  const B2Vector3D showerDirection = (1.0 / showerPosition.Mag()) * showerPosition;

  //---------------------------------------------------------------------
  // Calculate axes that span the perpendicular plane.
  //---------------------------------------------------------------------
  //xPrimeDirection = showerdirection.cross(zAxis)
  B2Vector3D xPrimeDirection = B2Vector3D(showerPosition.y(), -showerPosition.x(), 0.0);
  xPrimeDirection *= 1.0 / xPrimeDirection.Mag();

  B2Vector3D yPrimeDirection = xPrimeDirection.Cross(showerDirection);
  yPrimeDirection *= 1.0 / yPrimeDirection.Mag();

  //---------------------------------------------------------------------
  // Loop on CalDigits in shower and calculate the projection.
  //---------------------------------------------------------------------

  ECLGeometryPar* geometry = ECLGeometryPar::Instance();

  for (unsigned int iRelation = 0; iRelation < showerDigitRelations.size(); ++iRelation) {
    const auto calDigit = showerDigitRelations.object(iRelation);

    ProjectedECLDigit tmpProjectedDigit;

    //---------------------------------------------------------------------
    // Projected digit energy.
    //---------------------------------------------------------------------
    const auto weight = showerDigitRelations.weight(iRelation);
    tmpProjectedDigit.energy = weight * calDigit->getEnergy();

    //---------------------------------------------------------------------
    // Projected digit radial distance.
    //---------------------------------------------------------------------
    const int cellId = calDigit->getCellId();
    B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);

    // Angle between vector pointing to shower and vector pointing to CalDigit,
    //where the orgin is the detector origin (implicitly assuming IP = detector origin)
    const double angleDigitShower = calDigitPosition.Angle(showerPosition);
    tmpProjectedDigit.rho = showerR * TMath::Tan(angleDigitShower);

    //---------------------------------------------------------------------
    // Projected digit polar angle
    //---------------------------------------------------------------------
    // Vector perpendicular to the vector pointing to the shower position, pointing to the CalDigit.
    // It's length is not rho. Not normalized!!! We only care about the angle between in and xPrime.
    B2Vector3D projectedDigitDirection = calDigitPosition - calDigitPosition.Dot(showerDirection) * showerDirection;
    tmpProjectedDigit.alpha = projectedDigitDirection.Angle(xPrimeDirection);

    // adjust so that alpha spans 0..2pi
    if (projectedDigitDirection.Angle(yPrimeDirection) > TMath::Pi() / 2.0)
      tmpProjectedDigit.alpha = 2.0 * TMath::Pi() - tmpProjectedDigit.alpha;
    tmpProjectedECLDigits.push_back(tmpProjectedDigit);
  }

  return tmpProjectedECLDigits;
}

void ECLShowerShapeModule::endRun()
{
}

void ECLShowerShapeModule::terminate()
{
  if (m_neighbourMap9) delete m_neighbourMap9;
  if (m_neighbourMap21) delete m_neighbourMap21;
}

double ECLShowerShapeModule::computeLateralEnergy(const std::vector<ProjectedECLDigit>& projectedDigits,
                                                  const double avgCrystalDimension) const
{

//   auto relatedDigitsPairs = shower.getRelationsTo<ECLCalDigit>();
  if (projectedDigits.size() < 3.0) return 0;

  // Find the two projected digits with the maximum energy.
  double maxEnergy(0), secondMaxEnergy(0);
  unsigned int iMax(0), iSecondMax(0);

  for (unsigned int iProjecteDigit = 0; iProjecteDigit < projectedDigits.size(); iProjecteDigit++) {
    if (projectedDigits[iProjecteDigit].energy > maxEnergy) {
      secondMaxEnergy = maxEnergy;
      iSecondMax = iMax;
      maxEnergy = projectedDigits[iProjecteDigit].energy;
      iMax = iProjecteDigit;
    } else if (projectedDigits[iProjecteDigit].energy > secondMaxEnergy) {
      secondMaxEnergy = projectedDigits[iProjecteDigit].energy;
      iSecondMax = iProjecteDigit;
    }
  }

  //Calculate lateral energy
  double sumE = 0;
  for (unsigned int iProjecteDigit = 0; iProjecteDigit < projectedDigits.size(); iProjecteDigit++) {

    //2 highest energies are considered differently than the rest
    if (iProjecteDigit == iMax || iProjecteDigit == iSecondMax) continue;

    double rho = projectedDigits[iProjecteDigit].rho;
    double rho2 = rho * rho;
    double energy =  projectedDigits[iProjecteDigit].energy;
    sumE += energy * rho2;

  }

  const double r0sq = avgCrystalDimension * avgCrystalDimension; // average crystal dimension squared.
  return sumE / (sumE + r0sq * (maxEnergy + secondMaxEnergy));
}

double ECLShowerShapeModule::Rnm(const int n, const int m, const double rho) const
{
  // Some explicit polynomials.
  if (n == 1 && m == 1) return rho;
  if (n == 2 && m == 0) return 2.0 * rho * rho - 1.0;
  if (n == 2 && m == 2) return rho * rho;
  if (n == 3 && m == 1) return 3.0 * rho * rho * rho - 2.0 * rho;
  if (n == 3 && m == 3) return rho * rho * rho;
  if (n == 4 && m == 0) return 6.0 * rho * rho * rho * rho - 6.0 * rho * rho + 1.0;
  if (n == 4 && m == 2) return 4.0 * rho * rho * rho * rho - 3.0 * rho * rho;
  if (n == 4 && m == 4) return rho * rho * rho * rho;
  if (n == 5 && m == 1) return 10.0 * rho * rho * rho * rho * rho - 12.0 * rho * rho * rho + 3.0 * rho;
  if (n == 5 && m == 3) return 5.0 * rho * rho * rho * rho * rho - 4.0 * rho * rho * rho;
  if (n == 5 && m == 5) return rho * rho * rho * rho * rho;

  // Otherwise compute explicitely.
  double returnVal = 0;
  for (int idx = 0; idx <= (n - std::abs(m)) / 2; ++idx)
    returnVal += std::pow(-1, idx) * TMath::Factorial(n - idx) / TMath::Factorial(idx)
                 / TMath::Factorial((n + std::abs(m)) / 2 - idx) / TMath::Factorial((n - std::abs(m)) / 2 - idx) * std::pow(rho, n - 2 * idx);

  return returnVal;
}

std::complex<double> ECLShowerShapeModule::zernikeValue(const double rho, const double alpha, const int n, const int m) const
{
  // Zernike moment defined only on the unit cercile (rho < 1).
  if (rho > 1.0) return std::complex<double>(0, 0);

  std::complex<double> i(0, 1);
  std::complex<double> exponent = std::exp(i * std::complex<double>(m * alpha, 0));
  return std::complex<double>(Rnm(n, m, rho), 0) * exponent;
}


double ECLShowerShapeModule::computeAbsZernikeMoment(const std::vector<ProjectedECLDigit>& projectedDigits,
                                                     const double totalEnergy, const int n, const int m, const double rho0) const
{
  if (totalEnergy <= 0.0) return 0.0;

  // Make sure n,m are valid
  if (n < 0 || m < 0) return 0.0;
  if (m > n) return 0.0;

  std::complex<double> sum(0.0, 0.0);

  for (const auto projectedDigit : projectedDigits) {
    double normalizedRho = projectedDigit.rho / rho0;     // Normalize radial distance according to rho0.
    //Ignore crystals with rho > rho0, if requested
    if (normalizedRho > 1.0) {
      if (!m_zernike_useFarCrystals) continue;
      else normalizedRho = 1.0; //crystals with rho > rho0 are scaled to rho0 instead of discarded
    }

    sum += projectedDigit.energy * std::conj(zernikeValue(normalizedRho, projectedDigit.alpha, n, m));
  }
  return (n + 1.0) / TMath::Pi() * std::abs(sum) / totalEnergy;
}

double ECLShowerShapeModule::computeSecondMoment(const std::vector<ProjectedECLDigit>& projectedDigits,
                                                 const double totalEnergy) const
{
  if (totalEnergy <= 0.0) return 0.0;

  double sum = 0.0;

  for (const auto projectedDigit : projectedDigits) sum += projectedDigit.energy * projectedDigit.rho * projectedDigit.rho;

  return sum / totalEnergy;
}


double ECLShowerShapeModule::computeE1oE9(const ECLShower& shower) const
{

  // get central id
  const int centralCellId = shower.getCentralCellId();
  if (centralCellId == 0) return 0.0; //cell id starts at 1

  // get list of 9 neighbour ids
  const std::vector< short int > n9 = m_neighbourMap9->getNeighbours(centralCellId);

  double energy1 = 0.0; // to check: 'highest energy' data member may not always be the right one
  double energy9 = 0.0;

  auto relatedDigitsPairs = shower.getRelationsTo<ECLCalDigit>(eclCalDigitArrayName());

  for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
    const auto caldigit = relatedDigitsPairs.object(iRel);
    const auto weight = relatedDigitsPairs.weight(iRel);
    const auto energy = caldigit->getEnergy();
    const int cellid  = caldigit->getCellId();

    // get central cell id energy
    if (cellid == centralCellId) {
      energy1 = weight * energy;
    }

    // check if this is contained in the 9 neighbours
    const auto it9 = std::find(n9.begin(), n9.end(), cellid);
    if (it9 != n9.end()) {
      energy9 += weight * energy;
    }

  }

  if (energy9 > 1e-9) return energy1 / energy9;
  else return 0.0;
}

double ECLShowerShapeModule::computeE9oE21(const ECLShower& shower) const
{
  // get central id
  const int centralCellId = shower.getCentralCellId();
  if (centralCellId == 0) return 0.0; //cell id starts at 1

  // get list of 9 and 21 neighbour ids
  const std::vector< short int > n9 = m_neighbourMap9->getNeighbours(centralCellId);
  const std::vector< short int > n21 = m_neighbourMap21->getNeighbours(centralCellId);

  double energy9 = 0.0;
  double energy21 = 0.0;

  auto relatedDigitsPairs = shower.getRelationsTo<ECLCalDigit>(eclCalDigitArrayName());

  for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
    const auto caldigit = relatedDigitsPairs.object(iRel);
    const auto weight = relatedDigitsPairs.weight(iRel);
    const auto energy = caldigit->getEnergy();
    const int cellid  = caldigit->getCellId();

    // check if this is contained in the 9 neighbours
    const auto it9 = std::find(n9.begin(), n9.end(), cellid);
    if (it9 != n9.end()) {
      energy9 += weight * energy;
    }

    // check if this is contained in the 21 neighbours
    const auto it21 = std::find(n21.begin(), n21.end(), cellid);
    if (it21 != n21.end()) {
      energy21 += weight * energy;
    }

  }

  if (energy21 > 1e-9) return energy9 / energy21;
  else return 0.0;

}

void ECLShowerShapeModule::prepareSecondMomentCorrectionsCallback()
{
  //Clear m_secondMomentCorrections array
  for (auto iType = 0; iType < 2; ++iType)
    for (auto iHypothesis = 0; iHypothesis < 10; ++iHypothesis)
      m_secondMomentCorrections[iType][iHypothesis] = TGraph();

  // Read all corrections.
  for (const ECLShowerShapeSecondMomentCorrection& correction : m_secondMomentCorrectionArray) {
    const int type  = correction.getType();
    const int hypothesis  = correction.getHypothesisId();
    if (type < 0 or type > 1 or hypothesis < 1 or hypothesis > 9) {
      B2FATAL("Invalid type or hypothesis for second moment corrections.");
    }

    m_secondMomentCorrections[type][hypothesis] = correction.getCorrection();
  }

  //   Check that all corrections are there
  if (m_secondMomentCorrections[c_thetaType][ECLCluster::c_nPhotons].GetN() == 0 or
      m_secondMomentCorrections[c_phiType][ECLCluster::c_nPhotons].GetN() == 0 or
      m_secondMomentCorrections[c_thetaType][ECLCluster::c_neutralHadron].GetN() == 0 or
      m_secondMomentCorrections[c_phiType][ECLCluster::c_neutralHadron].GetN() == 0) {
    B2FATAL("Missing corrections for second moments..");
  }
}

double ECLShowerShapeModule::getSecondMomentCorrection(const double theta, const double phi, const int hypothesis) const
{
  // convert to deg.
  double thetadeg = theta * TMath::RadToDeg();

  // protect angular range.
  if (thetadeg < 0.1) thetadeg = 0.1;
  else if (thetadeg > 179.9) thetadeg = 179.9;

  //Convert phi
  double phideg = phi * TMath::RadToDeg();

  //Protect phi
  if (phideg < -179.9) phideg = -179.9;
  else if (phideg > 179.9) phideg = 179.9;


  // protect hypothesis.
  if (hypothesis < 1 or hypothesis > 10) {
    B2FATAL("Invalid hypothesis for second moment corrections.");
  }

  const double thetaCorrection = m_secondMomentCorrections[c_thetaType][hypothesis].Eval(thetadeg);
  const double phiCorrection = m_secondMomentCorrections[c_phiType][hypothesis].Eval(phideg);

  B2DEBUG(175, "Second momen theta crrection = " << thetaCorrection << ", phi correction = " << phiCorrection);

  return thetaCorrection * phiCorrection;
}
