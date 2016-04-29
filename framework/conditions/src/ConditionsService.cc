#include <framework/conditions/ConditionsService.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/core/Module.h>
#include <framework/logging/Logger.h>
#include <TFile.h>
#include <TXMLEngine.h>
#include <TMD5.h>
#include <TSystem.h>

#include <curl/curl.h>

#include <boost/filesystem.hpp>

#include <iostream>
#include <memory>
#include <string>

using namespace Belle2;

ConditionsService* ConditionsService::m_instance = NULL;

ConditionsService* ConditionsService::getInstance()
{

  if (!m_instance) m_instance = new ConditionsService;

  return m_instance;
};


ConditionsService::ConditionsService()
{
  m_RESTbase = "http://belle2db.hep.pnnl.gov/b2s/rest/v1/";
  m_FILEbase = "http://belle2db.hep.pnnl.gov/";
}

ConditionsService::~ConditionsService()
{

}


void ConditionsService::getPayloads(std::string GlobalTag, std::string ExperimentName, std::string RunName)
{

  m_payloads.clear(); // not sure we should do this here.

  if (!m_enabled) {
    B2DEBUG(50, "Central database access is disabled: getPayloads does nothing");
    return;
  }

  CURLcode res{CURLE_FAILED_INIT};
  CURL* curl = curl_easy_init();
  char errbuf[CURL_ERROR_SIZE];

  if (curl) {
    curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/xml");
    std::string REST_payloads = m_RESTbase + "iovPayloads/?gtName=" + GlobalTag + "&expName=" + ExperimentName + "&runName=" + RunName;
    B2DEBUG(50, "rest payload call: " << REST_payloads);
    m_buffer.clear();
    curl_easy_setopt(curl, CURLOPT_URL, REST_payloads.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_return);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // For debug.
    res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }

  if (res != CURLE_OK) {
    size_t len = strlen(errbuf);
    if (len) {
      B2WARNING("Could not get list of payloads from database: " << errbuf);
    } else {
      B2WARNING("Could not get list of payloads from database: " << curl_easy_strerror(res));
    }
    B2WARNING("Access to central database is disabled");
    m_enabled = false;
    return;
  }

  if (!m_buffer.empty()) parse_payloads(m_buffer);

  B2INFO("Conditions service retrieved " << m_payloads.size() << " payloads for experiment "
         << ExperimentName << " and run " << RunName << " listed under global tag " << GlobalTag);
}

namespace {
  void displayNodes(TXMLEngine* xml, XMLNodePointer_t node, Int_t level)
  {
    // this function display all accessible information about xml node and its childs

    printf("%*c node: %s\n", level, ' ', xml->GetNodeName(node));

    // display namespace
    XMLNsPointer_t ns = xml->GetNS(node);
    if (ns != 0)
      printf("%*c namespace: %s refer: %s\n", level + 2, ' ', xml->GetNSName(ns), xml->GetNSReference(ns));

    // display attributes
    XMLAttrPointer_t attr = xml->GetFirstAttr(node);
    while (attr != 0) {
      printf("%*c attr: %s value: %s\n", level + 2, ' ', xml->GetAttrName(attr), xml->GetAttrValue(attr));
      attr = xml->GetNextAttr(attr);
    }

    // display content (if exists)
    const char* content = xml->GetNodeContent(node);
    if (content != 0)
      printf("%*c cont: %s\n", level + 2, ' ', content);

    // display all child nodes
    XMLNodePointer_t child = xml->GetChild(node);
    while (child != 0) {
      displayNodes(xml, child, level + 2);
      child = xml->GetNext(child);
    }
  }
}


void ConditionsService::parse_return(std::string temp)
{
  TXMLEngine* xml = new TXMLEngine;

  XMLDocPointer_t xmldoc = xml->ParseString(temp.c_str());
  if (xmldoc == 0) {
    B2WARNING("corrupt return from REST call: " << temp.c_str());
    B2WARNING("Access to central database is disabled");
    m_enabled = false;
    return;
  }


  // take access to main node
  XMLNodePointer_t mainnode = xml->DocGetRootElement(xmldoc);

  // display recursively all nodes and subnodes
  displayNodes(xml, mainnode, 1);

  // Release memory before exit
  xml->FreeDoc(xmldoc);
  delete xml;

}

size_t ConditionsService::capture_return(void* buffer, size_t size, size_t nmemb, void*)
{
  std::string temp(static_cast<const char*>(buffer), size * nmemb);
  ConditionsService::getInstance()->addReturn(temp);
  return size * nmemb;
}


