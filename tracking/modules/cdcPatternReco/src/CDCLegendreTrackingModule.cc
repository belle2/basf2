/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcPatternReco/CDCLegendreTrackingModule.h>

//#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>

#include "GFTrackCand.h"

#include <cstdlib>
#include <iomanip>
#include <string>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <iostream>
#include <algorithm>
#include <memory>
#include <cmath>

using namespace std;
using namespace Belle2;

//ROOT macro
REG_MODULE(CDCLegendreTracking)

CDCLegendreTrackingModule::CDCLegendreTrackingModule() :
  Module(), m_rMin(-0.15), m_rMax(0.15)
{
  setDescription(
    "Performs the pattern recognition in the CDC with the conformal finder: digitized CDCHits are combined to track candidates (GFTrackCand). (Works for tracks momentum > 0.5 GeV, no curl track finder included yet).");

  addParam("CDCHitsColName", m_cdcHitsColName,
           "Input CDCHits collection (should be created by CDCDigi module)",
           string("CDCHits"));

  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName,
           "Output GFTrackCandidates collection",
           string("GFTrackCandidates_LegendreFinder"));

  addParam("NbinsTheta", m_nbinsTheta,
           "Number of bins in theta direction of the legendre plane", 1000);
  addParam("NbinsR", m_nbinsR,
           "Number of bins in r direction of the legendre plane", 1000);
  addParam("DistTheta", m_distTheta,
           "Minimal distance of two tracks in theta direction", m_PI / 30);
  addParam("DistR", m_distR, "Minimal distance of two tracks in r direction",
           0.004);
  addParam("Threshold", m_threshold, "Threshold for peak finder", 30.);

  addParam("ThresholdUnique", m_thresholdUnique,
           "Threshold of unique hits for track building", 30.);

  addParam("Resolution AxialHits", m_resolutionAxial,
           "Total resolution, used for the assignment of axial hits to tracks",
           0.3);

  addParam("Resolution StereoHits", m_resolutionStereo,
           "Total resolution, used for the assignment of stereo hits to tracks",
           0.05);
}

CDCLegendreTrackingModule::~CDCLegendreTrackingModule()
{

}

void CDCLegendreTrackingModule::initialize()
{
  //StoreArray with CDCLegendreTrackHits: a class derived from the original CDCHit, but also additional member variables and methods needed for tracking
  StoreArray<CDCLegendreTrackHit> cdcLegendreTrackHits(
    "CDCLegendreTrackHits");

  //StoreArray for CDCTrackCandidates: this track candidate class is the output of the conformal finder
  StoreArray<CDCLegendreTrackCandidate> cdcLegendreTrackCandidates(
    "CDCLegendreTrackCandidates");

  //StoreArray for GFTrackCandidates: interface class to Genfit, there should be one GFTrackCandidate for each CDCTrackCandidate
  StoreArray<GFTrackCand> trackCandidates(m_gfTrackCandsColName);

  //Calculation of minimal distances in bins from given values
  m_distThetaInBins = int(m_distTheta / m_PI * m_nbinsTheta);
  m_distRInBins = int(m_distR / (m_rMax - m_rMin) * m_nbinsR);

  //calculate the needed sin and cos values and store in array
  sin_theta = new double[m_nbinsTheta];
  cos_theta = new double[m_nbinsTheta];

  for (int i = 0; i < m_nbinsTheta; i++) {
    sin_theta[i] = sin((i + 0.5) * m_PI / m_nbinsTheta);
    cos_theta[i] = cos((i + 0.5) * m_PI / m_nbinsTheta);
  }
}

void CDCLegendreTrackingModule::beginRun()
{

}

