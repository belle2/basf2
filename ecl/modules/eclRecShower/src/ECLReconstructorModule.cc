/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen,Vishal                                       *
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


#include <boost/foreach.hpp>
#include <G4ParticleTable.hh>

#include<ctime>
#include <iomanip>

// ROOT
#include <TVector3.h>
#include <TMatrixFSym.h>

#define PI 3.14159265358979323846

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
  setDescription("Creates ECLRecShower from ECLDigi.");
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

  // Get tracks.
  StoreArray<Track> TrackArray;

  // ECL relevant objects.
  StoreArray<ECLHitAssignment> HitAssignArray;
  StoreArray<ECLShower> ECLShowerArray;
  StoreArray<ECLCluster> ECLClusterArray;
  HitAssignArray.registerInDataStore();
  ECLShowerArray.registerInDataStore();
  ECLClusterArray.registerInDataStore();
  // ECL related relations.
  ECLClusterArray.registerRelationTo(TrackArray);
  ECLClusterArray.registerRelationTo(ECLShowerArray);

  // Following lines are obsolete.
  // StoreArray<ECLHitAssignment>::registerPersistent();
  // StoreArray<ECLShower>::registerPersistent();
  //
  // StoreArray<ECLCluster>::registerPersistent();
  // RelationArray::registerPersistent<ECLCluster, ECLShower>("", "");
  // RelationArray::registerPersistent<ECLCluster, Track>("", "");

}

void ECLReconstructorModule::beginRun()
{
  B2INFO("ECLReconstructorModule: Processing run: " << m_nRun);
}