void ConditionsService::parse_payloads(std::string temp)
{
  std::unique_ptr<TXMLEngine> xml(new TXMLEngine);

  XMLDocPointer_t xmldoc = xml->ParseString(temp.c_str());
  if (xmldoc == 0) {
    B2WARNING("corrupt return from REST call: " << temp.c_str());
    B2WARNING("Access to central database is disabled");
    m_enabled = false;
    return;
  }

  // take access to main node
  XMLNodePointer_t mainnode = xml->DocGetRootElement(xmldoc);

  // display recursively all nodes and subnodes
  //  displayNodes(xml, mainnode, 1);

  // parse the payloads
  XMLNodePointer_t iov_node = xml->GetChild(mainnode);
  XMLNodePointer_t payload_node = xml->GetChild(iov_node);

  XMLNodePointer_t child_node;

  std::string PackageName, ModuleName, PayloadURL, Checksum;

  XMLAttrPointer_t attr;
  std::string nodeName;

  while (payload_node) {
    conditionsPayload payload;
    nodeName = xml->GetNodeName(payload_node);
    attr = xml->GetFirstAttr(payload_node);
    B2DEBUG(100, "Parsing payload... " << nodeName << " with id " << xml->GetAttrValue(attr));
    if (nodeName == "payload") { /// Found a payload, now parse the needed information.

      child_node = xml->GetChild(payload_node);
      while (child_node) { // Search for the module
        nodeName = xml->GetNodeName(child_node);

        if (nodeName == "payloadUrl") {
          payload.logicalFileName = xml->GetNodeContent(child_node);
        }
        if (nodeName == "checksum") {
          payload.md5Checksum = xml->GetNodeContent(child_node);
        }
        if (nodeName == "basf2Module") { // module found
          XMLNodePointer_t module_node = child_node;
          child_node = xml->GetChild(child_node);
          while (child_node) { // Search for the package
            nodeName = xml->GetNodeName(child_node);
            if (nodeName == "name") {
              // Get the module name.
              payload.module = xml->GetNodeContent(child_node);
            } else if (nodeName == "basf2Package") {
              XMLNodePointer_t package_node = child_node;
              child_node = xml->GetChild(child_node);
              while (child_node) { // Search for the package name
                nodeName = xml->GetNodeName(child_node);
                if (nodeName == "name") {
                  // Get the module name.
                  payload.package = xml->GetNodeContent(child_node);
                }
                child_node = xml->GetNext(child_node);
              }
              child_node = package_node;
            }
            child_node = xml->GetNext(child_node);
          }
          child_node = module_node;
        }
        child_node = xml->GetNext(child_node);
      }
    }

    payload_node = xml->GetNext(payload_node);
    nodeName = xml->GetNodeName(payload_node);
    if (nodeName == "payloadIov") { /// Found a payload, now parse the needed information.
      child_node = xml->GetChild(payload_node);
      while (child_node) { // Search for the runs
        std::string iovNodeName = xml->GetNodeName(child_node);
        if ((iovNodeName == "initialRunId") || (iovNodeName == "finalRunId")) {
          XMLNodePointer_t run_node = child_node;
          std::string runName, expName;
          child_node = xml->GetChild(child_node);
          while (child_node) { // Search for the run name and experiment
            nodeName = xml->GetNodeName(child_node);
            if (nodeName == "name") {
              runName = xml->GetNodeContent(child_node);
            }
            if (nodeName == "experiment") {
              XMLNodePointer_t exp_node = child_node;
              child_node = xml->GetChild(child_node);
              while (child_node) { // Search for the experiment name
                nodeName = xml->GetNodeName(child_node);
                if (nodeName == "name") {
                  expName = xml->GetNodeContent(child_node);
                }
                child_node = xml->GetNext(child_node);
              }
              child_node = exp_node;
            }

            child_node = xml->GetNext(child_node);
          }
          child_node = run_node;
          if (iovNodeName == "initialRunId") {
            payload.runInitial = runName;
            payload.expInitial = expName;
          }
          if (iovNodeName == "finalRunId") {
            payload.runFinal = runName;
            payload.expFinal = expName;
          }
        }
        child_node = xml->GetNext(child_node);
      }
    }

    if (payload.package.size() == 0 || payload.module.size() == 0 || payload.logicalFileName.size() == 0) {
      B2WARNING("ConditionsService::parse_payload Payload not parsed correctly: empty package, module or filename");
    } else {
      std::string payloadKey = payload.package + payload.module;
      if (ConditionsService::getInstance()->payloadExists(payloadKey)) {
        B2WARNING("Found duplicate payload key " << payloadKey <<
                  " while parsing conditions payloads. Using refusing to add payload with identical key.");
      } else {
        B2DEBUG(100, "Found payload for module " << payload.module << " in package " << payload.package
                << " at URL " << payload.logicalFileName << ".  Storing with key: "
                << payloadKey << " and checksum: " << payload.md5Checksum << ".  IOV (exp/run) from "
                << payload.expInitial << "/" << payload.runInitial << " to " << payload.expFinal << "/" << payload.runFinal);
        ConditionsService::getInstance()->addPayloadInfo(payloadKey, payload);
      }
    }

    iov_node = xml->GetNext(iov_node);
    payload_node = xml->GetChild(iov_node);
  }

  // Release memory before exit
  xml->FreeDoc(xmldoc);
}

