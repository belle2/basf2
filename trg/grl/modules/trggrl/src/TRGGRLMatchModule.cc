/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI, Yun-Tsung Lai, Junhao Yin                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// include
#include <trg/grl/modules/trggrl/TRGGRLMatchModule.h>
#include <trg/grl/dataobjects/TRGGRLMATCH.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/ecl/dataobjects/TRGECLCluster.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

//framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>


//utilities
#include <framework/utilities/FileSystem.h>

#include <boost/foreach.hpp>
#include <fstream>
#include <stdlib.h>
#include <iostream>


using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TRGGRLMatch)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TRGGRLMatchModule::TRGGRLMatchModule() : Module()
{
  // Set module properties
  setDescription("match CDC trigger tracks and ECL trigger clusters");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("SimulationMode", m_simulationMode, "TRGGRL simulation switch", 1);
  addParam("FastSimulationMode", m_fastSimulationMode, "TRGGRL fast simulation mode", m_fastSimulationMode);
  addParam("FirmwareSimulationMode", m_firmwareSimulationMode, "TRGGRL firmware simulation mode", m_firmwareSimulationMode);

  addParam("DrMatch", m_dr_threshold, "the threshold of dr between track and cluster if they are matched successfully", 25.);
  addParam("DzMatch", m_dz_threshold, "the threshold of dz between track and cluster if they are matched successfully", 30.);
  addParam("2DtrackCollection", m_2d_tracklist, "the 2d track list used in the match", std::string("TRGCDC2DFinderTracks"));
  addParam("3DtrackCollection", m_3d_tracklist, "the 3d track list used in the match", std::string("TRGCDCNeuroTracks"));
  addParam("TRGECLClusterCollection", m_clusterlist, "the cluster list used in the match", std::string("TRGECLClusters"));
  addParam("2DmatchCollection", m_2dmatch_tracklist, "the 2d tracklist with associated cluster", std::string("TRG2DMatchTracks"));
  addParam("3DmatchCollection", m_3dmatch_tracklist, "the 3d NN tracklist with associated cluster", std::string("TRG3DMatchTracks"));


}

TRGGRLMatchModule::~TRGGRLMatchModule()
{
}

void TRGGRLMatchModule::initialize()
{
  B2INFO("TRGGRLMatchModule processing");
  StoreArray<CDCTriggerTrack> track2Dlist(m_2d_tracklist);
  StoreArray<CDCTriggerTrack> track3Dlist(m_3d_tracklist);
  track2Dlist.isRequired();
  track3Dlist.isRequired();
  StoreArray<TRGECLCluster> clusterslist(m_clusterlist);
  clusterslist.isRequired();
  clusterslist.registerRelationTo(track2Dlist);
  clusterslist.registerRelationTo(track3Dlist);
//  track2Dlist.registerRelationTo(clusterslist);
// track3Dlist.registerRelationTo(clusterslist);

  StoreArray<TRGGRLMATCH>::registerPersistent(m_2dmatch_tracklist);
//  StoreArray<TRGGRLMATCH>::register(m_2dmatch_tracklist);
  StoreArray<TRGGRLMATCH>::registerPersistent(m_3dmatch_tracklist);

  StoreArray<TRGGRLMATCH> track2Dmatch(m_2dmatch_tracklist);
  track2Dmatch.registerRelationTo(track2Dlist);
  track2Dmatch.registerRelationTo(clusterslist);
  StoreArray<TRGGRLMATCH> track3Dmatch(m_3dmatch_tracklist);
  track3Dmatch.registerRelationTo(clusterslist);
  track3Dmatch.registerRelationTo(track3Dlist);



}

void TRGGRLMatchModule::beginRun()
{
}

