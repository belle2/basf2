#pragma once

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Weightfile.h>
#include <mva/interface/Expert.h>

using namespace Belle2;

using TrackFindingCDC::CDCWireHit;
using TrackFindingCDC::StoreWrappedObjPtr;

class CATFinderModule : public Module {

public:
  CATFinderModule();
  virtual ~CATFinderModule() {}

  void initialize() override;
  void beginRun() override;
  void event() override;
  void terminate() override;

private:

  StoreArray<CDCHit> m_CDCHits;
  StoreArray<RecoTrack> m_CDCRecoTracks{"CDCRecoTracks"};
  StoreArray<RecoHitInformation> m_recoHitInformations{"RecoHitInformations"};
  StoreWrappedObjPtr<std::vector<CDCWireHit>> m_wireHitVector{"CDCWireHitVector"};

  CDC::CDCGeometryPar* CDCGeometryPar;

  static constexpr double HIT_DISTANCE = 0.3;
  static constexpr int CDC_HIT_CUT = 10;
  static constexpr long unsigned int CDC_HIT_INDICES_CUT = 7;
  static constexpr int TARGET_DISTANCE = 16;
  static constexpr unsigned int N_INPUT_FEATURES = 7;
  static constexpr unsigned int N_OUTPUT_FEATURES = 11;
  static constexpr float T_BETA = 0.7;
  static constexpr double T_DISTANCE = 0.7;
  static constexpr double TDC_OFFSET = 4100;
  static constexpr double TDC_SCALE = 1100;
  static constexpr double ADC_CLIP = 600;
  static constexpr double SLAYER_SCALE = 10;
  static constexpr double CLAYER_SCALE = 56;
  static constexpr double LAYER_SCALE = 10;
  static constexpr int LATENT_SPACE_N_DIM = 3;

  int evtNo;

  // GNN inputs
  std::vector<double> CDCHitMiddleX, CDCHitMiddleY, CDCHitX, CDCHitY, CDCHitDriftTime, CDCHitDriftLength, CDCHitChargeDeposit;
  std::vector<unsigned short> CDCHitSuperlayer, CDCHitLayer, CDCHitCLayer, CDCHitTOT, CDCHitADC;
  std::vector<short> CDCHitTDC;

  std::unique_ptr<MVA::SingleDataset> m_dataset;
  std::vector<double> m_outputs;

  // GNN output
  std::vector<double> predBetas, predQs, conPointQs;
  std::vector<std::vector<double>> predPs, predVs, coords, conPoints, conPointPs, conPointVs;

  std::vector<int> betaIndices, selectedBetas;

  // MVA
  std::unique_ptr<MVA::Expert> m_expert;
  const std::string m_identifier = "CATFinderWeightfile";
  DBObjPtr<DatabaseRepresentationOfWeightfile> m_weightfile_representation{m_identifier};

  // Methods
  void preprocess();
  void runGNN();
  void postprocess();

  void prepareVectors();
  void collectOverThreshold(std::vector<int> betaIndices, std::vector<std::vector<double>> coords,
                            std::vector<int>& preselectedBetas);
  void initializeMVA(MVA::Weightfile& weightfile);

  bool isConPointOutOfRadius(const std::vector<double>& pointCandidate, const std::vector<std::vector<double>>& selectedBetas);
};
