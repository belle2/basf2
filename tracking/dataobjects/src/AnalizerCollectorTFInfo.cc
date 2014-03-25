/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/AnalizerCollectorTFInfo.h>

#include <framework/gearbox/Const.h>

#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>

#include "tracking/vxdCaTracking/FilterID.h"



//C++ std lib
#include <utility>
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Belle2;

/**
 * Information about the Methodes in AnalizerCollectorTFInfo.h
 */

ClassImp(AnalizerCollectorTFInfo)

const string AnalizerCollectorTFInfo::file_separator = ",";


AnalizerCollectorTFInfo::AnalizerCollectorTFInfo()
{
  //m_output_flag = 0;

}

AnalizerCollectorTFInfo::~AnalizerCollectorTFInfo()
{
}


/** Sets all Particle IDs and real-Information from all clusters */
void AnalizerCollectorTFInfo::setAllParticleIDs(double boarder)
{

  // The Information about the ParticleID is stored in the svd & pxd Clusters
  // so to get to the Information the relative Position (stored in the TF-Clusters)
  // in this StoreArray is used
  StoreArray<ClusterTFInfo> clusterTFInfo("");
  int n_count = clusterTFInfo.getEntries();
  if (n_count == 0) {B2DEBUG(100, "VXDTF: Display: clusterTFInfo is empty!");}
  B2DEBUG(100, "setAllParticleIDs -n_count: " << n_count);

  StoreArray<SVDCluster> svdClusters("");
  int nsvdclusters = svdClusters.getEntries();
  if (nsvdclusters == 0) {B2DEBUG(100, "AnalizerCollectorTFInfo: svdClusters is empty!");}
  B2DEBUG(100, "setAllParticleIDs - nsvdclusters: " << nsvdclusters);

  StoreArray<PXDCluster> pxdClusters("");
  int npxdclusters = pxdClusters.getEntries();
  if (npxdclusters == 0) {B2DEBUG(100, "AnalizerCollectorTFInfo: pxdClusters is empty!");}
  B2DEBUG(100, "setAllParticleIDs - npxdclusters: " << npxdclusters);


  for (auto & akt_cluster : clusterTFInfo) {

    // Detector Type for svd/pxd difference
    int particleID = -1;
    bool is_real = 0;
    int pdgCode = 0;

    int detector_type = akt_cluster.getDetectorType();
    int pass_index = akt_cluster.getPassIndex();
    int cluster_id = akt_cluster.getRealClusterID();
    int relative_cluster_id = akt_cluster.getRelativeClusterID();

    B2DEBUG(100, "setAllParticleIDs - init: Pass_index: " << pass_index << ", ClusterID: " << cluster_id << "; Detector Type: " << detector_type << "; Relative Cluster ID: " << relative_cluster_id);


    if (relative_cluster_id == -1) {
      B2DEBUG(100, "setAllParticleIDs - No Relative Cluster Information found");
      continue;
    }

    if (detector_type == Const::SVD)  {

//     B2DEBUG(100, "getParticleIDfromRelations - start_index: " << start_index << "; cluster_id-start_index: " << (cluster_id-start_index));

      if (nsvdclusters != 0) {

        //int akt_index = cluster_id-start_index;
        if (relative_cluster_id >= nsvdclusters) {
          B2DEBUG(100, "setAllParticleIDs - No valid Cluster ID in svd: " << relative_cluster_id);
          continue;
        }

        const SVDCluster* aCluster = svdClusters[relative_cluster_id];

        RelationVector<MCParticle> mcParticleRelations = aCluster->getRelationsTo<MCParticle>();

        // Same procedure VXDSimpleClusterizerModule => first particle (should not have a second particleID)
        particleID = mcParticleRelations[0]->getIndex();
        is_real = mcParticleRelations[0]->hasStatus(MCParticle::c_PrimaryParticle);
        pdgCode = mcParticleRelations[0]->getPDG();

        // should not have a second particleID
        if (mcParticleRelations.size() > 1) {
          B2DEBUG(100, "2nd Particle ID !!!: " << particleID);
        }

      }
    }

    // If PXD
    if (detector_type == Const::PXD) {

//     B2DEBUG(100, "getParticleIDfromRelations - start_index: " << start_index << "; cluster_id-start_index: " << (cluster_id-start_index));

      StoreArray<PXDCluster> pxdClusters("");
      int npxdclusters = pxdClusters.getEntries();
      if (npxdclusters != 0) {

//  int akt_index = cluster_id-start_index;
        if (relative_cluster_id >= npxdclusters) {
          B2DEBUG(100, "setAllParticleIDs - No valid Cluster ID in pxd: " << relative_cluster_id);
          continue;
        }

        // ID of PXD Clusters is cluster_id - Start of cluster ids for pxd
        const PXDCluster* aCluster = pxdClusters[relative_cluster_id];

        RelationVector<MCParticle> mcParticleRelations = aCluster->getRelationsTo<MCParticle>();

        // Same procedure VXDSimpleClusterizerModule => first particle (should not have a second particleID)
        particleID = mcParticleRelations[0]->getIndex();
        is_real = mcParticleRelations[0]->hasStatus(MCParticle::c_PrimaryParticle);
        pdgCode = mcParticleRelations[0]->getPDG();

        // should not have a second particleID
        if (mcParticleRelations.size() > 1) {
          B2DEBUG(100, "2nd Particle ID !!!: " << particleID);
        }

      }

    }

    B2DEBUG(100, "setAllParticleIDs - particleID : " << particleID << "; is real: " << is_real);
    akt_cluster.setParticleID(particleID);
    akt_cluster.setIsReal(is_real);
    akt_cluster.setPDG(pdgCode);

  }

  // Hits, Cells & TCand have to used seperated from each other to get the Information for all Objects

  // Hits real or not real
  StoreArray<HitTFInfo> hitTFInfo("");
  n_count = hitTFInfo.getEntries();
  if (n_count == 0) {B2DEBUG(100, "VXDTF: Display: hitTFInfo is empty!");}

  for (int i = 0; i < hitTFInfo.getEntries(); i++) {
    int akt_is_real[] = {0, 0};
    std::vector<int> particle_ids;

    for (auto & akt_cluster : hitTFInfo[i]->getAssignedCluster()) {
      akt_is_real[clusterTFInfo[akt_cluster]->getIsReal()]++;
      particle_ids.push_back(clusterTFInfo[akt_cluster]->getParticleID());
    }

    // 0 = false, 1 = true, 2 = part true
    int is_real = 0;
    if (akt_is_real[0] > 0) {
      if (akt_is_real[1] == 0) {
        is_real = 0;  // false
      } else {
        is_real = 2;  // part
      }
    } else if (akt_is_real[1] > 0) {
      is_real = 1;    // true
    } else {
      is_real = 2;  // part
    }

    hitTFInfo[i]->setIsReal(is_real);

    // Particle List with purity
    for (uint z = 0; z < particle_ids.size(); z++) {
      // if not already added to the particle list
      if (std::count(particle_ids.begin(), particle_ids.begin() + z, particle_ids[z]) == 0) {
        int anz_particleid = std::count(particle_ids.begin(), particle_ids.end(), particle_ids[z]);
        hitTFInfo[i]->push_back_UsedParticles(std::make_pair(particle_ids[z], ((double)anz_particleid / particle_ids.size())));
        B2DEBUG(100, "ParticleID: " << particle_ids[z] << "; purity: " << ((double)anz_particleid / particle_ids.size()));
      }
    }

    B2DEBUG(100, "setAllParticleIDs - hitID : " << i << "; is real: " << hitTFInfo[i]->getIsReal());
  }


  // Cell real or not real
  StoreArray<CellTFInfo> cellTFInfo("");
  n_count = cellTFInfo.getEntries();
  if (n_count == 0) {B2DEBUG(100, "VXDTF: Display: cellTFInfo is empty!");}

  for (int i = 0; i < cellTFInfo.getEntries(); i++) {
    int akt_is_real[] = {0, 0};
    std::vector<int> particle_ids;

    for (auto & akt_hits : cellTFInfo[i]->getAssignedHits()) {
      if (akt_hits != -1) {

        for (auto & akt_cluster : hitTFInfo[akt_hits]->getAssignedCluster()) {
          akt_is_real[clusterTFInfo[akt_cluster]->getIsReal()]++;
          particle_ids.push_back(clusterTFInfo[akt_cluster]->getParticleID());

//    B2INFO ("* " << clusterTFInfo[akt_cluster]->getParticleID());
        }

      }
    }

    // 0 = false, 1 = true, 2 = part true
    uint is_real = 0;
    if (akt_is_real[0] > 0) {
      if (akt_is_real[1] == 0) {
        is_real = 0;  // false
      } else {
        is_real = 2;  // part
      }
    } else if (akt_is_real[1] > 0) {
      is_real = 1;    // true
    } else {
      is_real = 2;  // part
    }
    cellTFInfo[i]->setIsReal(is_real);

    // Particle List with purity
    for (uint z = 0; z < particle_ids.size(); z++) {
      // if not already added to the particle list
//       B2INFO ("ParticleID ? " << particle_ids[z]);
      if (std::count(particle_ids.begin(), particle_ids.begin() + z, particle_ids[z]) == 0) {
        int anz_particleid = std::count(particle_ids.begin(), particle_ids.end(), particle_ids[z]);
        cellTFInfo[i]->push_back_UsedParticles(std::make_pair(particle_ids[z], ((double)anz_particleid / particle_ids.size())));
        B2DEBUG(100, "ParticleID: " << particle_ids[z] << "; purity: " << ((double)anz_particleid / particle_ids.size()));
      }
    }

    B2DEBUG(100, "setAllParticleIDs - CellID : " << i << "; is real: " << cellTFInfo[i]->getIsReal());
  }


  // TCand. ghost, clean or contaminated
  StoreArray<TrackCandidateTFInfo> tfcandTFInfo("");
  n_count = tfcandTFInfo.getEntries();
  if (n_count == 0) {B2DEBUG(100, "VXDTF: Display: tfcandTFInfo is empty!");}

  for (int i = 0; i < tfcandTFInfo.getEntries(); i++) {
    int akt_is_real[] = {0, 0};
    std::vector<int> particle_ids;

    for (auto & akt_cells : tfcandTFInfo[i]->getAssignedCell()) {
      if (akt_cells != -1) {

        for (auto & akt_hits : cellTFInfo[akt_cells]->getAssignedHits()) {

          if (akt_hits != -1) {

            for (auto & akt_cluster : hitTFInfo[akt_hits]->getAssignedCluster()) {
              akt_is_real[clusterTFInfo[akt_cluster]->getIsReal()]++;
              particle_ids.push_back(clusterTFInfo[akt_cluster]->getParticleID());
            }
          }

        }

      }
    }

    // Particle List with purity
    for (uint z = 0; z < particle_ids.size(); z++) {
      // if not already added to the particle list
      if (std::count(particle_ids.begin(), particle_ids.begin() + z, particle_ids[z]) == 0) {
        int anz_particleid = std::count(particle_ids.begin(), particle_ids.end(), particle_ids[z]);
        tfcandTFInfo[i]->push_back_UsedParticles(std::make_pair(particle_ids[z], ((double)anz_particleid / particle_ids.size())));

        B2DEBUG(100, "ParticleID: " << particle_ids[z] << "; purity: " << ((double)anz_particleid / particle_ids.size()));
      }
    }

    // 0 = ghost TC
    // 1 = clean TC
    // 2 = contaminated TC
    uint real_state = 3;

    if (akt_is_real[0] > 0) {
      if (akt_is_real[1] == 0) {
        real_state = 0;   // ghost
      }
    } else if (particle_ids.size() == 1) {
      real_state = 1;   // clean
    } else if (tfcandTFInfo[i]->getMainParticle().second > boarder) {
      real_state = 2;   // contaminated
    } else {
      real_state = 0;   // ghost
    }

    tfcandTFInfo[i]->setIsReal(real_state);

    B2DEBUG(100, "setAllParticleIDs - TfCand : " << i << "; is real: " << tfcandTFInfo[i]->getIsReal());
  }

}


