#ifndef HITTOTRUEHIT_H
#define HITTOTRUEHIT_H


#include <framework/core/Module.h>
#include <TTree.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/dataobjects/hitToTrueXP.h>
#include <TFile.h>



namespace Belle2 {

  //  This module from a data root file builds a tree of hitToTrueXP
  //  (see the class to know all the informations contained). The tree
  //  filled with tracks (so each "event" is a track), as vector of
  //  hitToTrueXP.
  //
  // The module create 4 tree:
  //  - one complete (TTree_hitToTrueXP.root),
  //  - one with only tracks with at least one hit per SVD layer (TTree_hitToTrueXPSel.root)
  //  - one with only tracks with exaclty one hit per SVD layer, so no overlap
  //    are allowed (TTree_hitToTrueXPTiSel.root)
  //  - one that does not use hitToTrueXP class but uses standard classes, it is
  //    lighter and cointains less informations but is usable externally from
  //    basf2 (TTree_hitToTrueXP_ext.root)

  class hitToTrueXPModule : public Module {
  public:

    hitToTrueXPModule();

    virtual ~hitToTrueXPModule();

    virtual void initialize() override;

    virtual void beginRun() override;

    virtual void event() override;

    virtual void endRun() override;

    virtual void terminate() override;

  private:
    TTree* m_tree;
    TFile* m_outputFile;

    std::vector<hitToTrueXP> m_hitToTrueXP; //= NULL;
    std::set<hitToTrueXP, hitToTrueXP::timeCompare> set_hitToTrueXP;

    int m_trackNumber;
    int m_eventNumber;
    int m_numberHitPerTrack;

    int track_iterator;
    int event_iterator;



    TTree* m_treeSel;
    TFile* m_outputFileSel;
    std::vector<hitToTrueXP> m_hitToTrueXPSel;
    int m_trackNumberSel;
    int m_eventNumberSel;
    int m_numberHitPerTrackSel;


    TTree* m_treeTiSel;
    TFile* m_outputFileTiSel;
    std::vector<hitToTrueXP> m_hitToTrueXPTiSel;
    int m_trackNumberTiSel;
    int m_eventNumberTiSel;
    int m_numberHitPerTrackTiSel;


    TFile* m_outputFileExt;
    TTree* m_treeExt;
    std::vector<double> e_positionEntryX;
    std::vector<double> e_positionEntryY;
    std::vector<double> e_positionEntryZ;
    std::vector<double> e_momentumEntryX;
    std::vector<double> e_momentumEntryY;
    std::vector<double> e_momentumEntryZ;
    std::vector<double> e_positionLocalEntryX;
    std::vector<double> e_positionLocalEntryY;
    std::vector<double> e_positionLocalEntryZ;
    std::vector<int> e_PDGID;
    std::vector<double> e_position0X;
    std::vector<double> e_position0Y;
    std::vector<double> e_position0Z;
    std::vector<double> e_momentum0X;
    std::vector<double> e_momentum0Y;
    std::vector<double> e_momentum0Z;
    std::vector<double> e_time;
    std::vector<int> e_sensorSensor;
    std::vector<int> e_sensorLayer;
    std::vector<int> e_sensorLadder;
    std::vector<int> e_reconstructed;
    std::vector<int> e_clusterU;
    std::vector<int> e_clusterV;
    std::vector<double> e_charge;
    int e_trackNumber;
    int e_eventNumber;
    int e_numberHitPerTrack;
    std::vector<double> e_omegaEntry;
    std::vector<double> e_omega0;
    std::vector<double> e_d0Entry;
    std::vector<double> e_d00;
    std::vector<double> e_phi0Entry;
    std::vector<double> e_phi00;
    std::vector<double> e_z0Entry;
    std::vector<double> e_z00;
    std::vector<double> e_tanlambdaEntry;
    std::vector<double> e_tanlambda0;
    int e_primary;

  protected:

  };
}
#endif
