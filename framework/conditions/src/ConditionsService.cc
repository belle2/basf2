#include <framework/conditions/ConditionsService.h>
#include <framework/core/Module.h>
#include <framework/logging/Logger.h>
#include <TFile.h>
#include <TXMLEngine.h>
#include <TMD5.h>
#include <TSystem.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <curl/curl.h>

#include <boost/filesystem.hpp>

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <set>

using namespace Belle2;

ConditionsService* ConditionsService::getInstance()
{

  static ConditionsService instance;

  return &instance;
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
  std::unique_ptr<TXMLEngine> xml(new TXMLEngine);

  XMLDocPointer_t xmldoc = xml->ParseString(temp.c_str());
  if (xmldoc == 0) {
    B2WARNING("corrupt return from REST call: " << temp);
    B2WARNING("Access to central database is disabled");
    m_enabled = false;
    return;
  }


  // take access to main node
  XMLNodePointer_t mainnode = xml->DocGetRootElement(xmldoc);

  // display recursively all nodes and subnodes
  displayNodes(xml.get(), mainnode, 1);

  // Release memory before exit
  xml->FreeDoc(xmldoc);
}

size_t ConditionsService::capture_return(void* buffer, size_t size, size_t nmemb, void*)
{
  std::string temp(static_cast<const char*>(buffer), size * nmemb);
  ConditionsService::getInstance()->addReturn(temp);
  return size * nmemb;
}


void ConditionsService::parse_payloads(std::string temp)
{
  std::stringstream input(temp);
  boost::property_tree::ptree pt;
  std::set<std::string> duplicates;
  try {
    boost::property_tree::read_xml(input, pt);

    for (auto& iov : pt.get_child("currentPayloadIovs")) {
      if (iov.first != "currentPayloadIov") {
        throw boost::property_tree::ptree_error("child node should be 'currentPayloadIov', not '" + iov.first + "'");
      }
      conditionsPayload payloadInfo;
      auto payload = iov.second.get_child("payload");
      auto payloadIov = iov.second.get_child("payloadIov");
      B2DEBUG(100, "Parsing payload with id " << payload.get("<xmlattr>.payloadId", ""));
      payloadInfo.package = payload.get<std::string>("basf2Module.basf2Package.name");
      payloadInfo.module = payload.get<std::string>("basf2Module.name");
      payloadInfo.md5Checksum = payload.get<std::string>("checksum");
      payloadInfo.logicalFileName = payload.get<std::string>("payloadUrl");
      payloadInfo.expInitial = payloadIov.get<std::string>("initialRunId.experiment.name");
      payloadInfo.runInitial = payloadIov.get<std::string>("initialRunId.name");
      payloadInfo.expFinal = payloadIov.get<std::string>("finalRunId.experiment.name");
      payloadInfo.runFinal = payloadIov.get<std::string>("finalRunId.name");
      payloadInfo.revision = payload.get<int>("revision", 0);

      if (payloadInfo.package.size() == 0 || payloadInfo.module.size() == 0 || payloadInfo.logicalFileName.size() == 0) {
        B2WARNING("ConditionsService::parse_payload Payload not parsed correctly: empty package, module or filename");
      } else {
        std::string payloadKey = payloadInfo.package + payloadInfo.module;
        auto payloadIter = m_payloads.find(payloadKey);
        if (payloadIter != m_payloads.end()) {
          int keep = std::max(payloadIter->second.revision, payloadInfo.revision);
          int drop = std::min(payloadIter->second.revision, payloadInfo.revision);
          if (payloadIter->second.revision < payloadInfo.revision) {
            payloadIter->second = payloadInfo;
          }
          B2DEBUG(10, "Found duplicate payload key " << payloadKey << " while parsing conditions payloads. "
                  "Discarding revision " << drop << " and using revision " << keep);
          duplicates.insert(payloadInfo.module);
        } else {
          B2DEBUG(100, "Found payload for module " << payloadInfo.module << " in package " << payloadInfo.package
                  << " at URL " << payloadInfo.logicalFileName << ".  Storing with key: "
                  << payloadKey << " and checksum: " << payloadInfo.md5Checksum << ".  IOV (exp/run) from "
                  << payloadInfo.expInitial << "/" << payloadInfo.runInitial << " to " << payloadInfo.expFinal << "/" << payloadInfo.runFinal);
          addPayloadInfo(payloadKey, payloadInfo);
        }
      }
    }
  } catch (boost::property_tree::ptree_error& e) {
    B2WARNING("Cannot parse database payload information: " << e.what());
    B2WARNING("Access to central database is disabled");
    m_enabled = false;
    return;
  }
  if (!duplicates.empty()) {
    B2INFO("Found more then one payload for the following keys: " << boost::algorithm::join(duplicates, ", "));
  }
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

size_t ConditionsService::write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
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
