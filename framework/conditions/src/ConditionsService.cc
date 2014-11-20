#include <framework/conditions/ConditionsService.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TFile.h>
#include <TList.h>

#include <iostream>
#include <string>

using namespace Belle2;

ConditionsService* ConditionsService::m_Instance = NULL;

ConditionsService* ConditionsService::GetInstance()
{

  if (!m_Instance) m_Instance = new ConditionsService;

  return m_Instance;
};


ConditionsService::ConditionsService()
{

}

ConditionsService::~ConditionsService()
{

}

Int_t ConditionsService::StartPayload(std::string PayloadTag,
                                      std::string PayloadType,
                                      std::string SubsystemTag,
                                      std::string AlgorithmName,
                                      std::string AlgorithmVersion,
                                      std::string Experiment,
                                      std::string InitialRun,
                                      std::string FinalRun)
{


  m_current_payloads[PayloadTag] = new TList;

  std::string directory = GenerateDirectory(PayloadTag,
                                            SubsystemTag,
                                            AlgorithmName,
                                            AlgorithmVersion,
                                            InitialRun,
                                            FinalRun);


  std::string filename = GenerateFilename(PayloadTag,
                                          SubsystemTag,
                                          AlgorithmName,
                                          AlgorithmVersion,
                                          InitialRun,
                                          FinalRun);

  TFile f(Form("%s%s", directory.c_str(), filename.c_str()),
          "new");
  if (f.IsOpen()) {

    TSQLServer* sql = TSQLServer::Connect("pgsql://localhost/conditions_dev", "app_cond", "belle2");

    if ((sql == NULL) || (sql->GetErrorCode() != 0)) {
      B2ERROR("Error connecting to conditions database.");
    } else {
      std::string initialRun, finalRun;

      sql->Query(Form("select add_payload_detector_tag('%s','%s')",
                      PayloadType.c_str(),
                      PayloadTag.c_str()));


      if (InitialRun == "NULL") {
        initialRun = InitialRun;
      } else initialRun = "'" + InitialRun + "'";
      if (FinalRun == "NULL") {
        finalRun = FinalRun;
      } else finalRun = "'" + FinalRun + "'";

      sql->Query(Form("select add_payload('%s','%s','%s','%s','%s','%s%s','%s',%s,%s)",
                      SubsystemTag.c_str(),
                      PayloadType.c_str(),
                      PayloadTag.c_str(),
                      AlgorithmName.c_str(),
                      AlgorithmVersion.c_str(),
                      directory.c_str(),
                      filename.c_str(),
                      Experiment.c_str(),
                      initialRun.c_str(),
                      finalRun.c_str()));


      B2INFO("started conditions payload: " << directory << filename);
      sql->Close();
    }
    f.Close();
    return 0;
  } else {
    B2ERROR("<ConditionsService::StartPayload> Error Creating Payload: " << directory << filename);
  }
  return -1;
}

Int_t ConditionsService::CommitPayload(std::string PayloadTag,
                                       std::string SubsystemTag,
                                       std::string AlgorithmName,
                                       std::string AlgorithmVersion,
                                       std::string InitialRun,
                                       std::string FinalRun)
{

  // Check that everything is ok, then commit to the database.
  if (m_current_payloads.count(PayloadTag)) {

    //// Next open the file
    std::string directory = GenerateDirectory(PayloadTag,
                                              SubsystemTag,
                                              AlgorithmName,
                                              AlgorithmVersion,
                                              InitialRun,
                                              FinalRun);


    std::string filename = GenerateFilename(PayloadTag,
                                            SubsystemTag,
                                            AlgorithmName,
                                            AlgorithmVersion,
                                            InitialRun,
                                            FinalRun);


    TFile f(Form("%s%s", directory.c_str(), filename.c_str()),
            "update");
    if (f.IsOpen()) { /// DB looks ok, file looks ok, write objects.
      TIter next(m_current_payloads[PayloadTag]);
      TObject* obj;
      while ((obj = next())) {
        obj->Write();
      }
      f.Close();
      m_current_payloads.erase(PayloadTag);
      return 0;
    } else {
      B2ERROR("<ConditionsService::WritePayloadObject> Error Opening Payload: " << directory << filename);
    }

  } else {
    B2ERROR("Error commiting payload; payload tag " << PayloadTag << " not started or already committed.")
  }

  m_buffer = PayloadTag + SubsystemTag + AlgorithmName + AlgorithmVersion + InitialRun + FinalRun;
  return 0;
}

