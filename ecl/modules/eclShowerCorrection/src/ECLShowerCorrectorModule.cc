/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module performs the correction for EM shower (mainly longitudinal *
 * leakage): corr = (Reconstructed / Truth).                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *               Gulgilemo De Nardo (denardo@na.infn.it) (GDN)            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclShowerCorrection/ECLShowerCorrectorModule.h>

// FRAMEWORK
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/gearbox/GearDir.h>

// ECL
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/utility/ECLShowerId.h>
#include <ecl/digitization/EclConfiguration.h> // this should not be in digitization but in a more general location (TF)


// OTHER
#include <vector>
#include <fstream>      // std::ifstream


using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLShowerCorrector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLShowerCorrectorModule::ECLShowerCorrectorModule() : Module()
{
  // Set description
  setDescription("ECLShowerCorrectorModule: Corrects for MC truth to reconstruction differences");
  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLShowerCorrectorModule::~ECLShowerCorrectorModule()
{
}

void ECLShowerCorrectorModule::initialize()
{
  // Register in datastore
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());
  StoreArray<ECLConnectedRegion> eclCRs(eclConnectedRegionArrayName());

  eclShowers.registerInDataStore();
  eclCRs.registerInDataStore();

  // Read correction accounting shower leakage to get unbiased photon energy
  ReadCorrection();

  std::string tmpCorrectionsFileName;
  if (EclConfiguration::get().background())
    tmpCorrectionsFileName = FileSystem::findFile("/data/ecl/tmpClusterCorrection-BG.txt");
  else
    tmpCorrectionsFileName = FileSystem::findFile("/data/ecl/tmpClusterCorrection.txt");
  assert(! tmpCorrectionsFileName.empty());
  m_tmpClusterCorrection.init(tmpCorrectionsFileName);

}

void ECLShowerCorrectorModule::beginRun()
{
  // Do not use this for Database updates, they will not follow the concept of a "run"
  ;
}

void ECLShowerCorrectorModule::event()
{
  // input array
  StoreArray<ECLConnectedRegion> eclCRs(eclConnectedRegionArrayName());
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());

  // utility function to unpack the showerid
  ECLShowerId SID;

  // loop over all ECLShowers
  for (auto& eclShower : eclShowers) {

    // example how to get the hypothesis of the eclShower
    //const int hypothesis = SID.getHypothesis(eclShower.getUniqueShowerId());
    //const int cr = SID.getCRID(eclShower.getUniqueShowerId());
    //    const int seed = SID.getSeed(eclShower.getUniqueShowerId());
//    B2INFO("ECLShowerCorrectorModule: hypothesis = " << hypothesis);
//    B2INFO("ECLShowerCorrectorModule: cr = " << cr);
//    B2INFO("ECLShowerCorrectorModule: seed = " << seed << "\n");

    // first correction step (OLD METHOD BY GDN) (TF)
    // correction for the shower
    const double uncorrectedEnergy = eclShower.getEnergy();
    const double theta             = eclShower.getTheta();
    const double correctionF       = correctionFactor(uncorrectedEnergy, theta);

    if (correctionF < 1e-12) {
      B2WARNING("ECLShowerCorrectorModule:  << correction factor is (almost) zero: " << correctionF);
    }

    const double correctedEnergy = uncorrectedEnergy / correctionF;
    eclShower.setEnergy(correctedEnergy);

    // correction for the highest energetic crystal in the shower (to be studied!) (TF)
    const double uncorrectedHighestEnergy = eclShower.getEnergyHighestCrystal();
    const double correctedHighestEnergy = uncorrectedHighestEnergy / correctionF;
    eclShower.setEnergyHighestCrystal(correctedHighestEnergy);

    // second correction step (OLD METHOD BY GDN) (TF)
    m_tmpClusterCorrection.scale(eclShower);
  }

}

void ECLShowerCorrectorModule::endRun()
{
  ;
}

void ECLShowerCorrectorModule::terminate()
{
  ;
}

double ECLShowerCorrectorModule::correctionFactor(double Energy, double Theta)
{
  // Need check if energy is (almost) zero during clustering development
  if (Energy < 1e-9) return 1.0;

  std::vector<double>::const_iterator it = upper_bound(m_ranges.begin(), m_ranges.end(), Theta);
  double x = log(Energy);
  // not really necessary check
  // if(it==m_ranges.begin()||it==m_ranges.end()){
  //   B2ERROR("Theta is out of range! Theta = "<<Theta<<" Theta_min = "<<m_ranges.front()<<" Theta_max = "<<m_ranges.back());
  //   return 1.0;
  // }
  double* c = &m_ecorr[0] + 4 * ((it - m_ranges.begin()) - 1);
  return c[0] + x * (c[1] + x * (c[2] + x * (c[3])));
}


