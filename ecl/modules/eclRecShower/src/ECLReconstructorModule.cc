/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen, Vishal                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclRecShower/ECLReconstructorModule.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLHitAssignment.h>

#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/rec_lib/TEclCFCR.h>
#include <ecl/rec_lib/TRecEclCF.h>
#include <ecl/rec_lib/TRecEclCFParameters.h>
#include <ecl/rec_lib/TEclCFShower.h>
#include <ecl/rec_lib/TRecEclCF.h>
#include <ecl/digitization/EclConfiguration.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/utilities/FileSystem.h>

#include <boost/foreach.hpp>
#include <G4ParticleTable.hh>

#include<ctime>
#include <iomanip>

// ROOT
#include <TVector3.h>
#include <TMatrixFSym.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLReconstructor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLReconstructorModule::ECLReconstructorModule() : Module()
{
  //Set module properties
  setDescription("Creates ECLShower from ECLDigi and ECLCluster from ECLShower and Relations among them.");
  setPropertyFlags(c_ParallelProcessingCertified);
}


ECLReconstructorModule::~ECLReconstructorModule()
{
}

void ECLReconstructorModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // CPU time start
  m_timeCPU = clock() * Unit::us;

  // ECL relevant objects.
  StoreArray<ECLHitAssignment> eclHitAssignments;
  StoreArray<ECLShower> eclShowers;
  StoreArray<ECLCluster> eclClusters;

  eclHitAssignments.registerInDataStore();
  eclShowers.registerInDataStore();
  eclClusters.registerInDataStore();
  eclClusters.registerRelationTo(eclShowers);

  ReadCorrection(); // read correction accounting shower leakage to get unbiased photon energy

  string tmpCorrectionsFileName;
  if (EclConfiguration::get().background())
    tmpCorrectionsFileName = FileSystem::findFile("/data/ecl/tmpClusterCorrection-BG.txt");
  else
    tmpCorrectionsFileName = FileSystem::findFile("/data/ecl/tmpClusterCorrection.txt");
  assert(! tmpCorrectionsFileName.empty());
  m_tmpClusterCorrection.init(tmpCorrectionsFileName);
}

