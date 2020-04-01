#include <cdc/calibration/WireEfficiencyAlgorithm.h>
#include <calibration/CalibrationAlgorithm.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;
WireEfficiencyAlgorithm::WireEfficiencyAlgorithm(): CalibrationAlgorithm("CDCCalibrationCollector")
{

  setDescription(
    " -------------------------- Wire Efficiency Estimation Algorithm -------------------------\n"
  );
}

void WireEfficiencyAlgorithm::buildEfficiencies()
{
  B2INFO("Creating efficiencies for every layer");

  unsigned short wireID;
  unsigned short layerID;
  float z;
  bool isFound;

  auto efftree = getObjectPtr<TTree>("efftree");
  efftree->SetBranchAddress("wireID", &wireID);
  efftree->SetBranchAddress("layerID", &layerID);
  efftree->SetBranchAddress("z", &z);
  efftree->SetBranchAddress("isFound", &isFound);

  // Set ranges and names for the TEfficiency objects
  B2INFO("Building empty efficiency objects");
  static const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

  for (const CDCWireLayer& wireLayer : wireTopology.getWireLayers()) {
    unsigned short layerNo = wireLayer.getICLayer();

    std::string m_nameOfLayer = std::string("effLayer_").append(std::to_string(layerNo));
    std::string m_titleOfLayer = std::string("Efficiency of wires in layer ").append(std::to_string(layerNo));
    unsigned short nzbins = 30 - layerNo / 7;
    unsigned short nwidbins = cdcgeo.nWiresInLayer(layerNo) + 2;
    double widbins[2] = { -0.5, cdcgeo.nWiresInLayer(layerNo) + 0.5};
    double zbins[2] = {wireLayer.getBackwardZ(), wireLayer.getForwardZ()};

    m_efficiencyInLayer[layerNo]->SetName(m_nameOfLayer.c_str());
    m_efficiencyInLayer[layerNo]->SetTitle(m_titleOfLayer.c_str());
    m_efficiencyInLayer[layerNo]->SetBins(nzbins, zbins[0], zbins[1], nwidbins, widbins[0], widbins[1]);
  }

  // loop over entries in the tree and build the TEfficiencies
  B2INFO("Filling the efficiencies");
  const Long64_t nEntries = efftree->GetEntries();
  B2INFO("Number of entries in tree: " << nEntries;);
  for (Long64_t i = 0; i < nEntries; i++) {
    efftree->GetEntry(i);
    m_efficiencyInLayer[layerID]->Fill(isFound, z, wireID);
  }
  B2INFO("TEfficiencies successfully filled.");
}

CalibrationAlgorithm::EResult WireEfficiencyAlgorithm::calibrate()
{
  const auto exprun = getRunList()[0];
  B2INFO("ExpRun used for DB Geometry : " << exprun.first << " " << exprun.second;);
  updateDBObjPtrs(1, exprun.second, exprun.first);

  B2INFO("Creating CDCGeometryPar object");
  CDC::CDCGeometryPar::Instance(&(*m_cdcGeo));

  buildEfficiencies();

  return c_OK;
}