void CDCLegendreTrackingModule::event()
{
  B2INFO("**********   CDCTrackingModule  ************");

  //StoreArray with digitized CDCHits, should already be created by CDCDigitizer module
  StoreArray<CDCHit> cdcHits(m_cdcHitsColName);
  B2DEBUG(100,
          "CDCTracking: Number of digitized Hits: " << cdcHits.getEntries());
  if (cdcHits.getEntries() == 0)
    B2WARNING("CDCTracking: cdcHitsCollection is empty!");

  //StoreArray with CDCTrackHits: a class derived from the original CDCHit, but also additional member variables and methods needed for tracking
  StoreArray<CDCLegendreTrackHit> cdcLegendreTrackHits(
    "CDCLegendreTrackHits");

  for (int iHit = 0; iHit < cdcHits.getEntries(); iHit++) {
    new(cdcLegendreTrackHits->AddrAt(iHit)) CDCLegendreTrackHit(
      cdcHits[iHit], iHit);
  }

  //perform track finding
  std::vector<std::pair<int, int> > tracks = DoTrackFinding();

  //create LegendreTrackCandidates
  createLegendreTrackCandidates(tracks);

  //create GenFit Track candidates
  createGFTrackCandidates();

}

std::vector<std::pair<int, int> > CDCLegendreTrackingModule::DoTrackFinding()
{

  //StoreArray with CDCTrackHits: a class derived from the original CDCHit, but also additional member variables and methods needed for tracking
  StoreArray<CDCLegendreTrackHit> cdcLegendreTrackHits(
    "CDCLegendreTrackHits");

  //Create 2 dimensional array, this is the plane in the legendre space
  std::auto_ptr<std::vector<std::vector<int> > > m_legendrePlane(
    new std::vector<std::vector<int> >(m_nbinsTheta,
                                       std::vector<int>(m_nbinsR, 0)));

  for (int iHit = 0; iHit < cdcLegendreTrackHits.getEntries(); ++iHit) {
    if (not cdcLegendreTrackHits[iHit]->getIsAxial())
      continue;

    //Do the transformation in legendre plane and voting (according to NIM A592 456 (2008))
    for (int theta_bin = 0; theta_bin < m_nbinsTheta; ++theta_bin) {
      double r1 = cdcLegendreTrackHits[iHit]->getConformalX()
                  * cos_theta[theta_bin]
                  + cdcLegendreTrackHits[iHit]->getConformalY()
                  * sin_theta[theta_bin]
                  + cdcLegendreTrackHits[iHit]->getConformalDriftTime();
      double r2 = cdcLegendreTrackHits[iHit]->getConformalX()
                  * cos_theta[theta_bin]
                  + cdcLegendreTrackHits[iHit]->getConformalY()
                  * sin_theta[theta_bin]
                  - cdcLegendreTrackHits[iHit]->getConformalDriftTime();

      ++(*m_legendrePlane)[theta_bin][calcBin(r1)];
      ++(*m_legendrePlane)[theta_bin][calcBin(r2)];
    }
  }

  //Find maximum in legendre plane
  std::vector<std::pair<int, std::pair<int, int> > > candidates;
  std::vector<std::pair<int, int> > tracks;

  //Find all entries with number of votes greater threshold and fill in candidates list
  for (int i = 0; i < m_nbinsTheta; ++i) {
    for (int j = 0; j < m_nbinsR; ++j) {
      if ((*m_legendrePlane)[i][j] > m_threshold) {
        candidates.push_back(
          std::make_pair((*m_legendrePlane)[i][j],
                         std::make_pair(i, j)));
      }
    }
  }

  //Sort once
  std::sort(candidates.begin(), candidates.end(), SortCandidatesByVotes());

  //Kick out entries nearer to the maximum than the defined minimal distance
  while (!candidates.empty()) {

    std::pair<int, int> p = candidates[0].second;
    int Theta_pri = p.first;
    int R_pri = p.second;

    //check remaining track candidates
    for (std::vector<std::pair<int, std::pair<int, int> > >::iterator i =
           candidates.begin() + 1; i != candidates.end();) {
      int Theta = i->second.first;
      int R = i->second.second;

      //check for distance
      if (fabs(Theta_pri - Theta) < m_distThetaInBins && fabs(R_pri - R) < m_distRInBins) {
        i = candidates.erase(i); //erase added candidate and keep iterator valid
      } else {
        ++i;
      }

    }

    tracks.push_back(std::make_pair(Theta_pri, R_pri));
    candidates.erase(candidates.begin());
  }

  return tracks;
}

