/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka (tadeas.bilka@gmail.com)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/MillepedeCalibration/MillepedeCalibrationModule.h>

#include <calibration/CalibrationManager.h>
#include <calibration/CalibrationModule.h>

#include <framework/datastore/StoreArray.h>
#include <genfit/Track.h>
#include <genfit/GblFitter.h>
#include <genfit/MilleBinary.h>
#include <genfit/GblPoint.h>
#include <genfit/GblTrajectory.h>
#include <genfit/GblFitStatus.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>
#include <TH1D.h>
#include <TFile.h>
#include <TSystem.h>

class TH1D;

using namespace std;
using namespace Belle2;
using namespace calibration;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MillepedeCalibration)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MillepedeCalibrationModule::MillepedeCalibrationModule() : CalibrationModule(),
  m_milleFile(0),
  m_histoProcID("", DataStore::c_Persistent),
  m_histoPval("", DataStore::c_Persistent),
  m_histoNormChi2("", DataStore::c_Persistent),
  m_histoNdf("", DataStore::c_Persistent)
{
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);
  // Set module properties
  setDescription("Global Belle2 calibration&alignment using GeneralBrokenLines refit and Millepede II");

  // Parameter definitions
  addParam("tracks", m_tracks, "Name of collection of genfit::Tracks for calibration", std::string(""));
  addParam("binary", m_binary, "Name of Mille binary file with calibration data", std::string("belle.mille"));
  addParam("steering", m_steering, "Name of text steering file for Pede to run calibration", std::string("steer.txt"));
  addParam("minPvalue", m_minPvalue, "Minimum p-value to write trajectory to Mille binary", double(0.));

  // Dependecies  (empty here)
  addDefaultDependencyList();

}