/** Stores All Hit Information in a file */
void AnalizerCollectorTFInfo::storeAllHitInformation(std::string filename)
{
  storeHitInformation(filename, -1);
}


/** Stores Hit Information of 1 Particle in a file */
// Information of the Hits:
// 0  hitid   int
// 1  pass_index  int
// 2  sector_id int
// 3  diet_at   string
// 4  diet_at_ID  int
// 5  real    int
// 6  MainParticleID  int
// 7  Purity    double
// 8  Hit_Position  double / double / double
// 9  Hit_Sigma double / double / double
// 10   FilterID::anglesRZ,
// 11 FilterID::anglesXY,
// 12 FilterID::distance3D,
// 13 FilterID::distanceXY,
// 14 FilterID::distanceZ,
// 15 FilterID::helixFit,
// 16 FilterID::slopeRZ,
// 17   FilterID::deltaSlopeRZ,
// 18 FilterID::pT,
// 19 FilterID::deltapT,
// 20 FilterID::normedDistance3D,
// 21 FilterID::distance2IP,
// 22 FilterID::deltaDistance2IP
// 23 FilterID::silentSegFinder
// 24   FilterID::silentTcc
// 24   Clusters size int
// 24   ClusterID 1 int
// 24   ClusterID 2 int

void AnalizerCollectorTFInfo::storeHitInformation(std::string filename, int particle_id_filter)
{
  std::vector<int> searchfilters = { FilterID::angles3D,
                                     FilterID::anglesRZ,
                                     FilterID::anglesXY,
                                     FilterID::distance3D,
                                     FilterID::distanceXY,
                                     FilterID::distanceZ,
                                     FilterID::helixFit,
                                     FilterID::slopeRZ,
                                     FilterID::deltaSlopeRZ,
                                     FilterID::pT,
                                     FilterID::deltapT,
                                     FilterID::normedDistance3D,
                                     FilterID::distance2IP,
                                     FilterID::deltaDistance2IP,
                                     FilterID::silentSegFinder,
                                     FilterID::silentTcc
                                   };

  StoreArray<HitTFInfo> hitTFInfo("");
  uint n_count = hitTFInfo.getEntries();
  if (n_count == 0) {B2DEBUG(100, "VXDTF: Display: hitTFInfo is empty!");}

  ofstream myfile(filename);

  string particle_text;
  if (particle_id_filter == -1) {
    particle_text = "MainParticleID";
  } else {
    particle_text = "ParticleID_param";
  }

  B2DEBUG(100, "Filename (storeHitInformation): " << filename);

  myfile << "hitid" << file_separator << "pass_index" << file_separator << "sector_id" << file_separator << "diet_at" << file_separator << "diet_ID" << file_separator << "real" << file_separator << particle_text <<
         file_separator << "Purity" << file_separator << "Hit_Position" << file_separator << "Hit_Sigma" << file_separator << "FilterID::anglesRZ" << file_separator << "FilterID::anglesXY" << file_separator << "FilterID::distance3D" << file_separator << "FilterID::distanceXY" << file_separator << "FilterID::distanceZ" << file_separator << "FilterID::helixFit" << file_separator << "FilterID::slopeRZ" << file_separator << "FilterID::deltaSlopeRZ" << file_separator <<  "FilterID::pT" << file_separator << "FilterID::deltapT" << file_separator << "FilterID::normedDistance3D" << file_separator << "FilterID::distance2IP" << file_separator << "FilterID::deltaDistance2IP" << file_separator << "FilterID::silentSegFinder" << file_separator << "FilterID::silentTcc" << file_separator << "Clusters_Size" << file_separator << "ClusterID_1" << file_separator << "ClusterID_2" << endl;


  for (int i = 0; i <  hitTFInfo.getEntries(); i++) {

    // particle_id_filter == -1 => all Entries
    if (particle_id_filter == -1 || hitTFInfo[i]->containsParticle(particle_id_filter)) {

      myfile << i << file_separator << hitTFInfo[i]->getPassIndex() << file_separator << hitTFInfo[i]->getSectorID() << file_separator << hitTFInfo[i]->getDiedAt() << file_separator << hitTFInfo[i]->getDiedID() << file_separator << hitTFInfo[i]->getIsReal();

      // Main Particle & Purity
      if (particle_id_filter == -1) {
        myfile << file_separator << hitTFInfo[i]->getMainParticle().first << file_separator << hitTFInfo[i]->getMainParticle().second;
      } else {
        myfile << file_separator << hitTFInfo[i]->getInfoParticle(particle_id_filter).first << file_separator << hitTFInfo[i]->getInfoParticle(particle_id_filter).second;
      }

      myfile << file_separator << hitTFInfo[i]->getPosition().X() << "/" << hitTFInfo[i]->getPosition().Y() << "/" << hitTFInfo[i]->getPosition().Z();

      myfile << file_separator << hitTFInfo[i]->getHitSigma().X() << "/" << hitTFInfo[i]->getHitSigma().Y() << "/" << hitTFInfo[i]->getHitSigma().Z();

      std::vector<int> accepted = hitTFInfo[i]->getAccepted();
      std::vector<int> rejected = hitTFInfo[i]->getRejected();

      B2DEBUG(100, "SaveHitInformation: i: " << i << ", pass index: " << hitTFInfo[i]->getPassIndex() << ", size accepted: " <<
              accepted.size() << ", size rejected: " << rejected.size());

      /*    for (auto &akt_filter: rejected) {
          B2DEBUG(100,"rejected: " << akt_filter);
          }
      */
      for (uint u = 0; u < searchfilters.size(); u++) {
        //B2DEBUG(100,"Filter ?: " << searchfilters[u]);
        myfile << file_separator;
        //B2DEBUG(100,"searchfilters: " << searchfilters[u]);

        if (std::find(accepted.begin(), accepted.end(), searchfilters[u]) != accepted.end()) {
          myfile << 1;      //true = accepted
        } else if (std::find(rejected.begin(), rejected.end(), searchfilters[u]) != rejected.end()) {
          myfile << 0;      //false = rejected
        } else {
          //myfile << 2;     //not found
        }

      }


      std::vector<int> assignedclusters = hitTFInfo[i]->getAssignedCluster();

      myfile << file_separator << assignedclusters.size();

      // all the time max 2 Cluster IDs
      for (uint m = 0; m < 2; m++) {

        if (m < assignedclusters.size()) {
          myfile << file_separator << assignedclusters[m];
        } else {
          myfile << file_separator;
        }

      }



      myfile << endl;
    }
  }

  myfile.close();
}