void CDCLegendreTrackingModule::createLegendreTrackCandidates(
  std::vector<std::pair<int, int> > tracks)
{
  int id_track = 0;
  std::pair<int, int> trackCandidate;
  //now it gets a little bit ugly, first we create all possible tracks, but in the end we only want to use the tracks with enough unique hits. Therefore we first need to store them somewhere else (since storing in the TClonesArray requires calling the constructor and deleting something from it might be problematic)
  boost::ptr_vector<CDCLegendreTrackCandidate> vTrackCandidates;

  BOOST_FOREACH(trackCandidate, tracks) {
    //get actual theta and r values for each track candidate
    double track_theta = getBinCenter(trackCandidate.first, 0, m_PI,
                                      m_nbinsTheta);
    double track_r = getBinCenter(trackCandidate.second, m_rMin, m_rMax,
                                  m_nbinsR);

    //get charge estimation for the track candidate
    int charge = CDCLegendreTrackCandidate::getChargeAssumption("CDCLegendreTrackHits", track_theta, track_r, m_resolutionAxial);

    //right now, we don't want to find curlers
    if (charge == CDCLegendreTrackCandidate::charge_curler)
      continue;

    //for curlers, negative, and positive tracks we want to create one track candidate
    if (charge == CDCLegendreTrackCandidate::charge_positive
        || charge == CDCLegendreTrackCandidate::charge_negative
        || charge == CDCLegendreTrackCandidate::charge_curler) {
      vTrackCandidates.push_back(
        new CDCLegendreTrackCandidate(id_track, track_theta,
                                      track_r, charge, "CDCLegendreTrackHits",
                                      m_resolutionAxial, m_resolutionStereo));
      ++id_track;
    }

    //here we create two oppositely charged tracks (with the same theta and r value)
    else if (charge == CDCLegendreTrackCandidate::charge_two_tracks) {
      vTrackCandidates.push_back(
        new CDCLegendreTrackCandidate(id_track, track_theta,
                                      track_r, CDCLegendreTrackCandidate::charge_positive,
                                      "CDCLegendreTrackHits", m_resolutionAxial,
                                      m_resolutionStereo));
      ++id_track;

      vTrackCandidates.push_back(
        new CDCLegendreTrackCandidate(id_track, track_theta,
                                      track_r, CDCLegendreTrackCandidate::charge_negative,
                                      "CDCLegendreTrackHits", m_resolutionAxial,
                                      m_resolutionStereo));
      ++id_track;
    }
    //This shouldn't happen, check CDCLegendreTrackCandidate::getChargeAssumption()
    else {
      B2ERROR(
        "Strange behavior of CDCLegendreTrackCandidate::getChargeAssumption");
      exit(EXIT_FAILURE);
    }

  }

  //create StoreArray for CDCTrackCandidates: this track candidate class is one of the output of the Legendre finder
  StoreArray<CDCLegendreTrackCandidate> cdcLegendreTrackCandidates(
    "CDCLegendreTrackCandidates");
  int addr_track = 0;
  BOOST_FOREACH(CDCLegendreTrackCandidate & trackCandidate, vTrackCandidates) {
    //here comes the check for unique hits, now the tracks are constructed again within the TClonesArray
    if (trackCandidate.getStartingUniqueHits() > m_thresholdUnique) {
      new(cdcLegendreTrackCandidates->AddrAt(addr_track)) CDCLegendreTrackCandidate(
        trackCandidate.getID(), trackCandidate.getTheta(),
        trackCandidate.getR(), trackCandidate.getCharge(),
        "CDCLegendreTrackHits", m_resolutionAxial,
        m_resolutionStereo, false);
      ++addr_track;
    }
  }

  B2INFO("Found " << addr_track << " tracks");
}