void TRGGRLMatchModule::event()
{

  StoreArray<CDCTriggerTrack> track2Dlist(m_2d_tracklist);
  StoreArray<CDCTriggerTrack> track3Dlist(m_3d_tracklist);
  StoreArray<TRGECLCluster> clusterlist(m_clusterlist);
  StoreArray<TRGGRLMATCH> track2Dmatch(m_2dmatch_tracklist);
  StoreArray<TRGGRLMATCH> track3Dmatch(m_3dmatch_tracklist);

//do 2d track match with cluster
  for (int i = 0; i < track2Dlist.getEntries(); i++) {

    double dr_tmp = 99999.;
    int cluster_ind = -1;
    for (int j = 0; j < clusterlist.getEntries(); j++) {
      double ds_ct[2] = {99999., 99999.};
      calculationdistance(track2Dlist[i], clusterlist[j], ds_ct, 0);
      if (dr_tmp > ds_ct[0]) {
        dr_tmp = ds_ct[0];
        cluster_ind = j;
      }
    }

    if (dr_tmp < m_dr_threshold && cluster_ind != -1) {
      TRGGRLMATCH* mat2d = track2Dmatch.appendNew();
      mat2d->setDeltaR(dr_tmp);
      mat2d->addRelationTo(track2Dlist[i]);
      mat2d->addRelationTo(clusterlist[cluster_ind]);
      //   track2Dlist[i]->addRelationTo(clusterlist[cluster_ind]);
      clusterlist[cluster_ind]->addRelationTo(track2Dlist[i]);
    }
  }


//do 3d track match with cluster
  for (int i = 0; i < track3Dlist.getEntries(); i++) {

    double dr_tmp = 99999.;
    double dz_tmp = 99999.;
    int cluster_ind = -1;
    for (int j = 0; j < clusterlist.getEntries(); j++) {
      double ds_ct[2] = {99999., 99999.};
      calculationdistance(track3Dlist[i], clusterlist[j], ds_ct, 1);
      if (dr_tmp > ds_ct[0]) {
        dr_tmp = ds_ct[0];
        dz_tmp = ds_ct[1];
        cluster_ind = j;
      }
    }
    if (dr_tmp < m_dr_threshold && dz_tmp < m_dz_threshold && cluster_ind != -1) {
      TRGGRLMATCH* mat3d = track3Dmatch.appendNew();
      mat3d->setDeltaR(dr_tmp);
      mat3d->setDeltaZ(dz_tmp);
      mat3d->addRelationTo(track3Dlist[i]);
      mat3d->addRelationTo(clusterlist[cluster_ind]);
      // if(mat3d->getRelatedTo<CDCTriggerTrack>())std::cout<<"get match-track3D" <<std::endl;
      //  track3Dlist[i]->addRelationTo(clusterlist[cluster_ind]);
      // if(track3Dlist[i]->getRelatedTo<TRGECLCluster>())std::cout<<"get trk-cluster" <<std::endl;
      clusterlist[cluster_ind]->addRelationTo(track3Dlist[i]);
      //if(clusterlist[cluster_ind]->getRelatedTo<CDCTriggerTrack>())std::cout<<"get cluster-trk" <<std::endl;
      //if(track3Dlist[i]->getRelatedFrom<TRGECLCluster>())std::cout<<"from trk-cluster" <<std::endl;
    }
  }



}

void TRGGRLMatchModule::endRun()
{
}

void TRGGRLMatchModule::terminate()
{
}


void TRGGRLMatchModule::calculationdistance(CDCTriggerTrack* _track, TRGECLCluster* _cluster, double* ds, int _match3D)
{

//double    _pt = _track->getTransverseMomentum(1.5);
  double    _r = abs(1.0 / _track->getOmega()) ;
  double    _phi = _track->getPhi0() ;

  //-- cluster/TRGECL information
  double    _cluster_x = _cluster->getPositionX();
  double    _cluster_y = _cluster->getPositionY();
  double    _cluster_z = _cluster->getPositionZ();
  double    _R = sqrt(_cluster_x * _cluster_x + _cluster_y * _cluster_y);
//double    _D = sqrt(_cluster_x * _cluster_x + _cluster_y * _cluster_y + _cluster_z * _cluster_z);
//double    _re_scaled_p = _pt * _D / _R;

  //-- calculation
  if (_R > (2 * _r)) {
    ds[0] = 99999.;
  } else {
    //  double theta0 = acos(_R/(2*_r)) + _phi;
    double theta0 = asin(_R / (2 * _r)) + _phi;
    double theta1 = 2 * _phi - theta0;

    double ex_x0 = _R * cos(theta0), ex_y0 = _R * sin(theta0), ex_x1 = _R * cos(theta1), ex_y1 = _R * sin(theta1);
    double dr0 = sqrt((ex_x0 - _cluster_x) * (ex_x0 - _cluster_x) + (ex_y0 - _cluster_y) * (ex_y0 - _cluster_y));
    double dr1 = sqrt((ex_x1 - _cluster_x) * (ex_x1 - _cluster_x) + (ex_y1 - _cluster_y) * (ex_y1 - _cluster_y));

    ds[0] = (dr0 < dr1) ? dr0 : dr1;
  }
  //z information
  if (_match3D == 1) {
    double      _z0 = _track->getZ0();
    double      _slope = _track->getCotTheta();
    double      _ex_z = _z0 + _slope * _R;
    ds[1] = fabs(_cluster_z - _ex_z);

  }

}