/** Stores All Cell Information in a file */
void AnalizerCollectorTFInfo::storeAllCellInformation(std::string filename)
{
  storeCellInformation(filename, -1);
}



/** Stores Cell Information (with one particleID) in a file */
// Information of the Hits:
// 0  cellid    int
// 1  pass_index  int
// 2  state   int
// 3  diet_at   string
// 4  diet_at_ID  int
// 5  real    int
// 6  MainParticleID  int
// 7  Purity    double
// 8  FilterID::distance3D,
// 9  FilterID::anglesXY,
// 10   FilterID::anglesRZ,
// 11 FilterID::distance2IP,
// 12   FilterID::deltaSlopeRZ,
// 13 FilterID::pT,
// 14 FilterID::helixFit
// 15 FilterID::nbFinderLost,
// 16 FilterID::cellularAutomaton,
// 17 FilterID::silentTcc
// 18   outer hit
// 19   inner Hit
//

void AnalizerCollectorTFInfo::storeCellInformation(std::string filename, int particle_id_filter)
{
  // nbfinder, CA
  std::vector<int> searchfilters = { FilterID::angles3D,
                                     FilterID::anglesXY,
                                     FilterID::anglesRZ,
                                     FilterID::distance2IP,
                                     FilterID::deltaSlopeRZ,
                                     FilterID::pT,
                                     FilterID::helixFit,
                                     FilterID::nbFinderLost,
                                     FilterID::cellularAutomaton,
                                     FilterID::silentTcc
                                   };

  ofstream myfile(filename);

  StoreArray<CellTFInfo> cellTFInfo("");
  uint n_count = cellTFInfo.getEntries();
  if (n_count == 0) {B2DEBUG(100, "VXDTF: Display: cellTFInfo is empty!");}

  string particle_text;
  if (particle_id_filter == -1) {
    particle_text = "MainParticleID";
  } else {
    particle_text = "ParticleID_param";
  }


  myfile << "cellid" << file_separator << "pass_index" << file_separator << "state" << file_separator << "diet_at" << file_separator << "diet_ID" << file_separator << " real" << file_separator << particle_text <<
         file_separator << "Purity" << file_separator << "FilterID::distance3D" << file_separator << "FilterID::anglesXY" << file_separator << "FilterID::anglesRZ" << file_separator << "FilterID::distance2IP" << file_separator << "FilterID::deltaSlopeRZ" << file_separator << "FilterID::pT" << file_separator << "FilterID::helixFit" << file_separator << "FilterID::nbFinderLost" << file_separator <<  "FilterID::cellularAutomaton" << file_separator << "FilterID::silentTcc" << file_separator << "outer hit" << file_separator << "inner Hit" << endl;

  for (int i = 0; i <  cellTFInfo.getEntries(); i++) {

    // particle_id_filter == -1 => all Entries
    if (particle_id_filter == -1 || cellTFInfo[i]->containsParticle(particle_id_filter)) {

      myfile << i << file_separator << cellTFInfo[i]->getPassIndex() << file_separator << cellTFInfo[i]->getState() << file_separator << cellTFInfo[i]->getDiedAt() << file_separator << cellTFInfo[i]->getDiedID() << file_separator << cellTFInfo[i]->getIsReal();

      // Main Particle & Purity
      if (particle_id_filter == -1) {
        myfile << file_separator << cellTFInfo[i]->getMainParticle().first << file_separator << cellTFInfo[i]->getMainParticle().second;
      } else {
        myfile << file_separator << cellTFInfo[i]->getInfoParticle(particle_id_filter).first << file_separator << cellTFInfo[i]->getInfoParticle(particle_id_filter).second;
      }

      std::vector<int> accepted = cellTFInfo[i]->getAccepted();
      std::vector<int> rejected = cellTFInfo[i]->getRejected();

      B2DEBUG(100, "SaveCellInformation: i: " << i << ", pass index: " << cellTFInfo[i]->getPassIndex() << ", size accepted: " <<
              accepted.size() << ", size rejected: " << rejected.size());

      for (uint u = 0; u < searchfilters.size(); u++) {
        //B2DEBUG(100,"Filter ?: " << searchfilters[u]);
        myfile << file_separator;

        if (std::find(accepted.begin(), accepted.end(), searchfilters[u]) != accepted.end()) {
          myfile << 1;      //true = accepted
        } else if (std::find(rejected.begin(), rejected.end(), searchfilters[u]) != rejected.end()) {
          myfile << 0;      //false = rejected
        } else {
          //myfile << 2;     //not found
        }

      }

      // outer hit  = Index 0, inner hit = Index 1
      myfile << file_separator << cellTFInfo[i]->getAssignedHits()[0];
      myfile << file_separator << cellTFInfo[i]->getAssignedHits()[1];

      myfile << endl;

    }
  }

  myfile.close();
}