std::string ConditionsService::GenerateFilename(std::string PayloadTag,
                                                std::string SubsystemTag,
                                                std::string AlgorithmName,
                                                std::string AlgorithmVersion,
                                                std::string InitialRun,
                                                std::string FinalRun)
{

  m_buffer = PayloadTag + SubsystemTag + AlgorithmName + AlgorithmVersion + InitialRun + FinalRun;

  std::string filename = SubsystemTag + "_" + AlgorithmName + "_" + AlgorithmVersion + "_" + PayloadTag + ".root";
  return filename;

}

std::string ConditionsService::GenerateDirectory(std::string PayloadTag,
                                                 std::string SubsystemTag,
                                                 std::string AlgorithmName,
                                                 std::string AlgorithmVersion,
                                                 std::string InitialRun,
                                                 std::string FinalRun)
{
  m_buffer = PayloadTag + SubsystemTag + AlgorithmName + AlgorithmVersion + InitialRun + FinalRun;
  std::string directory = "/srv/itop_data/conditions/" + SubsystemTag + "/" + AlgorithmName + "/" + AlgorithmVersion + "/";
  return directory;

}

Int_t ConditionsService::WritePayloadObject(TObject* payload,
                                            std::string PayloadTag,
                                            std::string SubsystemTag,
                                            std::string AlgorithmName,
                                            std::string AlgorithmVersion,
                                            std::string InitialRun,
                                            std::string FinalRun)
{

  //// First check to see if the payload has been defined in the DB.
  if (m_current_payloads.count(PayloadTag)) {
    m_current_payloads[PayloadTag]->Add(payload);
  } else {
    B2ERROR("Error adding object to payload; payload tag " << PayloadTag << " not started or already commited.")
  }
  m_buffer = PayloadTag + SubsystemTag + AlgorithmName + AlgorithmVersion + InitialRun + FinalRun;
  return 0; // Check the errors.

}

TList* ConditionsService::GetPayloadList(std::string GlobalTag,
                                         std::string  RunNumber,
                                         std::string AlgorithmName,
                                         std::string AlgorithmVersion)
{

  m_buffer = GlobalTag + RunNumber + AlgorithmName + AlgorithmVersion;

  B2INFO("Payload retrieval " << m_buffer);

  std::string query(Form("select payload_url from detector_tag, payload, payload_iov, algorithm where payload_iov.payload_id = payload.payload_id and algorithm.name='%s' and initial_run_id<=(select run_id from run where name='%s') and final_run_id>=(select run_id from run where name='%s') and tag_id = detector_tag.detector_tag_id;", (char*)AlgorithmName.c_str(), (char*)RunNumber.c_str(), (char*)RunNumber.c_str()));

  B2INFO("Payload retrieval " << query);

  TSQLServer* sql = TSQLServer::Connect("pgsql://localhost/conditions_dev", "app_cond", "belle2");
  TSQLResult* res = sql->Query(query.c_str());
  TSQLRow* row = res->Next();

  B2INFO("payload retrieval -> " << row->GetField(0));

  TFile* f = new TFile(row->GetField(0), "READ");
  TList* list = NULL;
  if (!f->IsZombie()) {
    list = f->GetListOfKeys();
  } else {
    B2ERROR("Error retrieving conditions payload.");
  }

  sql->Close();

  return list;

}