void ECLShowerCorrectorModule::ReadCorrection()
{
  /*Register the ECL.xml file, which further register the
   ECL-FirstCorrection.xml file and use GearDir to access the elements*/
  std::string path("Detector/DetectorComponent[@name=\"ECL\"]/Content/ECLFirstCorrection/ThetaBins/");
  GearDir ThetaBins(path);

  // The number of bins the detector is divided into
  const int Nbins = ThetaBins.getNumberNodes("ThetaBins");
  m_ecorr.resize(Nbins * 4); // 4 is a polynomial degree

  std::vector<std::pair<double, double> > ranges;
  ranges.reserve(Nbins);
  std::string p("Polynomial");
  for (int i = 0; i < Nbins; i++) { // We assumed here theta ranges is in ascending order
    GearDir T(path + "/ThetaBins[" + std::to_string(i + 1) + "]");
    ranges.push_back(std::pair<double, double>(T.getAngle("ThetaStart"), T.getAngle("ThetaEnd")));
    for (int j = 0; j < 4; j++) {
      m_ecorr[4 * i + j] = T.getDouble(p + std::to_string(j + 1));
    }
  }

  m_ranges.resize(Nbins + 1);
  m_ranges[0] = ranges[0].first; // to detect out of range case
  for (int i = 0; i < Nbins - 1; i++) {
    if (ranges[i].second != ranges[i + 1].first) // Sanity check
      B2ERROR("Energy correction is not continuous in theta range! Bin = " << i << " " << ranges[i].second << " " << ranges[i + 1].first);
    m_ranges[i + 1] = ranges[i].second;
  }
  m_ranges[Nbins] = ranges[Nbins - 1].second;

  if (std::abs(m_ranges[0]) > 1e-7 || std::abs(m_ranges[Nbins] - M_PI) > 1e-7) // Sanity check
    B2ERROR("Energy correction does not cover full theta range! Theta_min = " << m_ranges[0] << " Theta_max = " << m_ranges[Nbins]);
}


void ECLShowerCorrectorModule::TmpClusterCorrection::init(const std::string& filename)
{
//  B2INFO("ECLReconstructor: Reading ad hoc tmp cluster energy corrections from: " << filename);
  B2INFO("ECLShowerCorrectorModule: Reading ad hoc tmp shower energy corrections from: " << filename); // (TF)
  // header keeps the values of 3 parameters in any order
  std::string param;
  std::ifstream file(filename);
  int nbinsTheta = 0;
  for (int i = 0; i < 3; ++i) {
    file >> param;
    assert(file.good());
    if (param == "deltaE") {
      file >> m_deltaE;
      assert(file.good());
    }
    if (param == "npointsE") {
      file >> m_npointsE;
      assert(file.good());
    }
    if (param == "thetaRegions") {
      file >> nbinsTheta;
      assert(file.good());
      double theta;
      for (int j = 0; j < nbinsTheta - 1; ++j) {
        file >> theta ;
        assert(file.good());
        m_maxTheta.push_back(theta);
      }
    }
  }

  size_t ncoeffE{m_npointsE + 2};
  m_tmpCorrection.reserve(ncoeffE * nbinsTheta);
  double correction;
  // the index runs first on momentum and then on theta region
  for (size_t i = 0; i < ncoeffE * (m_maxTheta.size() + 1); ++i) {
    file >> correction;
    assert(file.good());
    m_tmpCorrection.push_back(correction);
  }
  file.close();

  B2INFO("ECLShowerCorrectorModule: Number of points for interpolation in E: " << m_npointsE);
  B2INFO("ECLShowerCorrectorModule: Number of Theta regions: " << nbinsTheta);

  for (int i = 0; i < nbinsTheta; ++i) {
    if (i < nbinsTheta - 1) {
      B2INFO("ECLShowerCorrectorModule: theta < " << m_maxTheta[i]);
    } else {
      B2INFO("ECLShowerCorrectorModule: theta >= " << m_maxTheta[i - 1]);
    }
    std::ostringstream ostr;
    for (size_t j = 0; j < ncoeffE; j++)
      ostr << m_tmpCorrection[ j + i * (ncoeffE) ] << " ";

    std::string out = ostr.str();
    B2INFO("ECLShowerCorrectorModule: corrections: " << out);
  }
}

void ECLShowerCorrectorModule::TmpClusterCorrection::scale(ECLShower& c) const
{
  double energy = c.getEnergy();
  if (energy > 0) {
    int thetaRegion = m_maxTheta.size();
    for (size_t itheta = 0; itheta < m_maxTheta.size(); itheta++) {
      if (c.getTheta() < m_maxTheta[itheta]) {
        thetaRegion = itheta;
        break;
      }
    }
    size_t ncoeffE{ m_npointsE + 2 };
    int offset = thetaRegion * ncoeffE;
    unsigned int iE = static_cast<unsigned int>(energy / m_deltaE);
    double corr{0};
    if (iE == 0) {
      double halfdeltaE = m_deltaE / 2;
      if (energy < halfdeltaE)
        corr = m_tmpCorrection[ offset ];
      else {
        double incE{ energy - halfdeltaE };
        const double& clow { m_tmpCorrection[offset] };
        const double& chigh{ m_tmpCorrection[offset + 1 ] };
        corr = clow + incE / halfdeltaE * (chigh - clow);
      }
    } else if (iE < m_npointsE - 1) {
      double incE{ energy - iE * m_deltaE };
      double clow{ m_tmpCorrection[offset + iE ] };
      double chigh{ m_tmpCorrection[offset + iE + 1 ] };
      corr = clow + incE / m_deltaE * (chigh - clow);
    } else
      corr =  m_tmpCorrection[ offset + m_npointsE + 1 ];

    c.setEnergy(energy * corr);
  }
}