/** Stores All TC Information in a file */
void AnalizerCollectorTFInfo::storeAllTCInformation(std::string filename)
{
  storeTCInformation(filename, -1);
}



/** Stores TCand Information in a file (with one particle_id) */
// Information of the Hits:
// 0  tcid    int
// 1  pass_index  int
// 2  ownid   int
// 3  diet_at   string
// 4  diet_at_id  int
// 5  real    int
// 6  MainParticleID  int
// 7  Purity    double
// 8  FilterID::hopfield,
// 9    FilterID::greedy,
// 10   FilterID::tcDuel,
// 11   FilterID::tcFinderCurr,
// 12   FilterID::ziggZaggXY,
// 13   FilterID::deltapT,
// 14   FilterID::deltaDistance2IP,
// 15   FilterID::ziggZaggRZ,
// 16   FilterID::calcQIbyKalman,
// 17   FilterID::overlapping,
// 18   FilterID::circlefit
// 19   Count Asso. Cell IDs
// 20-29 ... Cell IDs

void AnalizerCollectorTFInfo::storeTCInformation(std::string filename, int particle_id_filter)
{
  // 10 filters?
  std::vector<int> searchfilters = {
    FilterID::hopfield,
    FilterID::greedy,
    FilterID::tcDuel,
    FilterID::tcFinderCurr,
    FilterID::ziggZaggXY,
    FilterID::deltapT,
    FilterID::deltaDistance2IP,
    FilterID::ziggZaggRZ,
    FilterID::calcQIbyKalman,
    FilterID::overlapping,
    FilterID::circlefit
  };

  ofstream myfile(filename);

  StoreArray<TrackCandidateTFInfo> tfcandTFInfo("");
  uint n_count = tfcandTFInfo.getEntries();
  if (n_count == 0) {B2DEBUG(100, "VXDTF: Display: tfcandTFInfo is empty!");}

  string particle_text;
  if (particle_id_filter == -1) {
    particle_text = "MainParticleID";
  } else {
    particle_text = "ParticleID_param";
  }

  myfile << "tcid" << file_separator << "pass_index" << file_separator << "ownid" << file_separator << "diet_at" << file_separator << "diet_ID" << file_separator << "real" << file_separator << particle_text <<
         file_separator << "Purity" << file_separator << "FilterID::hopfield" << file_separator << "FilterID::greedy" << file_separator << "FilterID::tcDuel" << file_separator << "FilterID::tcFinderCurr" << file_separator << "FilterID::ziggZaggXY" << file_separator << "FilterID::deltapT" << file_separator << "FilterID::deltaDistance2IP" << file_separator << "FilterID::ziggZaggRZ" << file_separator <<  "FilterID::calcQIbyKalman" << file_separator << "FilterID::overlapping" << file_separator << "FilterID::circlefit" << file_separator << "Count Asso. Cell IDs" << file_separator << "Cell ID 1" << file_separator << "Cell ID 2" << file_separator << "Cell ID 3" << file_separator << "Cell ID 4" << file_separator << "Cell ID 5" << file_separator << "Cell ID 6" << file_separator
         << "Cell ID 7" << file_separator << "Cell ID 8" << file_separator << "Cell ID 9" << file_separator << "Cell ID 10" << endl;

  for (int i = 0; i <  tfcandTFInfo.getEntries(); i++) {

    // particle_id_filter == -1 => all Entries
    if (particle_id_filter == -1 || tfcandTFInfo[i]->containsParticle(particle_id_filter)) {

      myfile << i << file_separator << tfcandTFInfo[i]->getPassIndex() << file_separator << tfcandTFInfo[i]->getOwnID() << file_separator << tfcandTFInfo[i]->getDiedAt() << file_separator << tfcandTFInfo[i]->getDiedID() << file_separator << tfcandTFInfo[i]->getIsReal();

      // Main Particle & Purity
      if (particle_id_filter == -1) {
        myfile << file_separator << tfcandTFInfo[i]->getMainParticle().first << file_separator << tfcandTFInfo[i]->getMainParticle().second;
      } else {
        myfile << file_separator << tfcandTFInfo[i]->getInfoParticle(particle_id_filter).first << file_separator << tfcandTFInfo[i]->getInfoParticle(particle_id_filter).second;
      }

      std::vector<int> accepted = tfcandTFInfo[i]->getAccepted();
      std::vector<int> rejected = tfcandTFInfo[i]->getRejected();

      B2DEBUG(100, "SaveTCInformation: i: " << i << ", pass index: " << tfcandTFInfo[i]->getPassIndex() << ", size accepted: " <<
              accepted.size() << ", size rejected: " << rejected.size());

      for (uint u = 0; u < searchfilters.size(); u++) {
        //B2DEBUG(100,"Filter ?: " << searchfilters[u]);
        myfile << file_separator;

        if (std::find(accepted.begin(), accepted.end(), searchfilters[u]) != accepted.end()) {
          myfile << 1;      //true = accepted
        } else if (std::find(rejected.begin(), rejected.end(), searchfilters[u]) != rejected.end()) {
          myfile << 0;      //false = rejected
        } else {
          //myfile << 2;     //not found
        }

      }

      std::vector<int> assignedcells = tfcandTFInfo[i]->getAssignedCell();

      myfile << file_separator << assignedcells.size();

      // all the time first 10 Cell IDs
      for (uint m = 0; m < 10; m++) {

        if (m < assignedcells.size()) {
          myfile << file_separator << assignedcells[m];
        } else {
          myfile << file_separator;
        }

      }

      myfile << endl;
    }
  }

  myfile.close();
}