void ECLReconstructorModule::ReadCorrection()
{
  /*Register the ECL.xml file, which further register the
    ECL-FirstCorrection.xml file and use GearDir to access the elements*/
  string path("Detector/DetectorComponent[@name=\"ECL\"]/Content/ECLFirstCorrection/ThetaBins/");
  GearDir ThetaBins(path);

  // The number of bins the detector is divided into
  const int Nbins = ThetaBins.getNumberNodes("ThetaBins");
  m_ecorr.resize(Nbins * 4); // 4 is a polynomial degree

  vector<pair<double, double> > ranges;
  ranges.reserve(Nbins);
  string p("Polynomial");
  for (int i = 0; i < Nbins; i++) { // We assumed here theta ranges is in ascending order
    GearDir T(path + "/ThetaBins[" + to_string(i + 1) + "]");
    ranges.push_back(pair<double, double>(T.getAngle("ThetaStart"), T.getAngle("ThetaEnd")));
    for (int j = 0; j < 4; j++) {
      m_ecorr[4 * i + j] = T.getDouble(p + to_string(j + 1));
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

  if (abs(m_ranges[0]) > 1e-7 || abs(m_ranges[Nbins] - M_PI) > 1e-7) // Sanity check
    B2ERROR("Energy correction does not cover full theta range! Theta_min = " << m_ranges[0] << " Theta_max = " << m_ranges[Nbins]);
}

void ECLReconstructorModule::beginRun()
{
  B2INFO("ECLReconstructorModule: Processing run: " << m_nRun);
}


void ECLReconstructorModule::event()
{
  // Input Array
  StoreArray<ECLDigit> eclDigiArray;
  if (!eclDigiArray) {
    B2DEBUG(100, "ECLDigit in empty in event " << m_nEvent);
    return;
  }
  // Output Arrays
  StoreArray<ECLHitAssignment> eclHitAssignments;
  StoreArray<ECLShower> eclShowers;
  StoreArray<ECLCluster> eclClusters;

  cout.unsetf(ios::scientific);
  cout.precision(6);
  TRecEclCF& cf = TRecEclCF::Instance();
  cf.Clear();

  for (const auto& aECLDigi : eclDigiArray) {
    const double FitEnergy = (aECLDigi.getAmp()) / 20000.; // ADC count to GeV
    //float FitTime = (1520 - aECLDigi->getTimeFit())*24.*12/508/(3072/2) ;
    //in us -> obsolete, for record. 20150529 KM.

    const int cId = (aECLDigi.getCellId() - 1);
    if (FitEnergy < 0.) {continue;}
    // Register hit to cluster finder, cf.
    cf.Accumulate(m_nEvent, FitEnergy, cId);
  }

  // The cluster finder is called.
  cf.SearchCRs();
  cf.Attributes();

  int nShower = 0;

  // Loop over Connected Region (CR).
  for (const auto& cr : cf.CRs()) {
    // Each Shower is pointed from the CR.
    for (const auto& shower : cr.Showers()) {
      TEclCFShower iSh = shower.second;

      // In order for finding the highest energy crystal in a shower and
      // that crystals' timing. 20150529 K.Miyabayashi
      double v_HiEnergy = -9999999;
      double v_TIME = -10;

      std::vector<MEclCFShowerHA> HAs = iSh.HitAssignment();
      for (std::vector<MEclCFShowerHA>::iterator iHA = HAs.begin();
           iHA != HAs.end(); ++iHA) {

        if (!eclHitAssignments) eclHitAssignments.create();
        const auto eclHitAssignment = eclHitAssignments.appendNew();
        eclHitAssignment->setShowerId(nShower);
        eclHitAssignment->setCellId(iHA->Id() + 1);

        // From ECLDigit information, the highest energy crystal is found.
        // To speed up, Mapping to be used, by Vishal
        for (const auto& aECLDigi : eclDigiArray) {
          if (aECLDigi.getCellId() != (iHA->Id() + 1)) continue;
          const double v_FitEnergy    = (aECLDigi.getAmp()) / 20000.;
          const double v_FitTime      =  aECLDigi.getTimeFit();
          // If the crystal has highest energy, keep its information.
          if (v_HiEnergy < v_FitEnergy) {
            v_HiEnergy = v_FitEnergy;
            v_TIME = v_FitTime;
          }
        } // i_DigiLoop
      } // MEclCFShowerHA LOOP

      const double energyBfCorrect = shower.second.Energy();
      const double preliminaryCalibration = correctionFactor(energyBfCorrect, shower.second.Theta()) ;
      const double sEnergy = shower.second.Energy() / preliminaryCalibration;

      //... Calibration for Highest Energy in Shower
      //... Assuming that scaling will be constant
      //... TODO more careful study in future.
      //... Vishal
      const double HighCalibrationFactor = v_HiEnergy / energyBfCorrect;
      const double HiEnergyinShower = sEnergy * HighCalibrationFactor;

      if (!eclShowers) eclShowers.create();
      const auto eclRecShower = eclShowers.appendNew();
      eclRecShower->setShowerId(nShower);
      eclRecShower->setEnergy(sEnergy);
      eclRecShower->setTheta(shower.second.Theta());
      eclRecShower->setPhi(shower.second.Phi());
      eclRecShower->setR(shower.second.Distance());
      eclRecShower->setMass(shower.second.Mass());
      eclRecShower->setWidth(shower.second.Width());
      eclRecShower->setE9oE25(shower.second.E9oE25());
      eclRecShower->setE9oE25unf(shower.second.E9oE25unf());
      eclRecShower->setNHits(shower.second.NHits());
      eclRecShower->setStatus(shower.second.Status());
      eclRecShower->setGrade(shower.second.Grade());
      eclRecShower->setUncEnergy(shower.second.UncEnergy());
      eclRecShower->setTime(v_TIME);
      const double sTheta = shower.second.Theta();
      // In ECLShower convention, off-diagonals are ignored.
      float ErrorMatrix[3] = {
        static_cast<float>(errorEnergy(sEnergy)),
        static_cast<float>(errorTheta(sEnergy, sTheta)),
        static_cast<float>(errorPhi(sEnergy, sTheta))
      };
      eclRecShower->setError(ErrorMatrix);

      // For ECLCluster, keep room for off-diagonals though for a while 0.
      double Mdst_Error[6] = {
        errorEnergy(sEnergy),
        0,
        errorPhi(sEnergy, sTheta),
        0,
        0,
        errorTheta(sEnergy, sTheta)
      };

      // Fill ECLCluster here
      if (!eclClusters) eclClusters.create();
      // Loose timing cut is applied. 20150529 K.Miyabayashi
      if (-300.0 < v_TIME && v_TIME < 200.0) {
        const auto eclCluster = eclClusters.appendNew();

        eclCluster->setError(Mdst_Error);
        eclCluster->setTiming(v_TIME);
        eclCluster->setEnergy(sEnergy);
        eclCluster->setTheta(shower.second.Theta());
        eclCluster->setPhi(shower.second.Phi());
        eclCluster->setR(shower.second.Distance());
        eclCluster->setE9oE25(shower.second.E9oE25());
        eclCluster->setEnedepSum(shower.second.UncEnergy());
        eclCluster->setNofCrystals(shower.second.NHits());
        eclCluster->setHighestE(HiEnergyinShower);

        // Ad hoc Temp correction to reduce energy bias
        // to be replaced ASAP with a proper calibration

        m_tmpClusterCorrection.scale(*eclCluster);

        eclCluster->addRelationTo(eclRecShower);
        //..... ECLCluster is completed here......
      }// End of timing cut branch.

      nShower++; // Increment counter for ECLShower.
    }//EclCFShowerMap
  }//vector<TEclCFCR>
  m_nEvent++;
}

void ECLReconstructorModule::endRun()
{
  m_nRun++;
}

void ECLReconstructorModule::terminate()
{
  m_timeCPU = clock() * Unit::us - m_timeCPU;
}

double ECLReconstructorModule::correctionFactor(double Energy, double Theta)
{
  vector<double>::const_iterator it = upper_bound(m_ranges.begin(), m_ranges.end(), Theta);
  double x = log(Energy);
  // not really necessary check
  // if(it==m_ranges.begin()||it==m_ranges.end()){
  //   B2ERROR("Theta is out of range! Theta = "<<Theta<<" Theta_min = "<<m_ranges.front()<<" Theta_max = "<<m_ranges.back());
  //   return 1.0;
  // }
  double* c = &m_ecorr[0] + 4 * ((it - m_ranges.begin()) - 1);
  return c[0] + x * (c[1] + x * (c[2] + x * (c[3])));
}




//.................
// For track matching
// Copied from ECLGammaReconstructorModule to avoid dependency
// Vishal, One way is to use ECLTrackShowerMatchModule but I simply use
// funciton here
/*
void ECLReconstructorModule::readExtrapolate()
{
  for (int i = 0; i < 8736; i++) {
    m_TrackCellId[i] = -10 ;
  }

  StoreArray<Track> Tracks;
  StoreArray<ExtHit> extHits;
  RelationIndex<Track, ExtHit> TracksToExtHits(Tracks, extHits);

  if (extHits) {
    Const::EDetector myDetID = Const::EDetector::ECL; // ECL in this example
    int pdgCodePiP = G4ParticleTable::GetParticleTable()->FindParticle("pi+")->GetPDGEncoding();
    int pdgCodePiM = G4ParticleTable::GetParticleTable()->FindParticle("pi-")->GetPDGEncoding();

    typedef RelationIndex<Track, ExtHit>::Element relElement_t;
    for (int i_trkLoop = 0; i_trkLoop < Tracks.getEntries(); ++i_trkLoop) {
      BOOST_FOREACH(const relElement_t& rel, TracksToExtHits.getElementsFrom(Tracks[i_trkLoop])) {
        const ExtHit* extHit = rel.to;
        if (extHit->getPdgCode() != pdgCodePiP && extHit->getPdgCode() != pdgCodePiM) continue;
        if ((extHit->getDetectorID() != myDetID) || (extHit->getCopyID() == 0)) continue;
        // Fill here the id of the Tracks to
        // m_TrackCellId to put relation to Shower
        m_TrackCellId[extHit->getCopyID()] = i_trkLoop;
      }//for cands
    }//Tracks.getEntries()
  }//if extTrackCands

}

*/
//.................................................



//................
// Fill here Px, Py and Pz : Vishal
double ECLReconstructorModule::Px(double energy, double theta, double phi)
{
  return energy * sin(theta) * cos(phi);
}

double ECLReconstructorModule::Py(double energy, double theta, double phi)
{
  return energy * sin(theta) * sin(phi);
}

double ECLReconstructorModule::Pz(double energy, double theta)
{
  return energy * cos(theta);
}
//_______________


//...............
// Fill here Error on Energy
double ECLReconstructorModule::errorEnergy(double energy)
{
  const double sigmaE = 0.01 * (0.047 / energy + 1.105 / sqrt(sqrt(energy)) + 0.8563) * energy;
  //double sigmaE = 0.01 * E * sqrt(squ(0.066 / E) + squ(0.81) / sqrt(E) + squ(1.34)) ;
  //sigmaE / E = 0.066% / E +- 0.81% / (E)^(1/4)  +- 1.34%//NIM A441, 401(2000)
  return sigmaE;
}
//_______________


//...............
// Fill here Error on Theta
double ECLReconstructorModule::errorTheta(double energy, double theta)
{
  const double sigmaX = 0.1 * (0.27 + 3.4 / sqrt(energy) + 1.8 / sqrt(sqrt(energy))) ;
  //sigmaX (mm)=  0.27 + 3.4/ (E)^(1/2) + 1.8 / (E)^(1/4) ;E (GeV)

  const double zForward  =  196.2;
  const double zBackward = -102.2;
  const double Rbarrel   =  125.0;

  const double theta_f = atan2(Rbarrel, zForward);
  const double theta_b = atan2(Rbarrel, zBackward);

  if (theta < theta_f) {
    return sigmaX * squ(cos(theta)) / zForward;
  } else if (theta > theta_b) {
    return sigmaX * squ(cos(theta)) / (-1 * zBackward);
  } else {
    return sigmaX * sin(theta) / Rbarrel;
  }
}
//_______________


//...............
// Fill here Error on Phi
double ECLReconstructorModule::errorPhi(double energy, double theta)
{
  double sigmaX = 0.1 * (0.27 + 3.4 / sqrt(energy) + 1.8 / sqrt(sqrt(energy))) ;
//sigmaX (mm)=  0.27 + 3.4/ (E)^(1/2) + 1.8 / (E)^(1/4) ;E (GeV)

  const double zForward  =  196.2;
  const double zBackward = -102.2;
  const double Rbarrel   =  125.0;

  const double theta_f = atan2(Rbarrel, zForward);
  const double theta_b = atan2(Rbarrel, zBackward);

  if (theta < theta_f) {
    return sigmaX / (zForward * tan(theta));
  } else if (theta > theta_b) {
    return sigmaX / (zBackward * tan(theta));
  } else {
    return sigmaX / Rbarrel;
  }
}
//_______________



//.................
// For filling error matrix on Px,Py and Pz here : Vishal
void ECLReconstructorModule::readErrorMatrix(double energy, double theta,
                                             double phi, TMatrixFSym& errorMatrix)
{
  const double energyError = ECLReconstructorModule::errorEnergy(energy);
  const double thetaError =  ECLReconstructorModule::errorTheta(energy, theta);
  const double phiError  =   ECLReconstructorModule::errorPhi(energy, theta);

  TMatrixFSym  errEcl(3);   // 3x3 initialize to zero
  errEcl[0][0] = energyError * energyError; // Energy
  errEcl[1][0] = 0;
  errEcl[1][1] = phiError * phiError; // Phi
  errEcl[2][0] = 0;
  errEcl[2][0] = 0;
  errEcl[2][1] = 0;
  errEcl[2][2] = thetaError * thetaError; // Theta

  TMatrixF  jacobian(4, 3);
  const double cosPhi = cos(phi);
  const double sinPhi = sin(phi);
  const double cosTheta = cos(theta);
  const double sinTheta = sin(theta);
  const double E = energy;

  jacobian[0][0] =            cosPhi * sinTheta;
  jacobian[0][1] = -1.0 * E * sinPhi * sinTheta;
  jacobian[0][2] =        E * cosPhi * cosTheta;
  jacobian[1][0] =            sinPhi * sinTheta;
  jacobian[1][1] =        E * cosPhi * sinTheta;
  jacobian[1][2] =        E * sinPhi * cosTheta;
  jacobian[2][0] =                     cosTheta;
  jacobian[2][1] =                          0.0;
  jacobian[2][2] = -1.0 * E          * sinTheta;
  jacobian[3][0] =                          1.0;
  jacobian[3][1] =                          0.0;
  jacobian[3][2] =                          0.0;
  TMatrixFSym errCart(4);
  errCart = errEcl.Similarity(jacobian);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j <= i ; j++) {
      errorMatrix[i][j] = errCart[i][j];
    }
  }
}
//__________________________


