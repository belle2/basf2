#include <alignment/calibration/MillepedeAlgorithm.h>

#include <alignment/PedeApplication.h>
#include <alignment/PedeResult.h>
#include <alignment/dataobjects/PedeSteering.h>
#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/dbobjects/CDCCalibration.h>
#include <alignment/dbobjects/BKLMAlignment.h>
#include <alignment/GlobalLabel.h>

#include <eklm/alignment/AlignmentTools.h>
#include <eklm/dbobjects/EKLMAlignment.h>

#include <framework/dbobjects/BeamParameters.h>

using namespace std;
using namespace Belle2;
using namespace alignment;

MillepedeAlgorithm::MillepedeAlgorithm() : CalibrationAlgorithm("MillepedeCollector")
{
  setDescription("Millepede calibration & alignment algorithm");
}

CalibrationAlgorithm::EResult MillepedeAlgorithm::calibrate()
{
  B2INFO(" Mean of Chi2 / NDF of tracks before calibration: " << getObject<TH1F>("chi2/ndf").GetMean(););

  // Write out binary files from tree and add to steering
  prepareMilleBinary();
  for (auto file : getObject<MilleData>("mille").getFiles())
    m_steering.addFile(file);

  // Run calibration on steering
  m_result = m_pede.calibrate(m_steering);

  if (!m_pede.success() || !m_result.isValid()) {
    B2ERROR(m_pede.getExitMessage());
    return c_Failure;
  }

  if (m_result.getNoParameters() == 0) {
    B2INFO("No parameters to store. Failure.");
    return c_Failure;
  }

  int nBeamParams = 0;
  int nVXDparams = 0;
  int nCDCparams = 0;
  int nBKLMparams = 0;
  int nEKLMparams = 0;
  int undeterminedParams = 0;
  for (int ipar = 0; ipar < m_result.getNoParameters(); ipar++) {
    if (!m_result.isParameterDetermined(ipar)) {
      if (!m_result.isParameterFixed(ipar)) {
        ++undeterminedParams;
      }
      continue;
    }

    GlobalLabel param(m_result.getParameterLabel(ipar));
    if (param.isBeam()) ++nBeamParams;
    if (param.isVXD()) ++nVXDparams;
    if (param.isCDC()) ++nCDCparams;
    if (param.isBKLM()) ++nBKLMparams;
    if (param.isEKLM()) ++nEKLMparams;
  }
  if (!nBeamParams)
    B2INFO("No Beam parameters determined");
  if (!nVXDparams)
    B2INFO("No VXD parameters determined");
  if (!nCDCparams)
    B2INFO("No CDC parameters determined");
  if (!nBKLMparams)
    B2INFO("No BKLM parameters determined");
  if (!nEKLMparams)
    B2INFO("No EKLM parameters determined");

  if (undeterminedParams) {
    B2WARNING("There are " << undeterminedParams << " undetermined parameters. Not enough data for calibration.");
    return c_NotEnoughData;
  }

  // Now here comes some experimental part...
  // The main reason to do this, we are only computing
  // CORRECTIONS to previous alignment, not absolute values.
  // Therefore we must add our result to all previous constants...

  // Set of of all (exp, run) occured in data collection
  auto& runSet = getObject<RunRange>(CalibrationAlgorithm::RUN_RANGE_OBJ_NAME);
  // Objects in DB we are interested in
  std::list<Database::DBQuery> belle2Constants;
  if (nBeamParams)
    belle2Constants.push_back(Database::DBQuery("BeamParameters"));
  if (nVXDparams)
    belle2Constants.push_back(Database::DBQuery("VXDAlignment"));
  if (nCDCparams)
    belle2Constants.push_back(Database::DBQuery("CDCCalibration"));
  if (nBKLMparams)
    belle2Constants.push_back(Database::DBQuery("BKLMAlignment"));
  if (nEKLMparams)
    belle2Constants.push_back(Database::DBQuery("EKLMAlignment"));
  // Maps (key is IOV of object in DB)
  std::map<string, BeamParameters*> previousBeam;
  std::map<string, VXDAlignment*> previousVXD;
  std::map<string, CDCCalibration*> previousCDC;
  std::map<string, BKLMAlignment*> previousBKLM;
  std::map<string, EKLMAlignment*> previousEKLM;
  // Collect all distinct existing objects in DB:
  for (auto& exprun : runSet.getExpRunSet()) {
    // Ask DB for data at Event 1 in each run
    auto event1 = EventMetaData(1, exprun.second, exprun.first);
    Database::Instance().getData(event1, belle2Constants);
    for (auto& payload : belle2Constants) {
      if (auto beam = dynamic_cast<BeamParameters*>(payload.object)) {
        previousBeam[to_string(payload.iov)] = beam;
      }
      if (auto vxd = dynamic_cast<VXDAlignment*>(payload.object)) {
        previousVXD[to_string(payload.iov)] = vxd;
      }
      if (auto cdc = dynamic_cast<CDCCalibration*>(payload.object)) {
        previousCDC[to_string(payload.iov)] = cdc;
      }
      if (auto bklm = dynamic_cast<BKLMAlignment*>(payload.object)) {
        previousBKLM[to_string(payload.iov)] = bklm;
      }
      if (auto eklm = dynamic_cast<EKLMAlignment*>(payload.object)) {
        previousEKLM[to_string(payload.iov)] = eklm;
      }
    }
  }
  // All objects have to be re-created, with new constant values...
  std::map<string, BeamParameters*> newBeam;
  std::map<string, VXDAlignment*> newVXD;
  std::map<string, CDCCalibration*> newCDC;
  std::map<string, BKLMAlignment*> newBKLM;
  std::map<string, EKLMAlignment*> newEKLM;

  if (nBeamParams)
    for (auto& beam : previousBeam)
      newBeam[beam.first] = new BeamParameters(*beam.second);

  if (nVXDparams)
    for (auto& vxd : previousVXD)
      newVXD[vxd.first] = new VXDAlignment(*vxd.second);

  if (nCDCparams)
    for (auto& cdc : previousCDC)
      newCDC[cdc.first] = new CDCCalibration(*cdc.second);

  if (nBKLMparams)
    for (auto& bklm : previousBKLM)
      newBKLM[bklm.first] = new BKLMAlignment(*bklm.second);

  if (nEKLMparams)
    for (auto& eklm : previousEKLM)
      newEKLM[eklm.first] = new EKLMAlignment(*eklm.second);

  if (newBeam.empty() && nBeamParams) {
    B2ERROR("No previous BeamParameters found. First update from nominal. Only vertex position is filled!");
    newBeam.insert({to_string(getIovFromData()), new BeamParameters()});
  }

  if (newVXD.empty() && nVXDparams) {
    B2INFO("No previous VXDAlignment found. First update from nominal.");
    newVXD.insert({to_string(getIovFromData()), new VXDAlignment()});
  }

  if (newCDC.empty() && nCDCparams) {
    B2INFO("No previous CDCCalibration found. First update from nominal.");
    newCDC.insert({to_string(getIovFromData()), new CDCCalibration()});
  }

  if (newBKLM.empty() && nBKLMparams) {
    B2INFO("No previous BKLMAlignment found. First update from nominal.");
    newBKLM.insert({to_string(getIovFromData()), new BKLMAlignment()});
  }

  if (newEKLM.empty() && nEKLMparams) {
    B2INFO("No previous EKLMAlignment found. First update from nominal.");
    EKLMAlignment* alignment = new EKLMAlignment();
    EKLM::fillZeroDisplacements(alignment);
    newEKLM.insert({to_string(getIovFromData()), alignment});
  }

  double maxCorrectionPull = 0.;
  int maxCorrectionPullLabel = 0;
  int nParams = 0;
  double paramChi2 = 0.;

  // Loop over all determined parameters:
  for (int ipar = 0; ipar < m_result.getNoParameters(); ipar++) {
    if (!m_result.isParameterDetermined(ipar)) continue;

    GlobalLabel param(m_result.getParameterLabel(ipar));
    double correction = m_result.getParameterCorrection(ipar);
    double error = m_result.getParameterError(ipar);
    double pull = correction / error;

    ++nParams;
    paramChi2 += pull * pull;

    if (fabs(pull) > fabs(maxCorrectionPull)) {
      maxCorrectionPull = pull;
      maxCorrectionPullLabel = param.label();
    }

    if (param.isBeam()) {
      // Add correction to all objects
      for (auto& beam : newBeam) {
        auto vertex = beam.second->getVertex();

        if (param.getParameterId() == BeamID::vertexX)(m_invertSign) ? vertex[0] -= correction : vertex[0] += correction;
        if (param.getParameterId() == BeamID::vertexY)(m_invertSign) ? vertex[1] -= correction : vertex[1] += correction;
        if (param.getParameterId() == BeamID::vertexZ)(m_invertSign) ? vertex[2] -= correction : vertex[2] += correction;

        beam.second->setVertex(vertex);
      }
    }

    if (param.isVXD()) {
      // Add correction to all objects
      for (auto& vxd : newVXD) {
        vxd.second->add(param.getVxdID(), param.getParameterId(), correction, m_invertSign);
      }
    }

    if (param.isCDC()) {
      // Add correction to all objects
      for (auto& cdc : newCDC) {
        cdc.second->add(param.getWireID(), param.getParameterId(), correction, m_invertSign);
      }
    }

    if (param.isBKLM()) {
      // Add correction to all objects
      for (auto& bklm : newBKLM) {
        bklm.second->add(param.getBklmID(), param.getParameterId(), correction, m_invertSign);
      }
    }

    if (param.isEKLM()) {
      // Add correction to all objects
      for (auto& eklm : newEKLM) {
        int segment = param.getEklmID().getSegmentGlobalNumber();
        EKLMAlignmentData* alignmentData =
          eklm.second->getAlignmentData(segment);
        if (alignmentData == NULL)
          B2FATAL("EKLM alignment data not found, probable error in segment "
                  "number.");
        switch (param.getParameterId()) {
          case 1:
            alignmentData->setDy(correction);
            break;
          case 2:
            alignmentData->setDalpha(correction);
            break;
          default:
            B2FATAL("Incorrect EKLM alignment parameter " <<
                    param.getParameterId());
            break;
        }
      }
    }

  }

  // Save (possibly updated) objects
  for (auto& beam : newBeam)
    saveCalibration(beam.second, "BeamParameters", to_IOV(beam.first).overlap(getIovFromData()));
  for (auto& vxd : newVXD)
    saveCalibration(vxd.second, "VXDAlignment", to_IOV(vxd.first).overlap(getIovFromData()));
  for (auto& cdc : newCDC)
    saveCalibration(cdc.second, "CDCCalibration", to_IOV(cdc.first).overlap(getIovFromData()));
  for (auto& bklm : newBKLM)
    saveCalibration(bklm.second, "BKLMAlignment", to_IOV(bklm.first).overlap(getIovFromData()));
  for (auto& eklm : newEKLM)
    saveCalibration(eklm.second, "EKLMAlignment", to_IOV(eklm.first).overlap(getIovFromData()));

  //commit();

  if (paramChi2 / nParams > 1. || fabs(maxCorrectionPull) > 50.) {
    B2INFO("Largest correction/error is " << maxCorrectionPull << " for parameter with label " << maxCorrectionPullLabel);
    B2INFO("Parameter corrections Chi2/NDF, e.g. sum[(correction/error)^2]/#params = " << paramChi2 / nParams
           << " = " << paramChi2 << " / " << nParams << " > 1.");

    B2INFO("Requesting iteration.");
    return c_Iterate;
  }

  return c_OK;
}