void CDCLegendreTrackingModule::createGFTrackCandidates()
{
  StoreArray<CDCLegendreTrackCandidate> cdcLegendreTrackCandidates(
    "CDCLegendreTrackCandidates");
  StoreArray<CDCLegendreTrackHit> cdcLegendreTrackHits(
    "CDCLegendreTrackHits");

//StoreArray for GFTrackCandidates: interface class to Genfit
  StoreArray<GFTrackCand> gfTrackCandidates(m_gfTrackCandsColName);

  for (int i = 0; i < cdcLegendreTrackCandidates.getEntries(); i++) {

    new(gfTrackCandidates->AddrAt(i)) GFTrackCand();  //create one GFTrackCandidate for each CDCTrackCandidate

    //set the values needed as start values for the fit in the GFTrackCandidate from the CDCTrackCandidate information
    //variables stored in the GFTrackCandidates are: vertex position + error, momentum + error, pdg value, indices for the Hits

    TVector3 position;
    position.SetXYZ(0.0, 0.0, 0.0); //at the moment there is no vertex determination in the ConformalFinder, but maybe the origin or the innermost hit are good enough as start values...
    //position = cdcTrackCandidates[i]->getInnerMostHit().getWirePosition();

    TVector3 momentum =
      cdcLegendreTrackCandidates[i]->getMomentumEstimation();

    //Pattern recognition can determine only the charge, so here some dummy pdg value is set (with the correct charge), the pdg hypothesis can be then overwritten in the GenFitterModule
    int pdg = cdcLegendreTrackCandidates[i]->getChargeSign() * (211);

    //The initial covariance matrix is calculated from these errors and it is important (!!) that it is not completely wrong
    TVector3 posError;
    posError.SetXYZ(2.0, 2.0, 2.0);
    TVector3 momError;
    momError.SetXYZ(0.1, 0.1, 0.5);

    //set the start parameters
    gfTrackCandidates[i]->setComplTrackSeed(position, momentum, pdg,
                                            posError, momError);

    B2DEBUG(100, "Create GFTrackCandidate " << i << "  with pdg " << pdg);
    B2DEBUG(100,
            "position seed:  (" << position.x() << ", " << position.y() << ", " << position.z() << ")   position error: (" << posError.x() << ", " << posError.y() << ", " << posError.z() << ") ");
    B2DEBUG(100,
            "momentum seed:  (" << momentum.x() << ", " << momentum.y() << ", " << momentum.z() << ")   position error: (" << momError.x() << ", " << momError.y() << ", " << momError.z() << ") ");

    //find indices of the Hits
    vector<int> hitIndices;
    for (unsigned int j = 0;
         j < cdcLegendreTrackCandidates[i]->getTrackHits().size(); j++) {
      hitIndices.push_back(
        cdcLegendreTrackCandidates[i]->getTrackHits().at(j)->getStoreIndex());
    }

    //GenFit needs the hits in a correct order, here the hitIndices are sorted from small r to large r
    //it may also happen that there are two hits in the same layer (same r), their order is then chosen according to the curvature (charge) of the track
    double charge = TMath::Sign(1., gfTrackCandidates[i]->getQoverPseed());

    sortHits(hitIndices, "CDCLegendreTrackHits", charge);

    //now the correctly ordered hits can be added to the GFTrackCand

    B2DEBUG(100, " Add Hits: hitId rho planeId")

    for (unsigned int iter = 0; iter < hitIndices.size(); iter++) {
      int hitID = hitIndices.at(iter); //hit index
      float rho = cdcLegendreTrackHits[hitID]->getWirePosition().Mag(); //distance to the origin

      //for the DAF algorithm within GenFit it is important to assign a planeId to each hit
      //one can choose the layerId as the planeId, this would mean that hits from the same layer will 'compete' to be the 'best matching hit' in this layer
      //one can also give to each hit a unique planeId, so that e.g. two correct hits in the same layer get similar weights (without 'competition')
      //I am still not quite sure which way is the best one, this has to be tested...

      //int layerId = cdcTrackHits[hitID]->getLayerId();
      int uniqueId = cdcLegendreTrackHits[hitID]->getLayerId() * 10000
                     + cdcLegendreTrackHits[hitID]->getWireId(); //a value which should be unique for each hit

      gfTrackCandidates[i]->addHit(2, hitID, rho, uniqueId);
    }
    hitIndices.clear();
  }
}