//.................
// For filling error matrix on Px,Py and Pz here 7x 7
void ECLReconstructorModule::readErrorMatrix7x7(double energy, double theta,
                                                double phi, TMatrixFSym& errorMatrix)
{
  const double energyError = ECLReconstructorModule::errorEnergy(energy);
  const double thetaError = ECLReconstructorModule::errorTheta(energy, theta);
  const double phiError  = ECLReconstructorModule::errorPhi(energy, theta);

  TMatrixFSym  errEcl(3);   // 3x3 initialize to zero
  errEcl[0][0] = energyError * energyError; // Energy
  errEcl[1][0] = 0;
  errEcl[1][1] = phiError * phiError; // Phi
  errEcl[2][0] = 0;
  errEcl[2][0] = 0;
  errEcl[2][1] = 0;
  errEcl[2][2] = thetaError * thetaError; // Theta

  TMatrixF jacobian(4, 3);
  const double cosPhi = cos(phi);
  const double sinPhi = sin(phi);
  const double cosTheta = cos(theta);
  const double sinTheta = sin(theta);
  const double E = energy;


  jacobian[0][0] =            cosPhi * sinTheta;
  jacobian[0][1] = -1.0 * E * sinPhi * sinTheta;
  jacobian[0][2] =        E * cosPhi * cosTheta;
  jacobian[1][0] =            sinPhi * sinTheta;
  jacobian[1][1] =        E * cosPhi * sinTheta;
  jacobian[1][2] =        E * sinPhi * cosTheta;
  jacobian[2][0] =                     cosTheta;
  jacobian[2][1] =                          0.0;
  jacobian[2][2] = -1.0 * E          * sinTheta;
  jacobian[3][0] =                          1.0;
  jacobian[3][1] =                          0.0;
  jacobian[3][2] =                          0.0;
  TMatrixFSym errCart(4);
  errCart = errEcl.Similarity(jacobian);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j <= i ; j++) {
      errorMatrix[i][j] = errCart[i][j];
    }
  }
  for (int i = 4; i <= 6; ++i) {
    errorMatrix[i][i] = 1.0;
  }

}

