/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//This module
#include <ecl/modules/eclClusterPSD/ECLClusterPSD.h>

//BOOST
#include <boost/algorithm/string/predicate.hpp>

// ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/geometry/ECLGeometryPar.h>

//MVA
#include <mva/interface/Expert.h>
#include <mva/interface/Weightfile.h>
#include <mva/interface/Interface.h>
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>

#include<math.h>

// FRAMEWORK
#include <framework/logging/Logger.h>
#include <framework/geometry/B2Vector3.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(ECLClusterPSD)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
ECLClusterPSDModule::ECLClusterPSDModule()
{
  // Set module properties
  setDescription("Module uses offline two component fit results to compute pulse shape discrimation variables for particle identification.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("CrystalHadronEnergyThreshold", m_CrystalHadronEnergyThreshold,
           "Hadron component energy threshold to identify as hadron digit.(GeV)", 0.003);
  addParam("CrystalHadronIntensityThreshold", m_CrystalHadronIntensityThreshold,
           "Hadron component intensity threshold to identify as hadron digit.", 0.005);
  addParam("MVAidentifier", m_MVAidentifier, "MVA database identifier.", std::string{"eclClusterPSD_MVA"});
}

// destructor
ECLClusterPSDModule::~ECLClusterPSDModule()
{
}

// initialize MVA weightFile
void ECLClusterPSDModule::initializeMVAweightFile(const std::string& identifier,
                                                  std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>& weightFileRepresentation)
{
  if (not(boost::ends_with(identifier, ".root") or boost::ends_with(identifier, ".xml"))) {
    weightFileRepresentation = std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>(new
                               DBObjPtr<DatabaseRepresentationOfWeightfile>(identifier));
  }
  MVA::AbstractInterface::initSupportedInterfaces();
}

// initialize
void ECLClusterPSDModule::initialize()
{
  // ECL dataobjects
  m_eclShowers.registerInDataStore(eclShowerArrayName());
  m_eclCalDigits.registerInDataStore(eclCalDigitArrayName());
  initializeMVAweightFile(m_MVAidentifier, m_weightfile_representation);
}

// initialize MVA
void ECLClusterPSDModule::initializeMVA(const std::string& identifier,
                                        std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>& weightFileRepresentation, std::unique_ptr<MVA::Expert>& expert)
{
  MVA::Weightfile  weightfile;
  //Load MVA weight file
  if (weightFileRepresentation) {

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
  if (m_numMVAvariables != general_options.m_variables.size())
    B2FATAL("Expecting " << m_numMVAvariables << " variables, found " << general_options.m_variables.size());

  expert = supported_interfaces[general_options.m_method]->getExpert();
  expert->load(weightfile);

  //create new dataset
  if (weightFileRepresentation == m_weightfile_representation) {
    std::vector<float> dummy(general_options.m_variables.size(), 0);
    m_dataset = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(general_options, dummy, 0));
  }
}

// begin run
void ECLClusterPSDModule::beginRun()
{
  initializeMVA(m_MVAidentifier, m_weightfile_representation, m_expert);
}