void MillepedeCalibrationModule::Prepare()
{
  StoreArray<genfit::Track> tracks(m_tracks);
  tracks.isRequired();

  m_histoProcID.registerInDataStore(getName() + "_" + "processID", DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  m_histoProcID.construct("processID", "MillepedeCalibration: ProcessID in which track was written to Mille file", 65, -1., 64.);

  m_histoPval.registerInDataStore(getName() + "_" + "Pval", DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  m_histoPval.construct("Pval", "MillepedeCalibration: P-value of tracks in Mille file", 100, 0., 1.);

  m_histoNormChi2.registerInDataStore(getName() + "_" + "normChi2",
                                      DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  m_histoNormChi2.construct("normChi2", "MillepedeCalibration: Chi2/Ndf of tracks in Mille file", 100, 0., 10.);

  m_histoNdf.registerInDataStore(getName() + "_" + "ndf", DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  m_histoNdf.construct("ndf", "MillepedeCalibration: NDF of tracks in Mille file", 201, -1., 200.);
}


void MillepedeCalibrationModule::CollectData()
{
  // Input tracks (have to be fitted by GBL)
  StoreArray<genfit::Track> tracks(m_tracks);
  std::shared_ptr<genfit::GblFitter> gbl(new genfit::GblFitter());
  // If m_milleFile is null, this is first call to CollectData.
  // We need to create the file for this process here, because
  // Prepare() is called before processes are forkewithoutCutNamed.
  if (!m_milleFile) {
    int processID = ProcHandler::EvtProcID();


    if (ProcHandler::parallelProcessingUsed()) {
      if (ProcHandler::isEventProcess()) {
        //TODO: do not include processID to file extension, better
        // place it at the end of filename (like 'file1.mille')
        m_binary = m_binary + std::to_string(processID);

        B2INFO("Creating Mille file for process " << processID << " with name '" << m_binary << "'");
        m_milleFile = new gbl::MilleBinary(m_binary);
      } else if (ProcHandler::isOutputProcess()) {
        B2INFO("Creating Mille file with name '" << m_binary << "'");
        m_milleFile = new gbl::MilleBinary(m_binary);
      }

    } else {
      B2INFO("Creating Mille file with name '" << m_binary << "'");
      m_milleFile = new gbl::MilleBinary(m_binary);
    }

  }

  for (auto track : tracks) {
    if (!track.hasFitStatus())
      continue;
    genfit::GblFitStatus* fs = dynamic_cast<genfit::GblFitStatus*>(track.getFitStatus());
    if (!fs)
      continue;

    if (!fs->isFitConvergedFully())
      continue;

    if (fs->getPVal() >= m_minPvalue) {
      using namespace gbl;
      GblTrajectory trajectory(gbl->collectGblPoints(&track, track.getCardinalRep()), fs->hasCurvature());
      trajectory.milleOut(*m_milleFile);

      if (m_histoProcID.isValid())
        m_histoProcID->get().Fill(double(ProcHandler::EvtProcID()));
      if (m_histoPval.isValid())
        m_histoPval->get().Fill(double(fs->getPVal()));
      if (m_histoNormChi2.isValid())
        m_histoNormChi2->get().Fill(double(fs->getChi2() / fs->getNdf()));
      if (m_histoNdf.isValid())
        m_histoNdf->get().Fill(double(fs->getNdf()));
    }

  }
}

bool MillepedeCalibrationModule::StoreInDataBase()
{
  B2INFO("Starting StoreInDataBase for calibration module " << getName());
  return readResultWriteXml("millepede.xml");
}

CalibrationModule::ECalibrationModuleMonitoringResult MillepedeCalibrationModule::Monitor()
{
  B2INFO("Starting Monitoring for calibration module " << getName());
  // Here we are actually in single processing mode or in output path
  B2INFO("Accumulated mean of Chi2/Ndf histogram: " << m_histoNormChi2->get().GetMean());

  if (getCalibrationFile()) {
    m_histoProcID->get().SetDirectory(getCalibrationFile());
    m_histoProcID->write(getCalibrationFile());
    m_histoPval->get().SetDirectory(getCalibrationFile());
    m_histoPval->write(getCalibrationFile());
    m_histoNormChi2->get().SetDirectory(getCalibrationFile());
    m_histoNormChi2->write(getCalibrationFile());
    m_histoNdf->get().SetDirectory(getCalibrationFile());
    m_histoNdf->write(getCalibrationFile());
  }

  return CalibrationModule::c_MonitoringSuccess;
}

void MillepedeCalibrationModule::closeParallelFiles()
{
  // We have to properly close the file here
  if (m_milleFile) {
    B2INFO("Closing Mille file with name '" << m_binary << "'");

    delete m_milleFile;
    m_milleFile = nullptr;
  }
}

CalibrationModule::ECalibrationModuleResult MillepedeCalibrationModule::Calibrate()
{
  B2INFO("Starting Calibration for calibration module " << getName());
  // We have to properly close the file here if not yet closed (deleted)
  // That actually means we are in single processing mode or in output path
  if (m_milleFile) {
    B2INFO("Closing Mille file with name '" << m_binary << "'");

    delete m_milleFile;
    m_milleFile = nullptr;
  }


  std::cout << "Starting Millepede II Alignment/Calibration..." << std::endl;
  std::string cmd("pede");
  cmd = cmd + " " + m_steering;
  std::system(cmd.c_str());
  ifstream result("millepede.end");
  int resultID = -1;
  result >> resultID;

  std::stringstream buffer;
  buffer << result.rdbuf();

  std::string message(buffer.str());

  B2INFO("Millepede finished with code " << resultID << " and message:\n" << message);
  if (resultID >= 0)
    return CalibrationModule::c_Success;
  else
    return CalibrationModule::c_Failure;
}

bool MillepedeCalibrationModule::readResultWriteXml(const string& xml_filename)
{
  ifstream result("millepede.end");
  int resultID = -1;
  result >> resultID;

  std::stringstream buffer;
  buffer << result.rdbuf();

  std::string message(buffer.str());

  ofstream xml(xml_filename);
  string line;
  ifstream res("millepede.res");

  std::map<int, std::string>paramNames;
  paramNames.insert(std::pair<int, std::string>(1, "du"));
  paramNames.insert(std::pair<int, std::string>(2, "dv"));
  paramNames.insert(std::pair<int, std::string>(3, "dw"));
  paramNames.insert(std::pair<int, std::string>(4, "alpha"));
  paramNames.insert(std::pair<int, std::string>(5, "beta"));
  paramNames.insert(std::pair<int, std::string>(6, "gamma"));
  // skip fortran line
  getline(res, line);

  xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  xml << "<Alignment xmlns:xi=\"http://www.w3.org/2001/XInclude\">" << endl;
  xml << "  <Millepede>" << endl;
  xml << "    <ExitCode>" << resultID << "</ExitCode>" << endl;
  xml << "    <ExitMessage>" << message << "</ExitMessage>" << endl;
  xml << "  </Millepede>" << endl;

  unsigned int lastSensor = 0;
  bool startComp = false;
  while (getline(res, line)) {
    int label = -1;
    double param = 0.;
    double presigma = 0.;
    double differ = 0.;
    double error = 0.;
    stringstream lineSTream;
    lineSTream << line;
    lineSTream >> label >> param >> presigma >> differ >> error;

    // Now decode vxd id
    if (label < 10) continue;

    unsigned int id(floor(label / 10));
    unsigned int vxdId = id;
    unsigned int paramId = label - 10 * id;
    // skip segment (5 bits)
    id = id >> 5;
    unsigned int sensor = id & 7;
    id = id >> 3;
    unsigned int ladder = id & 31;
    id = id >> 5;
    unsigned int layer = id & 7;

    if (lastSensor != vxdId) {
      if (startComp) {
        xml << "  </Align>" << endl;
        startComp = false;
      }
      xml << "  <Align component=\"" << layer << "." << ladder << "." << sensor << "\">" << endl;
      lastSensor = vxdId;
      startComp = true;
    }
    xml << "    <" << paramNames.find(paramId)->second;
    if (presigma < 0.) xml << " fixed=\"true\"";



    if (paramId >= 1 && paramId <= 3)
      xml << " unit=\"cm\"";
    if (paramId >= 4 && paramId <= 6)
      xml << " unit=\"rad\"";

    if (error) xml << " error=\"" << error << "\"";


    xml << ">" << param << "</" << paramNames.find(paramId)->second << ">" << endl;
  }
  if (startComp) {
    xml << "  </Align>" << endl;
  }
  xml << "</Alignment>" << endl;

  return true;
}