conditionsPayload ConditionsService::getPayloadInfo(std::string PackageModuleName)
{
  conditionsPayload payload;
  if (payloadExists(PackageModuleName)) {
    payload = m_payloads[PackageModuleName];
  } else {
    B2WARNING("Payload information requested for " << PackageModuleName << ", but not found.");
  }
  return payload;
}
void ConditionsService::writePayloadFile(std::string payloadFileName, const Module* module)
{
  writePayloadFile(payloadFileName, module->getPackage(), module->getName());
}

void ConditionsService::writePayloadFile(std::string payloadFileName,
                                         std::string packageName,
                                         std::string moduleName)
{

  if (!m_enabled) {
    B2ERROR("Central database access is disabled: writePayloadFile does nothing");
    return;
  }

  std::unique_ptr<TMD5> checksum(TMD5::FileChecksum(payloadFileName.c_str()));

  if (!checksum) {
    B2ERROR("Error calculating checksum for file " << payloadFileName << ".  Are you sure it exists?");
    return;
  }
  if (checksum->AsString() == std::string("d41d8cd98f00b204e9800998ecf8427e")) {
    B2ERROR("Found empty file md5sum for " << payloadFileName << ", cowardly refusing to add file to conditions DB.");
    return;
  }
  std::string username = "BASF2 Conditions User";

  std::string description = "Automated add by BASF2, no user description provided.";

  TFile* f = TFile::Open(payloadFileName.c_str());
  if (!f->IsZombie()) {
  } else { // File is not a valid ROOT file.  Consider throwing an error.
    B2WARNING("The conditions payload " << payloadFileName.c_str() << " could not be loaded by ROOT properly.");
  }

  std::string json_header = "{ \"checksum\": \"";
  json_header = json_header + checksum->AsString() + "\" , \"isDefault\": false , \"modifiedBy\": \"" + username +
                "\" , \"description\": \"" + description + "\" }";
  B2INFO("json header: " << json_header);

  CURL* curl;
  CURLcode res;

  // build post
  std::string REST_payloads = m_RESTbase + "package/" + packageName + "/module/" + moduleName + "/payload";
  B2INFO("rest payload post: " << REST_payloads);


  struct curl_httppost* post = NULL;
  struct curl_httppost* last = NULL;
  struct curl_slist* headerlist = NULL;
  //  static const char buf[] = "Expect:";

  curl_global_init(CURL_GLOBAL_ALL);

  curl_formadd(&post, &last,
               CURLFORM_COPYNAME, "metadata",
               CURLFORM_COPYCONTENTS, json_header.c_str(),
               CURLFORM_CONTENTTYPE, "application/json", CURLFORM_END);
  curl_formadd(&post, &last,
               CURLFORM_COPYNAME, "contents",
               CURLFORM_FILECONTENT, payloadFileName.c_str(),
               CURLFORM_CONTENTTYPE, "application/x-root", CURLFORM_END);

  headerlist = curl_slist_append(headerlist, "Content-Type: multipart/mixed");
  headerlist = curl_slist_append(headerlist, "Accept: application/xml");

  curl = curl_easy_init();
  if (curl) {

    m_buffer.clear();
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_URL, REST_payloads.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_return);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      B2ERROR("Sending payload to database failed: " << curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    parse_return(m_buffer);
  }

  curl_slist_free_all(headerlist);
}

size_t ConditionsService:: write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
}

std::string ConditionsService::getPayloadFileURL(const Module* module)
{
  return getPayloadFileURL(module->getPackage(), module->getName());
}

