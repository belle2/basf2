#ifndef HITHIT_H
#define HITHIT_H


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
#include <tracking/dataobjects/hitXP.h>
#include <TFile.h>



namespace Belle2 {

  //  This module from a data root file builds a tree of hitXP
  //  (see the class to know all the informations contained). The tree
  //  filled with tracks (so each "event" is a track), as vector of
  //  hitXP.
  //
  // The module create 4 tree:
  //  - one complete (TTree_hitXP.root),
  //  - one with only tracks with at least one hit per SVD layer (TTree_hitXPSel.root)
  //  - one with only tracks with exaclty one hit per SVD layer, so no overlap
  //    are allowed (TTree_hitXPTiSel.root)
  //  - one that does not use hitXP class but uses standard classes, it is
  //    lighter and cointains less informations but is usable externally from
  //    basf2 (TTree_hitXP_ext.root)

  class hitXPModule : public Module {
  public:

    hitXPModule();

    virtual ~hitXPModule();

    virtual void initialize() override;

    virtual void beginRun() override;

    virtual void event() override;

    virtual void endRun() override;

    virtual void terminate() override;

  private:
    TTree* m_tree;
    TFile* m_outputFile;

    std::vector<hitXP> m_hitXP; //= NULL;
    std::set<hitXP, hitXP::timeCompare> m_hitXPSet;

    int m_trackNumber;
    int m_eventNumber;
    int m_numberHitPerTrack;

    int track_iterator;
    int event_iterator;



    TTree* m_treeSel;
    TFile* m_outputFileSel;
    std::vector<hitXP> m_hitXPSel;
    int m_trackNumberSel;
    int m_eventNumberSel;
    int m_numberHitPerTrackSel;


    TTree* m_treeTiSel;
    TFile* m_outputFileTiSel;
    std::vector<hitXP> m_hitXPTiSel;
    int m_trackNumberTiSel;
    int m_eventNumberTiSel;
    int m_numberHitPerTrackTiSel;


    TFile* m_outputFileExt;
    TTree* m_treeExt;
    std::vector<double> m_EpositionEntryX;
    std::vector<double> m_EpositionEntryY;
    std::vector<double> m_EpositionEntryZ;
    std::vector<double> m_EmomentumEntryX;
    std::vector<double> m_EmomentumEntryY;
    std::vector<double> m_EmomentumEntryZ;
    std::vector<double> m_EpositionLocalEntryX;
    std::vector<double> m_EpositionLocalEntryY;
    std::vector<double> m_EpositionLocalEntryZ;
    std::vector<int> m_EPDGID;
    std::vector<double> m_Eposition0X;
    std::vector<double> m_Eposition0Y;
    std::vector<double> m_Eposition0Z;
    std::vector<double> m_Emomentum0X;
    std::vector<double> m_Emomentum0Y;
    std::vector<double> m_Emomentum0Z;
    std::vector<double> m_Etime;
    std::vector<int> m_EsensorSensor;
    std::vector<int> m_EsensorLayer;
    std::vector<int> m_EsensorLadder;
    std::vector<int> m_Ereconstructed;
    std::vector<int> m_EclusterU;
    std::vector<int> m_EclusterV;
    std::vector<double> m_Echarge;
    int m_EtrackNumber;
    int m_EeventNumber;
    int m_EnumberHitPerTrack;
    std::vector<double> m_EomegaEntry;
    std::vector<double> m_Eomega0;
    std::vector<double> m_Ed0Entry;
    std::vector<double> m_Ed00;
    std::vector<double> m_Ephi0Entry;
    std::vector<double> m_Ephi00;
    std::vector<double> m_Ez0Entry;
    std::vector<double> m_Ez00;
    std::vector<double> m_EtanlambdaEntry;
    std::vector<double> m_Etanlambda0;
    int m_Eprimary;

  protected:

  };
}
#endif
