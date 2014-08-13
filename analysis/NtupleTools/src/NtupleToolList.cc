#include <framework/logging/Logger.h>
#include <analysis/NtupleTools/NtupleToolList.h>
#include <analysis/NtupleTools/NtupleKinematicsTool.h>
#include <analysis/NtupleTools/NtupleCMSKinematicsTool.h>
#include <analysis/NtupleTools/NtupleMomentumUncertaintyTool.h>
#include <analysis/NtupleTools/NtupleInvMassTool.h>
#include <analysis/NtupleTools/NtupleMassBeforeFitTool.h>
#include <analysis/NtupleTools/NtupleEventMetaDataTool.h>
#include <analysis/NtupleTools/NtupleDeltaEMbcTool.h>
#include <analysis/NtupleTools/NtupleChargeTool.h>
#include <analysis/NtupleTools/NtuplePDGCodeTool.h>
#include <analysis/NtupleTools/NtupleMCTruthTool.h>
#include <analysis/NtupleTools/NtupleMCHierarchyTool.h>
#include <analysis/NtupleTools/NtupleMCKinematicsTool.h>
#include <analysis/NtupleTools/NtuplePIDTool.h>
#include <analysis/NtupleTools/NtupleDeltaLogLTool.h>
#include <analysis/NtupleTools/NtupleTrackTool.h>
#include <analysis/NtupleTools/NtupleTrackHitsTool.h>
#include <analysis/NtupleTools/NtupleClusterTool.h>
#include <analysis/NtupleTools/NtupleRecoStatsTool.h>
#include <analysis/NtupleTools/NtupleDetectorStatsRecTool.h>
#include <analysis/NtupleTools/NtupleDetectorStatsSimTool.h>
#include <analysis/NtupleTools/NtupleMCReconstructibleTool.h>
#include <analysis/NtupleTools/NtupleVertexTool.h>
#include <analysis/NtupleTools/NtupleMCVertexTool.h>
#include <analysis/NtupleTools/NtupleCustomFloatsTool.h>
#include <analysis/NtupleTools/NtupleROEMultiplicitiesTool.h>
#include <analysis/NtupleTools/NtupleRecoilKinematicsTool.h>
#include <analysis/NtupleTools/NtupleExtraEnergyTool.h>
#include <analysis/NtupleTools/NtupleTagVertexTool.h>
#include <analysis/NtupleTools/NtupleMCTagVertexTool.h>
#include <analysis/NtupleTools/NtupleDeltaTTool.h>
#include <analysis/NtupleTools/NtupleMCDeltaTTool.h>
#include <analysis/NtupleTools/NtupleContinuumSuppressionTool.h>
#include <analysis/NtupleTools/NtupleMCDecayStringTool.h>
#include <analysis/NtupleTools/NtupleFlightInfoTool.h>
#include <analysis/NtupleTools/NtupleMCFlightInfoTool.h>
#include <analysis/NtupleTools/NtupleMomentumVectorDeviationTool.h>

using namespace Belle2;
using namespace std;

