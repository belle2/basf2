#include <analysis/NtupleTools/NtupleToolList.h>
#include <analysis/NtupleTools/NtupleKinematicsTool.h>
#include <analysis/NtupleTools/NtupleEventMetaDataTool.h>
#include <analysis/NtupleTools/NtupleDeltaEMbcTool.h>
#include <analysis/NtupleTools/NtupleMCTruthTool.h>
#include <analysis/NtupleTools/NtupleMCHierarchyTool.h>
#include <analysis/NtupleTools/NtupleMCKinematicsTool.h>
#include <analysis/NtupleTools/NtuplePIDTool.h>
#include <analysis/NtupleTools/NtupleTrackTool.h>
#include <analysis/NtupleTools/NtupleRecoStatsTool.h>
#include <analysis/NtupleTools/NtupleDetectorStatsRecTool.h>
#include <analysis/NtupleTools/NtupleDetectorStatsSimTool.h>
#include <analysis/NtupleTools/NtupleMCReconstructibleTool.h>
#include <analysis/NtupleTools/NtupleVertexTool.h>
#include <analysis/NtupleTools/NtupleMCVertexTool.h>

using namespace Belle2;
using namespace std;

NtupleFlatTool* NtupleToolList::create(string strName, TTree* tree, DecayDescriptor& d)
{
  if (strName.compare("Kinematics") == 0) return new NtupleKinematicsTool(tree, d);
  else if (strName.compare("EventMetaData") == 0) return new NtupleEventMetaDataTool(tree, d);
  else if (strName.compare("DeltaEMbc") == 0) return new NtupleDeltaEMbcTool(tree, d);
  else if (strName.compare("MCTruth") == 0) return new NtupleMCTruthTool(tree, d);
  else if (strName.compare("MCHierarchy") == 0) return new NtupleMCHierarchyTool(tree, d);
  else if (strName.compare("MCKinematics") == 0) return new NtupleMCKinematicsTool(tree, d);
  else if (strName.compare("PID") == 0) return new NtuplePIDTool(tree, d);
  else if (strName.compare("RecoStats") == 0) return new NtupleRecoStatsTool(tree, d);
  else if (strName.compare("DetectorStatsRec") == 0) return new NtupleDetectorStatsRecTool(tree, d);
  else if (strName.compare("DetectorStatsSim") == 0) return new NtupleDetectorStatsSimTool(tree, d);
  else if (strName.compare("MCReconstructible") == 0) return new NtupleMCReconstructibleTool(tree, d);
  else if (strName.compare("Track") == 0) return new NtupleTrackTool(tree, d);
  else if (strName.compare("Vertex") == 0) return new NtupleVertexTool(tree, d);
  else if (strName.compare("MCVertex") == 0) return new NtupleMCVertexTool(tree, d);

  B2WARNING("The specified NtupleTool is not available!");
  return NULL;
}
