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
#include <analysis/NtupleTools/NtupleTrackTool.h>
#include <analysis/NtupleTools/NtupleTrackHitsTool.h>
#include <analysis/NtupleTools/NtupleEventBasedTrackingTool.h>
#include <analysis/NtupleTools/NtupleClusterTool.h>
#include <analysis/NtupleTools/NtupleRecoStatsTool.h>
#include <analysis/NtupleTools/NtupleMCReconstructibleTool.h>
#include <analysis/NtupleTools/NtupleVertexTool.h>
#include <analysis/NtupleTools/NtupleMCVertexTool.h>
#include <analysis/NtupleTools/NtupleCustomFloatsTool.h>
#include <analysis/NtupleTools/NtupleROEMultiplicitiesTool.h>
#include <analysis/NtupleTools/NtupleROEGammasTool.h>
#include <analysis/NtupleTools/NtupleRecoilKinematicsTool.h>
#include <analysis/NtupleTools/NtupleExtraEnergyTool.h>
#include <analysis/NtupleTools/NtupleTagVertexTool.h>
#include <analysis/NtupleTools/NtupleMCTagVertexTool.h>
#include <analysis/NtupleTools/NtupleDeltaTTool.h>
#include <analysis/NtupleTools/NtupleMCDeltaTTool.h>
#include <analysis/NtupleTools/NtupleContinuumSuppressionTool.h>
#include <analysis/NtupleTools/NtupleFlavorTaggingTool.h>
#include <analysis/NtupleTools/NtupleFlavorTagInfoTool.h>
#include <analysis/NtupleTools/NtupleFlightInfoTool.h>
#include <analysis/NtupleTools/NtupleMCFlightInfoTool.h>
#include <analysis/NtupleTools/NtupleMomentumVectorDeviationTool.h>
#include <analysis/NtupleTools/NtupleParentRestKinematicsTool.h>
#include <analysis/NtupleTools/NtupleDalitzTool.h>
#include <analysis/NtupleTools/NtupleVVAnglesTool.h>
#include <analysis/NtupleTools/NtupleHelicityTool.h>
#include <analysis/NtupleTools/NtupleSLTool.h>
#include <analysis/NtupleTools/NtupleErrMatrixTool.h>
#include <analysis/NtupleTools/NtupleEventWeightsTool.h>
#include <analysis/NtupleTools/NtupleLECDCTool.h>
#include <analysis/NtupleTools/NtupleLEECLTool.h>
#include <analysis/NtupleTools/NtupleLEKLMTool.h>
#include <analysis/NtupleTools/NtupleHLTTagTool.h>
#include <analysis/NtupleTools/NtupleMCGenKinematicsTool.h>
#include <analysis/NtupleTools/NtupleMCGenCMSKinematicsTool.h>
#include <analysis/NtupleTools/NtupleKlongIDTool.h>
#include <analysis/NtupleTools/NtupleMCDecayStringTool.h>
#include <analysis/NtupleTools/NtupleEventShapeTool.h>

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
  else if (strToolName == "ParentRestKinematics") return new NtupleParentRestKinematicsTool(tree, d);
  else if (strToolName == "MomentumUncertainty") return new NtupleMomentumUncertaintyTool(tree, d);
  else if (strToolName == "InvMass") return new NtupleInvMassTool(tree, d, strOption);
  else if (strToolName == "MassBeforeFit") return new NtupleMassBeforeFitTool(tree, d);
  else if (strToolName == "EventMetaData") return new NtupleEventMetaDataTool(tree, d);
  else if (strToolName == "DeltaEMbc") return new NtupleDeltaEMbcTool(tree, d);
  else if (strToolName == "Charge") return new NtupleChargeTool(tree, d);
  else if (strToolName == "PDGCode") return new NtuplePDGCodeTool(tree, d);
  else if (strToolName == "MCTruth") return new NtupleMCTruthTool(tree, d);
  else if (strToolName == "MCHierarchy") return new NtupleMCHierarchyTool(tree, d, strOption);

  else if (strToolName == "MCKinematics") return new NtupleMCKinematicsTool(tree, d);
  else if (strToolName == "PID") return new NtuplePIDTool(tree, d);
  else if (strToolName == "RecoStats") return new NtupleRecoStatsTool(tree, d);
  else if (strToolName == "MCReconstructible") return new NtupleMCReconstructibleTool(tree, d);
  else if (strToolName == "Track") return new NtupleTrackTool(tree, d);
  else if (strToolName == "TrackHits") return new NtupleTrackHitsTool(tree, d);
  else if (strToolName == "EventBasedTracking") return new NtupleEventBasedTrackingTool(tree, d);
  else if (strToolName == "Cluster") return new NtupleClusterTool(tree, d);
  else if (strToolName == "Vertex") return new NtupleVertexTool(tree, d);
  else if (strToolName == "MCVertex") return new NtupleMCVertexTool(tree, d);
  else if (strToolName == "CustomFloats") return new NtupleCustomFloatsTool(tree, d, strOption);
  else if (strToolName == "ROEMultiplicities") return new NtupleROEMultiplicitiesTool(tree, d);
  else if (strToolName == "ROEGammas") return new NtupleROEGammasTool(tree, d);
  else if (strToolName == "RecoilKinematics") return new NtupleRecoilKinematicsTool(tree, d);
  else if (strToolName == "ExtraEnergy") return new NtupleExtraEnergyTool(tree, d);
  else if (strToolName == "TagVertex") return new NtupleTagVertexTool(tree, d);
  else if (strToolName == "MCTagVertex") return new NtupleMCTagVertexTool(tree, d);
  else if (strToolName == "DeltaT") return new NtupleDeltaTTool(tree, d);
  else if (strToolName == "MCDeltaT") return new NtupleMCDeltaTTool(tree, d);
  else if (strToolName == "ContinuumSuppression") return new NtupleContinuumSuppressionTool(tree, d, strOption);
  else if (strToolName == "FlavorTagging") return new NtupleFlavorTaggingTool(tree, d, strOption);
  else if (strToolName == "FlavorTagInfo") return new NtupleFlavorTagInfoTool(tree, d);
  else if (strToolName == "FlightInfo") return new NtupleFlightInfoTool(tree, d);
  else if (strToolName == "MCFlightInfo") return new NtupleMCFlightInfoTool(tree, d);
  else if (strToolName == "MomentumVectorDeviation") return new NtupleMomentumVectorDeviationTool(tree, d);
  else if (strToolName == "Dalitz") return new NtupleDalitzTool(tree, d);
  else if (strToolName == "VVAngles") return new NtupleVVAnglesTool(tree, d);
  else if (strToolName == "Helicity") return new NtupleHelicityTool(tree, d);
  else if (strToolName == "SL") return new NtupleSLTool(tree, d);
  else if (strToolName == "ErrMatrix") return new NtupleErrMatrixTool(tree, d);
  else if (strToolName == "EventWeights") return new NtupleEventWeightsTool(tree, d);
  else if (strToolName == "LECDC")  return new NtupleLECDCTool(tree, d);
  else if (strToolName == "LEECL")  return new NtupleLEECLTool(tree, d);
  else if (strToolName == "LEKLM")  return new NtupleLEKLMTool(tree, d);
  else if (strToolName == "HLTTag")  return new NtupleHLTTagTool(tree, d);
  else if (strToolName == "MCGenKinematics") return new NtupleMCGenKinematicsTool(tree, d, strOption);
  else if (strToolName == "MCGenCMSKinematics") return new NtupleMCGenCMSKinematicsTool(tree, d, strOption);
  else if (strToolName == "KlongID") return new NtupleKlongIDTool(tree, d);
  else if (strToolName == "MCDecayString") return new NtupleMCDecayStringTool(tree, d);
  else if (strToolName == "EventShape") return new NtupleEventShapeTool(tree, d);
  B2ERROR("NtupleTool \"" << strToolName << "\" is not available: no tool with that name could be found!");
  return NULL;
}
