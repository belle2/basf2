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
  if (strOption != "") {
    B2INFO("   -> using these options: " << strOption);
  }
  */

  if (strToolName == "Kinematics") return new NtupleKinematicsTool(tree, d);
  else if (strToolName == "CMSKinematics") return new NtupleCMSKinematicsTool(tree, d);
  else if (strToolName == "MomentumUncertainty") return new NtupleMomentumUncertaintyTool(tree, d);
  else if (strToolName == "InvMass") return new NtupleInvMassTool(tree, d, strOption);
  else if (strToolName == "MassBeforeFit") return new NtupleMassBeforeFitTool(tree, d);
  else if (strToolName == "EventMetaData") return new NtupleEventMetaDataTool(tree, d);
  else if (strToolName == "DeltaEMbc") return new NtupleDeltaEMbcTool(tree, d);
  else if (strToolName == "Charge") return new NtupleChargeTool(tree, d);
  else if (strToolName == "PDGCode") return new NtuplePDGCodeTool(tree, d);
  else if (strToolName == "MCTruth") return new NtupleMCTruthTool(tree, d);
  else if (strToolName == "MCHierarchy") return new NtupleMCHierarchyTool(tree, d);
  else if (strToolName == "MCKinematics") return new NtupleMCKinematicsTool(tree, d);
  else if (strToolName == "PID") return new NtuplePIDTool(tree, d);
  else if (strToolName == "DeltaLogL") return new NtupleDeltaLogLTool(tree, d);
  else if (strToolName == "RecoStats") return new NtupleRecoStatsTool(tree, d);
  else if (strToolName == "DetectorStatsRec") return new NtupleDetectorStatsRecTool(tree, d);
  else if (strToolName == "DetectorStatsSim") return new NtupleDetectorStatsSimTool(tree, d);
  else if (strToolName == "MCReconstructible") return new NtupleMCReconstructibleTool(tree, d);
  else if (strToolName == "Track") return new NtupleTrackTool(tree, d);
  else if (strToolName == "TrackHits") return new NtupleTrackHitsTool(tree, d);
  else if (strToolName == "Cluster") return new NtupleClusterTool(tree, d);
  else if (strToolName == "Vertex") return new NtupleVertexTool(tree, d);
  else if (strToolName == "MCVertex") return new NtupleMCVertexTool(tree, d);
  else if (strToolName == "CustomFloats") return new NtupleCustomFloatsTool(tree, d, strOption);
  else if (strToolName == "ROEMultiplicities") return new NtupleROEMultiplicitiesTool(tree, d);
  else if (strToolName == "RecoilKinematics") return new NtupleRecoilKinematicsTool(tree, d);
  else if (strToolName == "ExtraEnergy") return new NtupleExtraEnergyTool(tree, d);
  else if (strToolName == "TagVertex") return new NtupleTagVertexTool(tree, d);
  else if (strToolName == "MCTagVertex") return new NtupleMCTagVertexTool(tree, d);
  else if (strToolName == "DeltaT") return new NtupleDeltaTTool(tree, d);
  else if (strToolName == "MCDeltaT") return new NtupleMCDeltaTTool(tree, d);
  else if (strToolName == "ContinuumSuppression") return new NtupleContinuumSuppressionTool(tree, d);
  else if (strToolName == "MCDecayString") return new NtupleMCDecayStringTool(tree, d);
  else if (strToolName == "FlightInfo") return new NtupleFlightInfoTool(tree, d);
  else if (strToolName == "MCFlightInfo") return new NtupleMCFlightInfoTool(tree, d);
  else if (strToolName == "MomentumVectorDeviation") return new NtupleMomentumVectorDeviationTool(tree, d);

  B2WARNING("NtupleTool " << strToolName << " is not available!");
  return NULL;
}