// evaluates mva
double ECLClusterPSDModule::evaluateMVA(const ECLShower* cluster)
{

  //geometry for cell id position
  ECL::ECLGeometryPar* geometry = ECL::ECLGeometryPar::Instance();

  auto relatedDigits = cluster->getRelationsTo<ECLCalDigit>();

  //EnergyToSort vector is used for sorting digits by offline two component energy
  std::vector<std::tuple<double, unsigned int>> EnergyToSort;

  for (unsigned int iRel = 0; iRel < relatedDigits.size(); iRel++) {

    const auto caldigit = relatedDigits.object(iRel);

    //exclude digits without waveforms
    const double digitChi2 = caldigit->getTwoComponentChi2();
    if (digitChi2 < 0)  continue;

    ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();

    //exclude digits digits with poor chi2
    if (digitFitType1 == ECLDsp::poorChi2) continue;

    //exclude digits with diode-crossing fits
    if (digitFitType1 == ECLDsp::photonDiodeCrossing) continue;

    EnergyToSort.emplace_back(caldigit->getTwoComponentTotalEnergy(), iRel);

  }

  //sorting by energy
  std::sort(EnergyToSort.begin(), EnergyToSort.end(), std::greater<>());

  //get cluster position information
  const double showerR = cluster->getR();
  const double showerTheta = cluster->getTheta();
  const double showerPhi = cluster->getPhi();

  B2Vector3D showerPosition;
  showerPosition.SetMagThetaPhi(showerR, showerTheta, showerPhi);

  size_t input_index{0};
  auto& input = m_dataset->m_input;

  for (unsigned int digit = 0; digit < maxdigits; ++digit) {

    if (digit >= EnergyToSort.size()) break;

    const auto [digitEnergy, next] = EnergyToSort[digit];

    const auto caldigit = relatedDigits.object(next);
    const double digitHadronEnergy = caldigit->getTwoComponentHadronEnergy();
    const double digitOnlineEnergy = caldigit->getEnergy();
    const double digitWeight = relatedDigits.weight(next);
    ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
    const int digitFitType = digitFitType1;
    const int cellId = caldigit->getCellId();
    B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
    TVector3 tempP = showerPosition - calDigitPosition;
    const double Rval = tempP.Mag();
    const double theVal = tempP.CosTheta();
    const double phiVal = cos(tempP.Phi());

    input[input_index++] = theVal;
    input[input_index++] = phiVal;
    input[input_index++] = Rval;
    input[input_index++] = digitOnlineEnergy;
    input[input_index++] = digitEnergy;
    input[input_index++] = (digitHadronEnergy / digitEnergy);
    input[input_index++] = digitFitType;
    input[input_index++] = digitWeight;

  }

  //fill remainder with defaults
  while (input_index < input.size()) {
    if (((input_index - 6) % 8) != 0) {
      input[input_index++] = 0.0;
    } else {
      input[input_index++] = -1.0;  //Fit Type
    }
  }

  //compute mva from input variables
  const double MVAout = m_expert->apply(*m_dataset)[0];

  return MVAout;
}


void ECLClusterPSDModule::event()
{

  for (auto& shower : m_eclShowers) {


    auto relatedDigits = shower.getRelationsTo<ECLCalDigit>();

    double cluster2CTotalEnergy = 0;
    double cluster2CHadronEnergy = 0;
    double numberofHadronDigits = 0;
    double nWaveforminCluster = 0;

    for (unsigned int iRel = 0; iRel < relatedDigits.size(); iRel++) {

      const auto weight = relatedDigits.weight(iRel);

      const auto caldigit = relatedDigits.object(iRel);
      const double digit2CChi2 = caldigit->getTwoComponentChi2();

      if (digit2CChi2 < 0)  continue; //only digits with waveforms

      ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();

      //exclude digits digits with poor chi2
      if (digitFitType1 == ECLDsp::poorChi2) continue;

      //exclude digits with diode-crossing fits
      if (digitFitType1 == ECLDsp::photonDiodeCrossing) continue;

      const double digit2CTotalEnergy = caldigit->getTwoComponentTotalEnergy();
      const double digit2CHadronComponentEnergy = caldigit->getTwoComponentHadronEnergy();

      cluster2CTotalEnergy += digit2CTotalEnergy;
      cluster2CHadronEnergy += digit2CHadronComponentEnergy;

      if (digit2CTotalEnergy < 0.6) {
        if (digit2CHadronComponentEnergy > m_CrystalHadronEnergyThreshold)  numberofHadronDigits += weight;
      } else {
        const double digitHadronComponentIntensity = digit2CHadronComponentEnergy / digit2CTotalEnergy;
        if (digitHadronComponentIntensity > m_CrystalHadronIntensityThreshold)  numberofHadronDigits += weight;
      }

      nWaveforminCluster += weight;

    }

    if (nWaveforminCluster > 0) {
      if (cluster2CTotalEnergy != 0) shower.setShowerHadronIntensity(cluster2CHadronEnergy / cluster2CTotalEnergy);
      //evaluates mva classifier only if waveforms are available in the cluster
      const double mvaout = evaluateMVA(&shower);
      shower.setPulseShapeDiscriminationMVA(mvaout);

      shower.setNumberOfHadronDigits(numberofHadronDigits);
      shower.addStatus(ECLShower::c_hasPulseShapeDiscrimination);

    } else {
      shower.setShowerHadronIntensity(0);
      shower.setPulseShapeDiscriminationMVA(0.5);
      shower.setNumberOfHadronDigits(0);
    }
  }
}

// end run
void ECLClusterPSDModule::endRun()
{
}

// terminate
void ECLClusterPSDModule::terminate()
{
}
