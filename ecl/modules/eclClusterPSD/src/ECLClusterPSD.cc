/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 *   This module computes shower variables using pulse shape              *
 *   information from offline two component fits.  Using pulse            *
 *   shape discrimination, these shower variables can be used             *
 *   for particle id.                                                     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Savino Longo (longos@uvic.ca)                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
using namespace ECL;

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
  addParam("Chi2Threshold", m_Chi2Threshold, "Chi2 Threshold", 60.);
  addParam("CrystalHadronEnergyThreshold", m_CrystalHadronEnergyThreshold,
           "Hadron component energy threshold to identify as hadron digit.(GeV)", 0.003);
  addParam("CrystalHadronIntensityThreshold", m_CrystalHadronIntensityThreshold,
           "Hadron component intensity threshold to identify as hadron digit.", 0.005);
  addParam("MVAidentifier", m_MVAidentifier, "MVA database identifier.", std::string{"DigitMVA_s3_BDT_D20XYRWEonEoffNhFT"});
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

// evaluates PSD mva
double ECLClusterPSDModule::evaluatePSDmva(const ECLShower* cluster)
{

  //geometry for cell id position
  ECLGeometryPar* geometry = ECLGeometryPar::Instance();

  auto relatedDigits = cluster->getRelationsTo<ECLCalDigit>();

  //EnergyToSort vector is used for sorting digits by offline two component energy
  std::vector< std::vector<double> > EnergyToSort;

  for (unsigned int iRel = 0; iRel < relatedDigits.size(); iRel++) {

    const auto caldigit = relatedDigits.object(iRel);

    const double digitChi2 = caldigit->getTwoComponentChi2();

    std::vector<double> temp(2);
    temp[0] = 0;
    temp[1] = (double)iRel;

    //only digits with offline waveform and offline fit chi2 was good (chi2<m_Chi2Threshold)
    if (digitChi2 > 0 && digitChi2 < m_Chi2Threshold) {
      const double digitEnergy = caldigit->getTwoComponentTotalEnergy();
      temp[0] = digitEnergy;
    }

    EnergyToSort.push_back(temp);

  }

  //sorting by energy
  std::sort(EnergyToSort.begin(), EnergyToSort.end(), std::greater<>());

  //get cluster position information
  const double showerR = cluster->getR();
  const double showerTheta = cluster->getTheta();
  const double showerPhi = cluster->getPhi();

  B2Vector3D showerPosition;
  showerPosition.SetMagThetaPhi(showerR, showerTheta, showerPhi);

  //mva can include up to 20 input digits. Note input digits must have offline waveform.
  const int maxdigits = 20;

  //vectors below are used to organize the digit quantities to be used as inputs for the mva.  Sorted by offline energy.
  std::vector<double>  Eon(maxdigits); //One photon template fit energy
  std::vector<double>  Eoff(maxdigits); //Offline two component fit energy
  std::vector<double>  Rdist(maxdigits); //Distance from cluster centre to crystal centre
  std::vector<double>  The(maxdigits); //cos theta of vector pointing to from cluster centre to crystal centre
  std::vector<double>  Phi(maxdigits); //cos phi of vector pointing to from cluster centre to crystal centre
  std::vector<double>  Nh(maxdigits); //hadron component intensity from offline two component fit
  std::vector<double>  Ft(maxdigits); //offline fit type
  std::vector<double>  Weight(maxdigits); //digit weight from clustering

  for (unsigned int i = 0; i < maxdigits; i++) {

    //initalizing entries. Extra digits are set to 0
    Eon[i] = 0;
    Eoff[i] = 0;
    Rdist[i] = 0;
    The[i] = 0;
    Phi[i] = 0;
    Nh[i] = 0;
    Ft[i] = -1;
    Weight[i] = 0;

    if (i < EnergyToSort.size()) {

      if (EnergyToSort[i][0] > 0) {

        const unsigned int next = EnergyToSort[i][1]; //index of next highest energy digit
        const auto caldigit = relatedDigits.object(next);
        const double digitEnergy = caldigit->getTwoComponentTotalEnergy();
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
        Eon[i] = digitOnlineEnergy;
        Eoff[i] = digitEnergy;
        Rdist[i] = Rval;
        The[i] = theVal;
        Phi[i] = phiVal;
        Nh[i] = (digitHadronEnergy / digitEnergy);
        Ft[i] = digitFitType;
        Weight[i] = digitWeight;
      }
    }
  }

  //use vectors filled above to input data into mva dataset.  20 digits x 8 variables = 160 inputs.
  for (int i = 0; i < maxdigits; i++) {
    m_dataset->m_input[(i * 8) + 0] = The[i];
    m_dataset->m_input[(i * 8) + 1] = Phi[i];
    m_dataset->m_input[(i * 8) + 2] = Rdist[i];
    m_dataset->m_input[(i * 8) + 3] = Eon[i];
    m_dataset->m_input[(i * 8) + 4] = Eoff[i];
    m_dataset->m_input[(i * 8) + 5] = Nh[i];
    m_dataset->m_input[(i * 8) + 6] = Ft[i];
    m_dataset->m_input[(i * 8) + 7] = Weight[i];
  }

  //compute mva from input variables
  const double MVAout = m_expert->apply(*m_dataset)[0];

  return MVAout;
}


void ECLClusterPSDModule::event()
{

  for (auto& shower : m_eclShowers) {

    //evaluates mva classifier
    const double mvaout = evaluatePSDmva(&shower);

    auto relatedDigits = shower.getRelationsTo<ECLCalDigit>();

    double numberofHadronDigits = 0;
    double nWaveforminCluster = 0;

    for (unsigned int iRel = 0; iRel < relatedDigits.size(); iRel++) {

      const auto weight = relatedDigits.weight(iRel);

      const auto caldigit = relatedDigits.object(iRel);
      const double digit2CChi2 = caldigit->getTwoComponentChi2();

      if (digit2CChi2 < 0)  continue; //only digits with waveforms

      //ECLDsp::TwoComponentFitType digitFitType = caldigit->getTwoComponentFitType();

      //if (digitFitType != ECLDsp::photonHadron)  continue; //only standard fits

      if (digit2CChi2 < m_Chi2Threshold) { //must be a good fit

        const double digit2CTotalEnergy = caldigit->getTwoComponentTotalEnergy();
        const double digit2CHadronComponentEnergy = caldigit->getTwoComponentHadronEnergy();

        if (digit2CTotalEnergy < 0.6) {
          if (digit2CHadronComponentEnergy > m_CrystalHadronEnergyThreshold)  numberofHadronDigits += weight;
        } else {
          const double digitHadronComponentIntensity = digit2CHadronComponentEnergy / digit2CTotalEnergy;
          if (digitHadronComponentIntensity > m_CrystalHadronIntensityThreshold)  numberofHadronDigits += weight;
        }
        nWaveforminCluster += weight;
      }
    }

    shower.setPulseShapeDiscriminationMVA(mvaout);
    if (nWaveforminCluster > 0) {
      shower.setNumberOfHadronDigits(numberofHadronDigits);
      shower.addStatus(ECLShower::c_hasPulseShapeDiscrimination);
    } else {
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