void ECLReconstructorModule::event()
{
  //Input Array
  StoreArray<ECLDigit> eclDigiArray;
  if (!eclDigiArray) {
    B2DEBUG(100, "ECLDigit in empty in event " << m_nEvent);
    return;
  }


  //Fill information with extrapolate..
  //.. Vishal
  readExtrapolate();
  StoreArray<Track> Tracks;



  cout.unsetf(ios::scientific);
  cout.precision(6);
  TRecEclCF& cf = TRecEclCF::Instance();
  cf.Clear();

  //cout<<"Event "<< m_nEvent<<" Total input entries of Digi Array  "<<eclDigiArray->GetEntriesFast()<<endl;

  TEclEnergyHit ss;
  for (int ii = 0; ii < eclDigiArray.getEntries(); ii++) {
    ECLDigit* aECLDigi = eclDigiArray[ii];
    float FitEnergy    = (aECLDigi->getAmp()) / 20000.;//ADC count to GeV
    //float FitTime      =  (1520 - aECLDigi->getTimeFit())*24.*12/508/(3072/2) ;//in us

    int cId         = (aECLDigi->getCellId() - 1);
    if (FitEnergy < 0.) {continue;}

    cf.Accumulate(m_nEvent, FitEnergy, cId);
  }

  cf.SearchCRs();
  cf.Attributes();

  int nShower = 0;

  for (std::vector<TEclCFCR>::const_iterator iCR = cf.CRs().begin();
       iCR != cf.CRs().end(); ++iCR) {


    for (EclCFShowerMap::const_iterator iShower = iCR->Showers().begin(); iShower != iCR->Showers().end(); ++iShower) {
      TEclCFShower iSh = (*iShower).second;

      //.. To fill Highest Energy in single crystal in a shower
      //.. and fit time for that particular cyrstal in Timing for now
      //.. Also no. of tracks -- Vishal
      double v_HiEnergy = -9999999;
      double v_TIME = -10;
      // bool  i_extMatch = false;
      int v_NofTracks = 0;


      //... This is where the ECLCluster dataobject is filled
      //... i_Mdst is simply the number and used to relate with ECLShower
      //... Vishal
      StoreArray<ECLCluster> eclMdstArray;
      if (!eclMdstArray) eclMdstArray.create();
      eclMdstArray.appendNew();
      int i_Mdst = eclMdstArray.getEntries() - 1;

      //Object for relation of ECLCluster to others
      //.. 16-5-2k14
      const ECLCluster* ecluster = eclMdstArray[i_Mdst];



      std::vector<MEclCFShowerHA> HAs = iSh.HitAssignment();
      for (std::vector<MEclCFShowerHA>::iterator iHA = HAs.begin();
           iHA != HAs.end(); ++iHA) {

        StoreArray<ECLHitAssignment> eclHaArray;
        if (!eclHaArray) eclHaArray.create();
        eclHaArray.appendNew();
        m_HANum = eclHaArray.getEntries() - 1;

        eclHaArray[m_HANum]->setShowerId(nShower);
        eclHaArray[m_HANum]->setCellId(iHA->Id() + 1);


        //... Below simply check for the TrackCellid for particular id
        //... Based on tracks, it count the number of tracks
        //... Also related to the id of the track which is filled to
        //... m_TrackCellId[] in ECLReconstructorModule::readExtrapolate()
        //... Vishal

        RelationArray ECLClustertoTracks(eclMdstArray, Tracks);
        if (m_TrackCellId[iHA->Id()] >= 0) {

          //.. 16-5-2k14
          const Track*  v_track = Tracks[m_TrackCellId[iHA->Id()]];
          ecluster->addRelationTo(v_track);


          ++v_NofTracks;
        }


        //... To get ECLDigit information for particular shower
        //... in order to get FitEnergy and FitTime for each crystal
        //... in particular shower. To speed up Mapping to be used.
        //... Vishal
        for (int i_DigiLoop = 0; i_DigiLoop < eclDigiArray.getEntries();
             i_DigiLoop++) {
          ECLDigit* aECLDigi = eclDigiArray[i_DigiLoop];
          if (aECLDigi->getCellId() != (iHA->Id() + 1))continue;
          float v_FitEnergy    = (aECLDigi->getAmp()) / 20000.;
          float v_FitTime      =  aECLDigi->getTimeFit();
          if (v_HiEnergy < v_FitEnergy) {
            v_HiEnergy = v_FitEnergy;
            v_TIME = v_FitTime;
          }
        } // i_DigiLoop
      } // MEclCFShowerHA LOOP


      double energyBfCorrect = (*iShower).second.Energy();
      double preliminaryCalibration = correctionFactor(energyBfCorrect, (*iShower).second.Theta()) ;
      double sEnergy = (*iShower).second.Energy() / preliminaryCalibration;


      //... Calibration for Highest Energy in Shower
      //... Assuming that scaling will be constant
      //... TODO more careful study in future.
      //... Vishal
      double HighCalibrationFactor = v_HiEnergy / energyBfCorrect;
      double HiEnergyinShower = sEnergy * HighCalibrationFactor;




      StoreArray<ECLShower> eclRecShowerArray;
      if (!eclRecShowerArray) eclRecShowerArray.create();
      eclRecShowerArray.appendNew();
      m_hitNum = eclRecShowerArray.getEntries() - 1;
      eclRecShowerArray[m_hitNum]->setShowerId(nShower);
      eclRecShowerArray[m_hitNum]->setEnergy((float) sEnergy);
      eclRecShowerArray[m_hitNum]->setTheta((float)(*iShower).second.Theta());
      eclRecShowerArray[m_hitNum]->setPhi((float)(*iShower).second.Phi());
      eclRecShowerArray[m_hitNum]->setR((float)(*iShower).second.Distance());
      eclRecShowerArray[m_hitNum]->setMass((float)(*iShower).second.Mass());
      eclRecShowerArray[m_hitNum]->setWidth((float)(*iShower).second.Width());
      eclRecShowerArray[m_hitNum]->setE9oE25((float)(*iShower).second.E9oE25());
      eclRecShowerArray[m_hitNum]->setE9oE25unf((float)(*iShower).second.E9oE25unf());
      eclRecShowerArray[m_hitNum]->setNHits((*iShower).second.NHits());
      eclRecShowerArray[m_hitNum]->setStatus((*iShower).second.Status());
      eclRecShowerArray[m_hitNum]->setGrade((*iShower).second.Grade());
      eclRecShowerArray[m_hitNum]->setUncEnergy((float)(*iShower).second.UncEnergy());



      double sTheta = (*iShower).second.Theta();
      float ErrorMatrix[3] = {
        errorE(sEnergy),
        errorTheta(sEnergy, sTheta),
        errorPhi(sEnergy, sTheta)
      };
      eclRecShowerArray[m_hitNum]->setError(ErrorMatrix);


      //... This is where the ECLCluster dataobject is filled
      //... i_Mdst is simply the number and used to relate with ECLShower
      //... Vishal


      eclRecShowerArray[m_hitNum]->setTheta((float)(*iShower).second.Theta());
      eclRecShowerArray[m_hitNum]->setPhi((float)(*iShower).second.Phi());
      eclRecShowerArray[m_hitNum]->setR((float)(*iShower).second.Distance());

      float Mdst_Error[6] = {
        errorE(sEnergy),
        0,
        errorPhi(sEnergy, sTheta),
        0,
        0,
        errorTheta(sEnergy, sTheta)
      };


      //.. Fill ECLCluster here
      //      RelationArray ECLClustertoShower(eclMdstArray, eclRecShowerArray);
      eclMdstArray[i_Mdst]->setError(Mdst_Error);
      eclMdstArray[i_Mdst]->setTiming((float) v_TIME);
      eclMdstArray[i_Mdst]->setEnergy((float) sEnergy);
      eclMdstArray[i_Mdst]->setTheta((float)(*iShower).second.Theta());
      eclMdstArray[i_Mdst]->setPhi((float)(*iShower).second.Phi());
      eclMdstArray[i_Mdst]->setR((float)(*iShower).second.Distance());
      eclMdstArray[i_Mdst]->setE9oE25((float)(*iShower).second.E9oE25());
      eclMdstArray[i_Mdst]->setEnedepSum((float)(*iShower).second.UncEnergy());

      // To check if matches with charged tracks or not
      bool v_isTrack = false;
      if (v_NofTracks > 0) {v_isTrack = true;}
      else { v_isTrack = false;}
      eclMdstArray[i_Mdst]->setisTrack(v_isTrack);
      eclMdstArray[i_Mdst]->setTiming((float) v_TIME);
      eclMdstArray[i_Mdst]->setHighestE((float)  HiEnergyinShower);

      //Object for relation of ECLShower to ECLCLuster
      //.. 16-5-2k14
      const ECLShower* eclshower = eclRecShowerArray[m_hitNum];



      //... Relation of ECLCluster to ECLShower
      ecluster->addRelationTo(eclshower);


      //..... ECLCluster is completed here......




      nShower++;

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



//... Correction are now in ECL-FirstCorrection.xml
//...  Read using GearDir : Vishal

double ECLReconstructorModule::correctionFactor(double Energy, double Theta)
{

  //Used to break free of loop and also to check for any mistake in logic
  int  eCorrId = -10;
  //Corrections
  double energyCorrectPolynomial[4] = {1.0, 0.0, 0.0, 0.0};
  //... The above initialiation (1,0,0,0) will provide calibration factor of 1
  //... i.e. is no correction : Vishal




  /*Register the ECL.xml file, which further register the
    ECL-FirstCorrection.xml file and use GearDir to access the elements*/
  GearDir ThetaBins("Detector/DetectorComponent[@name=\"ECL\"]/Content/ECLFirstCorrection/ThetaBins/");
  GearDir ThetaBinsContent(ThetaBins);


  /* Below section help in reducing the looping time
     checks which bin theta  should be, above or below
     the middle of detector. This may help in speed up
     of code. LoopStart is the first from where loop
     will start and LoopStop is the last. */

  // This is kept 1 as first entry is 1 in xml file
  int LoopStart = 1;
  // The number of bins the detector is divided into
  int LoopStop  = ThetaBinsContent.getNumberNodes("ThetaBins");
  int MiddleBin = int((LoopStop) / 2); // Take in between the bins

  ThetaBinsContent.append((boost::format("ThetaBins[%1%]/") % (MiddleBin)).str());
  if (Theta <  ThetaBinsContent.getAngle("ThetaEnd"))
  {LoopStop = MiddleBin;}
  else {LoopStart = MiddleBin + 1;}



  for (int iThetaBins = LoopStart; iThetaBins <= LoopStop; ++iThetaBins) {
    GearDir ThetaBinsContent(ThetaBins);

    ThetaBinsContent.append((boost::format("ThetaBins[%1%]/") % (iThetaBins)).str());

    if (Theta >= ThetaBinsContent.getAngle("ThetaStart")
        && Theta < ThetaBinsContent.getAngle("ThetaEnd")) {
      eCorrId = iThetaBins;
      energyCorrectPolynomial[0] = ThetaBinsContent.getDouble("Polynomial1");
      energyCorrectPolynomial[1] = ThetaBinsContent.getDouble("Polynomial2");
      energyCorrectPolynomial[2] = ThetaBinsContent.getDouble("Polynomial3");
      energyCorrectPolynomial[3] = ThetaBinsContent.getDouble("Polynomial4");
    }

    if (eCorrId > 0) break;
    //break loop Here as now we have got correction,
    //Save Time No need of looping more.
  }


  //Check, if there is some problem to get proper Theta !!!
  if (eCorrId < 1 || eCorrId > 15) {
    B2ERROR("ECLShower theta out of range " << (Theta / PI * 180))
  }

  double  preliminaryCalibration = energyCorrectPolynomial[0]
                                   + energyCorrectPolynomial[1] * pow(log(Energy), 1)
                                   + energyCorrectPolynomial[2] * pow(log(Energy), 2)
                                   + energyCorrectPolynomial[3] * pow(log(Energy), 3);


  /* OLD CODE to get correction with hard coded numbers.. To be removed in future -- Vishal

  int  eCorrId = -1;
  if (Theta > 0 && (Theta / PI * 180) < 32.2) {
    eCorrId = 0;
  } else if ((Theta / PI * 180) >= 32.2 && (Theta / PI * 180) < 35.0) {
    eCorrId = 1;
  } else if ((Theta / PI * 180) >= 35.0 && (Theta / PI * 180) < 37.5) {
    eCorrId = 2;
  } else if ((Theta / PI * 180) >= 37.5 && (Theta / PI * 180) < 40.0) {
    eCorrId = 3;
  } else if ((Theta / PI * 180) >= 40.0 && (Theta / PI * 180) < 42.5) {
    eCorrId = 4;
  } else if ((Theta / PI * 180) >= 42.5 && (Theta / PI * 180) < 45.0) {
    eCorrId = 5;
  } else if ((Theta / PI * 180) >= 45.0 && (Theta / PI * 180) < 55.0) {
    eCorrId = 6;
  } else if ((Theta / PI * 180) >= 55.0 && (Theta / PI * 180) < 65.0) {
    eCorrId = 7;
  } else if ((Theta / PI * 180) >= 65.0 && (Theta / PI * 180) < 75.0) {
    eCorrId = 8;
  } else if ((Theta / PI * 180) >= 75.0 && (Theta / PI * 180) < 85.0) {
    eCorrId = 9;
  } else if ((Theta / PI * 180) >= 85.0 && (Theta / PI * 180) < 95.0) {
    eCorrId = 10;
  } else if ((Theta / PI * 180) >= 95.0 && (Theta / PI * 180) < 105.0) {
    eCorrId = 11;
  } else if ((Theta / PI * 180) >= 105.0 && (Theta / PI * 180) < 115.0) {
    eCorrId = 12;
  } else if ((Theta / PI * 180) >= 115.0 && (Theta / PI * 180) < 128.7) {
    eCorrId = 13;
  } else if ((Theta / PI * 180) >= 128.7 && (Theta / PI * 180) <= 180) {
    eCorrId = 14;

  } else {
    B2ERROR("ECLShower theta out of range " << (Theta / PI * 180));

  }




  double energyCorrectPolynomial[15][4] = {
    {9.56670E-01, -2.28268E-03, -2.23364E-03, 2.41601E-04},
    {9.39221E-01, -6.01211E-03, -1.01388E-03, 2.21742E-04},
    {9.47695E-01, -4.00680E-03, -6.62361E-04, 3.51879E-04},
    {9.46688E-01, -2.83932E-03, -6.50563E-04, 2.73304E-04},
    {9.51124E-01, -3.64272E-03, -1.03425E-03, 2.14907E-04},
    {9.53309E-01, -2.62172E-03, -8.97187E-04, 3.51823E-04},
    {9.53309E-01, -2.62172E-03, -8.97187E-04, 3.51823E-04},
    {9.58557E-01, -2.44004E-03, -1.49555E-03, 1.41088E-04},
    {9.60535E-01, -2.77543E-03, -1.44099E-03, 2.44692E-04},
    {9.61075E-01, -3.09389E-03, -1.45289E-03, 2.16016E-04},
    {9.60538E-01, -3.35572E-03, -1.36629E-03, 2.30296E-04},
    {9.61748E-01, -4.30415E-03, -1.05225E-03, 3.59288E-04},
    {9.61925E-01, -4.44564E-03, -1.13480E-03, 2.55418E-04},
    {9.58142E-01, -4.75945E-03, -6.66410E-04, 4.77996E-04},
    {9.49364E-01, -3.31338E-03, -2.25274E-03, 1.43136E-04}
  };




  double  preliminaryCalibration = energyCorrectPolynomial[eCorrId][0]
                                   + energyCorrectPolynomial[eCorrId][1] * pow(log(Energy), 1)
                                   + energyCorrectPolynomial[eCorrId][2] * pow(log(Energy), 2)
                                   + energyCorrectPolynomial[eCorrId][3] * pow(log(Energy), 3);
  */

  return preliminaryCalibration;

}




//.................
// For track matching
// Copied from ECLGammaReconstructorModule to avoid dependency
// Vishal, One way is to use ECLTrackShowerMatchModule but I simply use
// funciton here
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
      BOOST_FOREACH(const relElement_t & rel, TracksToExtHits.getElementsFrom(Tracks[i_trkLoop])) {
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
//.................................................



//................
// Fill here Px, Py and Pz : Vishal
float ECLReconstructorModule::Px(double Energy, double Theta, double Phi)
{
  return float(Energy * sin(Theta) * cos(Phi));
}

float ECLReconstructorModule::Py(double Energy, double Theta, double Phi)
{
  return float(Energy * sin(Theta) * sin(Phi));
}

float ECLReconstructorModule::Pz(double Energy, double Theta)
{
  return float(Energy * cos(Theta));
}
//_______________


//...............
// Fill here Error on Energy
float ECLReconstructorModule::errorE(double E)
{
  double sigmaE = 0.01 * (0.047 / E + 1.105 / sqrt(sqrt(E)) + 0.8563) * E;
  //double sigmaE = 0.01 * E * sqrt(squ(0.066 / E) + squ(0.81) / sqrt(E) + squ(1.34)) ;
  //sigmaE / E = 0.066% / E +- 0.81% / (E)^(1/4)  +- 1.34%//NIM A441, 401(2000)
  return (float)sigmaE;
}
//_______________


//...............
// Fill here Error on Theta
float ECLReconstructorModule::errorTheta(double Energy, double Theta)
{
  double sigmaX = 0.1 * (0.27 + 3.4 / sqrt(Energy) + 1.8 / sqrt(sqrt(Energy))) ;
  //sigmaX (mm)=  0.27 + 3.4/ (E)^(1/2) + 1.8 / (E)^(1/4) ;E (GeV)

  double zForward  =  196.2;
  double zBackward = -102.2;
  double Rbarrel   =  125.0;

  double theta_f = atan2(Rbarrel, zForward);
  double theta_b = atan2(Rbarrel, zBackward);

  if (Theta < theta_f) {
    return (float)(sigmaX * squ(cos(Theta)) / zForward);
  } else if (Theta > theta_b) {
    return (float)(sigmaX * squ(cos(Theta)) / (-1 * zBackward));
  } else {
    return (float)(sigmaX * sin(Theta) / Rbarrel)   ;
  }
}
//_______________


//...............
// Fill here Error on Phi
float ECLReconstructorModule::errorPhi(double Energy, double Theta)
{
  double sigmaX = 0.1 * (0.27 + 3.4 / sqrt(Energy) + 1.8 / sqrt(sqrt(Energy))) ;
//sigmaX (mm)=  0.27 + 3.4/ (E)^(1/2) + 1.8 / (E)^(1/4) ;E (GeV)

  double zForward  =  196.2;
  double zBackward = -102.2;
  double Rbarrel   =  125.0;

  double theta_f = atan2(Rbarrel, zForward);
  double theta_b = atan2(Rbarrel, zBackward);

  if (Theta < theta_f) {
    return (float)(sigmaX / (zForward * tan(Theta)))  ;
  } else if (Theta > theta_b) {
    return (float)(sigmaX / (zBackward * tan(Theta)))   ;
  } else {
    return (float)(sigmaX / Rbarrel);
  }
}
//_______________



//.................
// For filling error matrix on Px,Py and Pz here : Vishal
void ECLReconstructorModule::readErrorMatrix(double Energy, double Theta,
                                             double Phi, TMatrixFSym& ErrorMatrix)
{
  float EnergyError = float(ECLReconstructorModule::errorE(Energy));
  float ThetaError =  float(ECLReconstructorModule::errorTheta(Energy, Theta));
  float PhiError  =   float(ECLReconstructorModule::errorPhi(Energy, Theta));

  TMatrixFSym  errEcl(3);   // 3x3 initialize to zero
  errEcl[ 0 ][ 0 ] = EnergyError * EnergyError; // Energy
  errEcl[ 1 ][ 0 ] = 0;
  errEcl[ 1 ][ 1 ] = PhiError * PhiError; // Phi
  errEcl[ 2 ][ 0 ] = 0;
  errEcl[ 2 ][ 0 ] = 0;
  errEcl[ 2 ][ 1 ] = 0;
  errEcl[ 2 ][ 2 ] = ThetaError * ThetaError; // Theta

  TMatrixF  jacobian(4, 3);
  float  cosPhi = cos(Phi);
  float  sinPhi = sin(Phi);
  float  cosTheta = cos(Theta);
  float  sinTheta = sin(Theta);
  float   E = float(Energy);


  jacobian[ 0 ][ 0 ] =            cosPhi * sinTheta;
  jacobian[ 0 ][ 1 ] =  -1.0 * E * sinPhi * sinTheta;
  jacobian[ 0 ][ 2 ] =        E * cosPhi * cosTheta;
  jacobian[ 1 ][ 0 ] =            sinPhi * sinTheta;
  jacobian[ 1 ][ 1 ] =        E * cosPhi * sinTheta;
  jacobian[ 1 ][ 2 ] =        E * sinPhi * cosTheta;
  jacobian[ 2 ][ 0 ] =                     cosTheta;
  jacobian[ 2 ][ 1 ] =           0.0;
  jacobian[ 2 ][ 2 ] =  -1.0 * E          * sinTheta;
  jacobian[ 3 ][ 0 ] =           1.0;
  jacobian[ 3 ][ 1 ] =           0.0;
  jacobian[ 3 ][ 2 ] =           0.0;
  TMatrixFSym errCart(4);
  errCart = errEcl.Similarity(jacobian);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j <= i ; j++) {
      ErrorMatrix[i][j] = errCart[i][j];
    }
  }
}
//__________________________


//.................
// For filling error matrix on Px,Py and Pz here 7x 7
void ECLReconstructorModule::readErrorMatrix7x7(double Energy, double Theta,
                                                double Phi, TMatrixFSym& ErrorMatrix)
{
  float EnergyError = float(ECLReconstructorModule::errorE(Energy));
  float ThetaError =  float(ECLReconstructorModule::errorTheta(Energy, Theta));
  float PhiError  =   float(ECLReconstructorModule::errorPhi(Energy, Theta));

  TMatrixFSym  errEcl(3);   // 3x3 initialize to zero
  errEcl[ 0 ][ 0 ] = EnergyError * EnergyError; // Energy
  errEcl[ 1 ][ 0 ] = 0;
  errEcl[ 1 ][ 1 ] = PhiError * PhiError; // Phi
  errEcl[ 2 ][ 0 ] = 0;
  errEcl[ 2 ][ 0 ] = 0;
  errEcl[ 2 ][ 1 ] = 0;
  errEcl[ 2 ][ 2 ] = ThetaError * ThetaError; // Theta

  TMatrixF  jacobian(4, 3);
  float  cosPhi = cos(Phi);
  float  sinPhi = sin(Phi);
  float  cosTheta = cos(Theta);
  float  sinTheta = sin(Theta);
  float   E = float(Energy);


  jacobian[ 0 ][ 0 ] =            cosPhi * sinTheta;
  jacobian[ 0 ][ 1 ] =  -1.0 * E * sinPhi * sinTheta;
  jacobian[ 0 ][ 2 ] =        E * cosPhi * cosTheta;
  jacobian[ 1 ][ 0 ] =            sinPhi * sinTheta;
  jacobian[ 1 ][ 1 ] =        E * cosPhi * sinTheta;
  jacobian[ 1 ][ 2 ] =        E * sinPhi * cosTheta;
  jacobian[ 2 ][ 0 ] =                     cosTheta;
  jacobian[ 2 ][ 1 ] =           0.0;
  jacobian[ 2 ][ 2 ] =  -1.0 * E          * sinTheta;
  jacobian[ 3 ][ 0 ] =           1.0;
  jacobian[ 3 ][ 1 ] =           0.0;
  jacobian[ 3 ][ 2 ] =           0.0;
  TMatrixFSym errCart(4);
  errCart = errEcl.Similarity(jacobian);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j <= i ; j++) {
      ErrorMatrix[i][j] = errCart[i][j];
    }
  }
  for (int i = 4; i <= 6; ++i) {
    ErrorMatrix[i][i] = 1.0;
  }

}