void CDCLegendreTrackingModule::endRun()
{

}

void CDCLegendreTrackingModule::terminate()
{
  B2INFO("CDCTrackingModule: total number of found tracks: " << m_nTracks);

  delete[] cos_theta;
  delete[] sin_theta;

}

inline int CDCLegendreTrackingModule::calcBin(double r)
{
  return (int((r - m_rMin) / (m_rMax - m_rMin) * m_nbinsR));
}

inline double CDCLegendreTrackingModule::getBinCenter(int bin, double min,
                                                      double max, int nbins)
{
  return (min + (bin + 0.5) * (max - min) / nbins);
}

void CDCLegendreTrackingModule::sortHits(std::vector<int> & hitIndices,
                                         std::string CDCLegendreTrackHits, double charge)
{
  StoreArray<CDCLegendreTrackHit> cdcLegendreTrackHits(CDCLegendreTrackHits);

  vector<boost::tuple<int, double, int, double> > hitsToSort;

//fill the tuple with the information from the hits
  for (unsigned int i = 0; i < hitIndices.size(); i++) {
    int hitId = hitIndices.at(i);
    boost::tuple<int, double, int, double> newtouple(hitId,
                                                     cdcLegendreTrackHits[hitId]->getWirePosition().Mag(),
                                                     cdcLegendreTrackHits[hitId]->getWireId(), charge);
    hitsToSort.push_back(newtouple);
  }

  stable_sort(hitsToSort.begin(), hitsToSort.end(), tupleComp);

//refill the hitIndices with the correct ordered indices
  for (unsigned i = 0; i < hitIndices.size(); i++) {
    hitIndices.at(i) = hitsToSort.at(i).get<0>();
  }

}

bool CDCLegendreTrackingModule::tupleComp(
  boost::tuple<int, double, int, double> tuple1,
  boost::tuple<int, double, int, double> tuple2)
{

  bool result = true;

//the comparison function for the tuples created by the sort function

  if ((int) tuple1.get<1>() == (int) tuple2.get<1>()) {
    //special case: several hits in the same layer
//now we have to proceed differently for positive and negative tracks
//in a common case we just have to check the wireIds and decide the order according to the charge
//if however the track is crossing the wireId 0, we have again to treat it specially
//the number 100 is just a more or less arbitrary number, assuming that no track will be 'crossing' 100 different wireIds

    //in general this solution does not look very elegant, so if you have some suggestions how to improve it, do not hesitate to tell me

    if (tuple1.get<3>() < 0) {
      //negative charge

      //check for special case with wireId 0
      if (tuple1.get<2>() == 0 && tuple2.get<2>() > 100) {
        result = false;
      }
      if (tuple1.get<2>() > 100 && tuple2.get<2>() == 0) {
        result = true;
      }
      //'common' case
      if (tuple1.get<2>() < tuple2.get<2>()) {
        result = true;
      }
      if (tuple1.get<2>() > tuple2.get<2>()) {
        result = false;
      }
    } //end negative charge

    else {
      //positive charge

      //check for special case with wireId 0
      if (tuple1.get<2>() == 0 && tuple2.get<2>() > 100) {
        result = true;
      }
      if (tuple1.get<2>() > 100 && tuple2.get<2>() == 0) {
        result = false;
      }
      //'common' case
      if (tuple1.get<2>() < tuple2.get<2>()) {
        result = false;
      }
      if (tuple1.get<2>() > tuple2.get<2>()) {
        result = true;
      }
    } //end positive charge

  }

//usual case: hits sorted by the rho value
  else
    result = (tuple1.get<1>() < tuple2.get<1>());

  return result;

}

bool SortCandidatesByVotes::operator()(
  const std::pair<int, std::pair<int, int> > &lhs,
  const std::pair<int, std::pair<int, int> > &rhs) const
{
  return lhs.first > rhs.first;
}