void MillepedeAlgorithm::prepareMilleBinary()
{
  // Clear list of binaries in steering
  m_steering.clearFiles();

  // Temp file name
  const std::string milleFileName("gbl-data.mille");

  // For no entries, no binary file is created
  auto& gblDataTree = getObject<TTree>("GblDataTree");
  if (!gblDataTree.GetEntries()) {
    B2WARNING("No trajectories in GBL data tree.");
    return;
  }

  // Create new mille binary
  auto milleBinary = new gbl::MilleBinary(milleFileName);

  // Containers for GBL data
  double aValue(0.);
  double aErr(0.);
  std::vector<unsigned int>* indLocal;
  std::vector<double>* derLocal;
  std::vector<int>* labGlobal;
  std::vector<double>* derGlobal;

  // Read vectors of GblData from tree branch
  std::vector<gbl::GblData>* currentGblData = new std::vector<gbl::GblData>();
  gblDataTree.SetBranchAddress("GblData", &currentGblData);

  B2INFO("Writing Millepede binary files...");
  for (unsigned int iRecord = 0; iRecord < gblDataTree.GetEntries(); ++iRecord) {
    gblDataTree.GetEntry(iRecord);

    if (!currentGblData)
      continue;

    for (gbl::GblData& theData : *currentGblData) {
      theData.getAllData(aValue, aErr, indLocal, derLocal, labGlobal,
                         derGlobal);
      milleBinary->addData(aValue, aErr, *indLocal, *derLocal, *labGlobal,
                           *derGlobal);
    }
    milleBinary->writeRecord();
  }
  // Closes the file
  delete milleBinary;
  B2INFO("Millepede binary files written.");

  // Add binary to steering
  m_steering.addFile(milleFileName);
}




