/** Stores Clusters Information in a file */
// Information of the Clusters:
// 0  ClusterID   int
// 1  pass_index    int
// 2  Relative Cluster ID int
// 3  diet_at     string
// 4  diet_at_id    int
// 5  real      int
// 6  Detector Type   int
// 7  ParticleID    int

void AnalizerCollectorTFInfo::storeClustersInformation(std::string filename)
{

  ofstream myfile(filename);

  StoreArray<ClusterTFInfo> clusterTFInfo("");
  uint n_count = clusterTFInfo.getEntries();
  if (n_count == 0) {B2DEBUG(100, "AnalizerCollectorTFInfo: Display: clusterTFInfo is empty!");}

  myfile << "ClusterID" << file_separator << "pass_index" << file_separator << "Relative_Cluster_ID" << file_separator << "diet_at" << file_separator << "diet_ID" << file_separator << "real" << file_separator << "Detector_Type" << file_separator << "Particle_ID" << file_separator << "PDG" << endl;

  for (int i = 0; i <  clusterTFInfo.getEntries(); i++) {

    myfile << i << file_separator << clusterTFInfo[i]->getPassIndex() << file_separator << clusterTFInfo[i]->getRelativeClusterID() << file_separator << clusterTFInfo[i]->getDiedAt() << file_separator << clusterTFInfo[i]->getDiedID() << file_separator << clusterTFInfo[i]->getIsReal() << file_separator <<  clusterTFInfo[i]->getDetectorType() << file_separator << clusterTFInfo[i]->getParticleID() << file_separator << clusterTFInfo[i]->getPDG() << endl;

  }

  myfile.close();
}