NtupleFlatTool* NtupleToolList::create(string strName, TTree* tree, DecayDescriptor& d)
{

  size_t iPosBracket1 = strName.find("[");
  string strOption("");
  string strToolName("");

  //B2INFO("Length : " << strName.size());
  //B2INFO("Position of ( : " << iPosBracket1);
  //B2INFO("Position of ) : " << strName.find(")"));

  if (iPosBracket1 != string::npos) {
    if (strName.find("]") == strName.size() - 1) {
      strToolName = strName.substr(0, iPosBracket1);
      strOption = strName.substr(iPosBracket1 + 1, strName.size() - iPosBracket1 - 2);
    } else {
      B2ERROR("Incorrect NtupleTool name: " << strName);
      return NULL;
    }
  } else {
    strToolName = strName;
  }

  B2INFO(" -> creating Ntuple tool: " << strToolName);

  // print out options in the individual tools if neccessary
  /*
  if (strOption.compare(std::string("")) != 0) {
    B2INFO("   -> using these options: " << strOption);
  }
  */

  if (strToolName.compare("Kinematics") == 0) return new NtupleKinematicsTool(tree, d);
  else if (strToolName.compare("CMSKinematics") == 0) return new NtupleCMSKinematicsTool(tree, d);
  else if (strToolName.compare("MomentumUncertainty") == 0) return new NtupleMomentumUncertaintyTool(tree, d);
  else if (strToolName.compare("InvMass") == 0) return new NtupleInvMassTool(tree, d, strOption);
  else if (strToolName.compare("MassBeforeFit") == 0) return new NtupleMassBeforeFitTool(tree, d);
  else if (strToolName.compare("EventMetaData") == 0) return new NtupleEventMetaDataTool(tree, d);
  else if (strToolName.compare("DeltaEMbc") == 0) return new NtupleDeltaEMbcTool(tree, d);
  else if (strToolName.compare("Charge") == 0) return new NtupleChargeTool(tree, d);
  else if (strToolName.compare("PDGCode") == 0) return new NtuplePDGCodeTool(tree, d);
  else if (strToolName.compare("MCTruth") == 0) return new NtupleMCTruthTool(tree, d);
  else if (strToolName.compare("MCHierarchy") == 0) return new NtupleMCHierarchyTool(tree, d);
  else if (strToolName.compare("MCKinematics") == 0) return new NtupleMCKinematicsTool(tree, d);
  else if (strToolName.compare("PID") == 0) return new NtuplePIDTool(tree, d);
  else if (strToolName.compare("DeltaLogL") == 0) return new NtupleDeltaLogLTool(tree, d);
  else if (strToolName.compare("RecoStats") == 0) return new NtupleRecoStatsTool(tree, d);
  else if (strToolName.compare("DetectorStatsRec") == 0) return new NtupleDetectorStatsRecTool(tree, d);
  else if (strToolName.compare("DetectorStatsSim") == 0) return new NtupleDetectorStatsSimTool(tree, d);
  else if (strToolName.compare("MCReconstructible") == 0) return new NtupleMCReconstructibleTool(tree, d);
  else if (strToolName.compare("Track") == 0) return new NtupleTrackTool(tree, d);
  else if (strToolName.compare("TrackHits") == 0) return new NtupleTrackHitsTool(tree, d);
  else if (strToolName.compare("Cluster") == 0) return new NtupleClusterTool(tree, d);
  else if (strToolName.compare("Vertex") == 0) return new NtupleVertexTool(tree, d);
  else if (strToolName.compare("MCVertex") == 0) return new NtupleMCVertexTool(tree, d);
  else if (strToolName.compare("CustomFloats") == 0) return new NtupleCustomFloatsTool(tree, d, strOption);
  else if (strToolName.compare("ROEMultiplicities") == 0) return new NtupleROEMultiplicitiesTool(tree, d);
  else if (strToolName.compare("RecoilKinematics") == 0) return new NtupleRecoilKinematicsTool(tree, d);
  else if (strToolName.compare("ExtraEnergy") == 0) return new NtupleExtraEnergyTool(tree, d);
  else if (strToolName.compare("TagVertex") == 0) return new NtupleTagVertexTool(tree, d);
  else if (strToolName.compare("MCTagVertex") == 0) return new NtupleMCTagVertexTool(tree, d);
  else if (strToolName.compare("DeltaT") == 0) return new NtupleDeltaTTool(tree, d);
  else if (strToolName.compare("MCDeltaT") == 0) return new NtupleMCDeltaTTool(tree, d);
  else if (strToolName.compare("ContinuumSuppression") == 0) return new NtupleContinuumSuppressionTool(tree, d);
  else if (strToolName.compare("MCDecayString") == 0) return new NtupleMCDecayStringTool(tree, d);
  else if (strToolName.compare("FlightInfo") == 0) return new NtupleFlightInfoTool(tree, d);
  else if (strToolName.compare("MCFlightInfo") == 0) return new NtupleMCFlightInfoTool(tree, d);
  else if (strToolName.compare("MomentumVectorDeviation") == 0) return new NtupleMomentumVectorDeviationTool(tree, d);

  B2WARNING("NtupleTool " << strToolName << " is not available!");
  return NULL;
}