std::string ConditionsService::getPayloadFileURL(std::string packageName, std::string moduleName)
{
  if (!m_enabled) {
    B2DEBUG(50, "Central database access is disabled: getPayloadFileURL does nothing");
    return "";
  }

  if (!payloadExists(packageName + moduleName)) {
    B2WARNING("Unable to find conditions payload for requested package " << packageName << "\t and module " << moduleName);
    return "";
  }
  std::string remote_file = m_payloads[packageName + moduleName].logicalFileName;
  std::string local_file = m_FILEbase + remote_file; // This will work for local files and CVMFS.

  if (m_FILEbase.substr(0, 7) == "http://") { // May need to transfer files locally.
    local_file = m_FILElocal + boost::filesystem::path(remote_file).filename().string();
    std::unique_ptr<TMD5> checksum(TMD5::FileChecksum(local_file.c_str())); // check if the file exists

    remote_file = m_FILEbase + remote_file;
    if (!checksum) { // file isn't there.  Start downloading.
      B2DEBUG(100, "Did not find file " << local_file << " starting download from " << remote_file);

      CURL* curl;
      FILE* fp;
      CURLcode res;
      char errbuf[CURL_ERROR_SIZE];

      curl = curl_easy_init();
      if (curl) {
        fp = fopen(local_file.c_str(), "wb");
        curl_easy_setopt(curl, CURLOPT_URL, remote_file.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, true);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
        //  curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);

        if (res != CURLE_OK) {
          size_t len = strlen(errbuf);
          if (len) {
            B2WARNING("Could not download payload for " << packageName << "/" << moduleName << " from database: " << errbuf);
          } else {
            B2WARNING("Could not download payload for " << packageName << "/" << moduleName << " from database: " << curl_easy_strerror(res));
          }
          //FIXME: delete file
          return "";
        }
      }
      checksum.reset(TMD5::FileChecksum(local_file.c_str())); // check checksum
    } else if (checksum->AsString() == m_payloads[packageName + moduleName].md5Checksum) {
      B2INFO("Found file: " << local_file << " with correct MD5 checksum: " << checksum->AsString());
      return local_file;
    }


    while (checksum->AsString() != m_payloads[packageName + moduleName].md5Checksum) { // then there was a checksum mis-match
      gSystem->Sleep(1000);
      std::unique_ptr<TMD5> checksum_new(TMD5::FileChecksum(local_file.c_str())); // check checksum again
      if (std::string(checksum->AsString()) != std::string(checksum_new->AsString())) {   // Then we are downloading the file already.
        B2INFO("File with incorrect checksum found, download appears to be occuring... waiting for file to complete ");
        B2INFO("checksum: " << checksum->AsString() << "\tchecksum_new: " << checksum_new->AsString());
        checksum = std::move(checksum_new);
      } else { // File isn't downloading, but checksums don't match.  Throw an error.
        B2FATAL("Error with file " << local_file.c_str() << " checksum expected: " << m_payloads[packageName + moduleName].md5Checksum <<
                " found: " << checksum->AsString());
        return NULL;
      }

    }
  }

  std::unique_ptr<TMD5> checksum(TMD5::FileChecksum(local_file.c_str())); // check if the file exists
  if (!checksum) { // file isn't there.  Toss an error.
    B2WARNING("Did not find file " << local_file << " check inputs.");
    return "";
  } else if (std::string(checksum->AsString()) != m_payloads[packageName +
                                                             moduleName].md5Checksum) { // MD5 checksum doesn't match the database entry.
    B2FATAL("Conditions file " << local_file << " error! Expected MD5 checksum " << m_payloads[packageName + moduleName].md5Checksum <<
            " and calculated checksum " << checksum->AsString());
    return "";
  }

  return local_file;
}

/*
int ConditionsService::progress_func(void* ptr, double TotalToDownload, double NowDownloaded,
                                     double TotalToUpload, double NowUploaded)
{
  // how wide you want the progress meter to be
  int totaldotz = 40;

  double fractiondownloaded = NowDownloaded / TotalToDownload;
  double fractionuploaded   = NowUploaded / TotalToUpload;
  // part of the progressmeter that's already "full"
  int dotz = round(fractiondownloaded * totaldotz);
  printf("%3.0f%% [", fractiondownloaded * 100);
  if (TotalToUpload > 0) {
    dotz = round(fractionuploaded * totaldotz);
    printf("%3.0f%% [", fractionuploaded * 100);
  }

  // create the "meter"
  int ii = 0;

  // part  that's full already
  for (; ii < dotz; ii++) {
    printf("=");
  }
  // remaining part (spaces)
  for (; ii < totaldotz; ii++) {
    printf(" ");
  }
  // and back to line begin - do not forget the fflush to avoid output buffering problems!
  printf("]\r");
  fflush(stdout);
  // if you don't return 0, the transfer will be aborted - see the documentation
  return 0;
}
*/