/** Stores Sector Information in a file */
// Information of the Sector:
// 0  SectorID    int
// 1  pass_index    int
// 2  Real Sector ID    int
// 3  diet_at     string
// 4  diet_at_id    int
// 5  Friends only    int
// 6  Point 1     double / double / double
// 7  Point 2     double / double / double
// 8  Point 3     double / double / double
// 9  Point 4     double / double / double
// 10 Friend Sectors Count  int
// 11-25 Friend Sector IDs  int

void AnalizerCollectorTFInfo::storeSectorInformation(std::string filename, bool with_friends)
{

  ofstream myfile(filename);

  StoreArray<SectorTFInfo> sectorTFInfo("");
  int n_count = sectorTFInfo.getEntries();
  if (n_count == 0) {B2DEBUG(100, "AnalizerCollectorTFInfo: Display: sectorTFInfo is empty!");}

  myfile << "SectorID" << file_separator << "pass_index" << file_separator << "Real_Sector_ID" << file_separator << "diet_at" << file_separator << "diet_ID" << file_separator << "Friends_only" << file_separator << "Point_1" << file_separator << "Point_2" << file_separator << "Point_3" << file_separator << "Point_4" << file_separator << "Friends_Sector_IDs";

  for (uint m = 0; m < 15; m++) {
    myfile << file_separator << "Sector ID " << (m + 1);
  }

  myfile << endl;

  for (int i = 0; i <  sectorTFInfo.getEntries(); i++) {

    if (with_friends || (!sectorTFInfo[i]->getIsOnlyFriend())) {

      myfile << i << file_separator << sectorTFInfo[i]->getPassIndex() << file_separator << sectorTFInfo[i]->getSectorID() << file_separator << sectorTFInfo[i]->getDiedAt() << file_separator << sectorTFInfo[i]->getDiedID() << file_separator << sectorTFInfo[i]->getIsOnlyFriend() << file_separator <<  sectorTFInfo[i]->getPoint(0).X() << "/" << sectorTFInfo[i]->getPoint(0).Y() << "/" << sectorTFInfo[i]->getPoint(0).Z() << file_separator << sectorTFInfo[i]->getPoint(1).X() << "/" << sectorTFInfo[i]->getPoint(1).Y() << "/" << sectorTFInfo[i]->getPoint(1).Z() << file_separator <<  sectorTFInfo[i]->getPoint(2).X() << "/" << sectorTFInfo[i]->getPoint(2).Y() << "/" << sectorTFInfo[i]->getPoint(2).Z() << file_separator << sectorTFInfo[i]->getPoint(3).X() << "/" << sectorTFInfo[i]->getPoint(3).Y() << "/" << sectorTFInfo[i]->getPoint(3).Z();

      std::vector<int> friend_sectors = sectorTFInfo[i]->getFriends();

      myfile << file_separator << friend_sectors.size();

      // all the time first 15 Sector Friends
      for (uint m = 0; m < 15; m++) {

        if (m < friend_sectors.size()) {
          myfile << file_separator << friend_sectors[m];
        } else {
          myfile << file_separator;
        }

      }

      myfile << endl;

    }
  }

  myfile.close();
}