void ECLReconstructorModule::TmpClusterCorrection::init(const string& filename)
{
  B2INFO("ECLReconstructor: Reading ad hoc tmp cluster energy corrections from: " << filename);
  // header keeps the values of 3 parameters in any order
  string param;
  ifstream file(filename);
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

  B2INFO("ECLReconstructor: Number of points for interpolation in E: " << m_npointsE);
  B2INFO("ECLReconstructor: Number of Theta regions: " << nbinsTheta);

  for (int i = 0; i < nbinsTheta; ++i) {
    if (i < nbinsTheta - 1) {
      B2INFO("ECLReconstructor: theta < " << m_maxTheta[i]);
    } else {
      B2INFO("ECLReconstructor: theta >= " << m_maxTheta[i - 1]);
    }
    ostringstream ostr;
    for (size_t j = 0; j < ncoeffE; j++)
      ostr << m_tmpCorrection[ j + i * (ncoeffE) ] << " ";
    const char* out = ostr.str().c_str();
    B2INFO("ECLReconstructor: corrections: " << out);
  }
}

void ECLReconstructorModule::TmpClusterCorrection::scale(ECLCluster& c) const
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
    if (iE == 0)
      corr = m_tmpCorrection[ offset ];
    else if (iE < m_npointsE - 1) {
      double incE{ energy - iE * m_deltaE };
      double clow{ m_tmpCorrection[offset + iE ] };
      double chigh{ m_tmpCorrection[offset + iE + 1 ] };
      corr = clow + incE / m_deltaE * (chigh - clow);
    } else
      corr =  m_tmpCorrection[ offset + m_npointsE + 1 ];

    c.setEnergy(energy * corr);
  }
}



