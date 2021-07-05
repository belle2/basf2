/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackFilter/TrackFilterModule.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

double TrackFilterModule::m_min_d0 = -100;
double TrackFilterModule::m_max_d0 = +100;
double TrackFilterModule::m_min_z0 = -500;
double TrackFilterModule::m_max_z0 = +500;
int TrackFilterModule::m_min_NumHitsSVD = 0;
int TrackFilterModule::m_min_NumHitsPXD = 0;
int TrackFilterModule::m_min_NumHitsCDC = 0;
double TrackFilterModule::m_min_pCM = 0;
double TrackFilterModule::m_min_pT = 0;
double TrackFilterModule::m_min_Pval = 0;

bool TrackFilterModule::m_saveControlNtuples = true;

TNtuple* TrackFilterModule::m_selectedNtpl = nullptr;
TNtuple* TrackFilterModule::m_rejectedNtpl = nullptr;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackFilterModule::TrackFilterModule() : Module()
{
  // Set module properties
  setDescription("generates a new StoreArray from the input StoreArray which has all specified Tracks removed");

  // Parameter definitions
  addParam("inputArrayName", m_inputArrayName, "StoreArray with the input tracks", std::string("Tracks"));
  addParam("outputINArrayName", m_outputINArrayName, "StoreArray with the output tracks", std::string(""));
  addParam("outputOUTArrayName", m_outputOUTArrayName, "StoreArray with the output tracks", std::string(""));

  //selection parameter definition
  addParam("min_d0", m_min_d0, "minimum value of the d0", double(-100));
  addParam("max_d0", m_max_d0, "maximum value of the d0", double(+100));
  addParam("min_z0", m_min_z0, "minimum value of the z0", double(-500));
  addParam("max_z0", m_max_z0, "maximum value of the z0", double(+500));
  addParam("min_pCM", m_min_pCM, "minimum value of the center-of-mass-momentum", double(0));
  addParam("min_pT", m_min_pT, "minimum value of the transverse momentum", double(0));
  addParam("min_Pvalue", m_min_Pval, "minimum value of the P-Value of the track fit", double(0));
  addParam("min_NumHitPXD", m_min_NumHitsPXD, "minimum number of PXD hits associated to the trcak", int(0));
  addParam("min_NumHitSVD", m_min_NumHitsSVD, "minimum number of SVD hits associated to the trcak", int(0));
  addParam("min_NumHitCDC", m_min_NumHitsCDC, "minimum number of CDC hits associated to the trcak", int(0));

  addParam("saveControNtuples", m_saveControlNtuples, "if true, generate a rootfile containing histograms ", bool(true));
  addParam("outputFileName", m_rootFileName, "Name of output root file.",
           std::string("TrackFilterControlNtuples.root"));

}


void TrackFilterModule::initialize()
{

  B2INFO("TrackFilterModule::inputArrayName: " << m_inputArrayName);
  B2INFO("TrackFilterModule::outputINArrayName: " <<  m_outputINArrayName);
  B2INFO("TrackFilterModule::outputOUTArrayName: " <<  m_outputOUTArrayName);


  StoreArray<Track> inputArray(m_inputArrayName);
  inputArray.isRequired();

  m_selectedTracks.registerSubset(inputArray, m_outputINArrayName);
  m_selectedTracks.inheritAllRelations();

  m_notSelectedTracks.registerSubset(inputArray, m_outputOUTArrayName);
  m_notSelectedTracks.inheritAllRelations();

  if (m_saveControlNtuples) {
    //set the ROOT File
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
    m_selectedNtpl = new TNtuple("selected", "Selected Tracks", "d0:z0:phi:tanDip:omega:pT:pCM:nPXD:nSVD:nCDC:pVal");
    m_rejectedNtpl = new TNtuple("rejected", "Rejected Tracks", "d0:z0:phi:tanDip:omega:pT:pCM:nPXD:nSVD:nCDC:pVal");
  }


}



void TrackFilterModule::event()
{

  m_selectedTracks.select(isSelected);

  m_notSelectedTracks.select([](const Track * track) {
    return !isSelected(track);
  }
                            );

}

void TrackFilterModule::terminate()
{
  if (m_rootFilePtr != nullptr) {
    m_rootFilePtr->cd();

    m_selectedNtpl->Write();
    m_rejectedNtpl->Write();

    m_rootFilePtr->Close();

  }
}

bool TrackFilterModule::isSelected(const Track* track)
{

  bool isExcluded = false;
  int pionCode = 211;

  const TrackFitResult*  tfr = track->getTrackFitResult(Const::ChargedStable(pionCode));
  if (tfr == nullptr)
    return false;

  if (tfr->getD0() < m_min_d0 || tfr->getD0() > m_max_d0)
    isExcluded = true;

  if (tfr->getZ0() < m_min_z0 || tfr->getZ0() > m_max_z0)
    isExcluded = true;

  if (tfr->getPValue() < m_min_Pval)
    isExcluded = true;

  if (tfr->getMomentum().Perp() < m_min_pT)
    isExcluded = true;

  HitPatternVXD hitPatternVXD = tfr->getHitPatternVXD();
  if (hitPatternVXD.getNSVDHits() < m_min_NumHitsSVD ||  hitPatternVXD.getNPXDHits() < m_min_NumHitsPXD)
    isExcluded = true;

  HitPatternCDC hitPatternCDC = tfr->getHitPatternCDC();
  if (hitPatternCDC.getNHits() < m_min_NumHitsCDC)
    isExcluded = true;

  if (m_saveControlNtuples)
    fillControlNtuples(track, !isExcluded);


  return !isExcluded;

}

void TrackFilterModule::fillControlNtuples(const Track* track , bool isSelected)
{

  int pionCode = 211;
  const TrackFitResult*  tfr = track->getTrackFitResult(Const::ChargedStable(pionCode));
  HitPatternVXD hitPatternVXD = tfr->getHitPatternVXD();
  HitPatternCDC hitPatternCDC = tfr->getHitPatternCDC();

  double d0 = tfr->getD0();
  double z0 = tfr->getZ0();
  float phi = tfr->getPhi();
  float tanDip = tfr->getTanLambda();
  float omega = tfr->getOmega();

  double pt = tfr->getMomentum().Pt();
  TLorentzVector pStar = tfr->get4Momentum();
  pStar.Boost(0, 0, 3. / 11);
  double pCM = pStar.P();
  double pVal = tfr->getPValue();
  int nPXDhits = hitPatternVXD.getNPXDHits();
  int nSVDhits = hitPatternVXD.getNSVDHits();
  int nCDChits = hitPatternCDC.getNHits();

  float nPXD = nPXDhits;
  float nSVD = nSVDhits;
  float nCDC = nCDChits;

  if (isSelected)
    m_selectedNtpl->Fill(d0, z0, phi, tanDip, omega, pt, pCM, nPXD, nSVD, nCDC, pVal);
  else
    m_rejectedNtpl->Fill(d0, z0, phi, tanDip, omega, pt, pCM, nPXD, nSVD, nCDC, pVal);


}
