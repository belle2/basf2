#include <framework/conditions/ConditionsService.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <TFile.h>
#include <TList.h>
#include <TXMLEngine.h>
#include <TMD5.h>
#include <TSystem.h>

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

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

  m_runPayloads.clear();

  CURL* curl;
  CURLcode res;

  curl = curl_easy_init();

  int count = 0;

  if (curl) {
    std::string REST_payloads = m_RESTbase + "iovPayloads/?gtName=" + GlobalTag + "&expName=" + ExperimentName + "&runName=" + RunName;
    B2INFO("rest payload call: " << REST_payloads);
    m_buffer.clear();
    curl_easy_setopt(curl, CURLOPT_URL, REST_payloads.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_return);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&count);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // For debug.

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      B2ERROR("curl_easy_perform() failed: " << curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
  }
  parse_payloads(m_buffer);

  B2INFO("Conditions service retrieved " << m_runPayloads.size() << " payloads for experiment " << ExperimentName << " and run " <<
         RunName << " listed under global tag " << GlobalTag);

  return ;
}

void DisplayNodes(TXMLEngine* xml, XMLNodePointer_t node, Int_t level)
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
    DisplayNodes(xml, child, level + 2);
    child = xml->GetNext(child);
  }
}


void ConditionsService::parse_return(std::string temp)
{


  TXMLEngine* xml = new TXMLEngine;

  XMLDocPointer_t xmldoc = xml->ParseString(temp.c_str());
  if (xmldoc == 0) {
    B2FATAL("corrupt return from REST call: " << temp.c_str());
  }


  // take access to main node
  XMLNodePointer_t mainnode = xml->DocGetRootElement(xmldoc);

  // display recursively all nodes and subnodes
  DisplayNodes(xml, mainnode, 1);

}

size_t ConditionsService::capture_return(void* buffer, size_t size, size_t nmemb, void* userp)
{

  std::string temp(static_cast<const char*>(buffer), size * nmemb);
  int count = *((int*)userp);
  count = 1;
  ConditionsService::getInstance()->addReturn(temp);
  return size * nmemb * count;

}


void ConditionsService::parse_payloads(std::string temp)
{

  TXMLEngine* xml = new TXMLEngine;

  XMLDocPointer_t xmldoc = xml->ParseString(temp.c_str());
  if (xmldoc == 0) {
    B2FATAL("corrupt return from REST call: " << temp.c_str());
  }

  // take access to main node
  XMLNodePointer_t mainnode = xml->DocGetRootElement(xmldoc);

  // display recursively all nodes and subnodes
  // DisplayNodes(xml, mainnode, 1);

  // parse the payloads
  XMLNodePointer_t iov_node = xml->GetChild(mainnode);
  XMLNodePointer_t payload_node = xml->GetChild(iov_node);

  XMLNodePointer_t child_node;

  std::string PackageName, ModuleName, PayloadURL, Checksum;

  XMLAttrPointer_t attr;
  std::string nodeName;

  while (payload_node) {

    nodeName = xml->GetNodeName(payload_node);
    attr = xml->GetFirstAttr(payload_node);
    B2INFO("Parsing payload... " << nodeName << " with id " << xml->GetAttrValue(attr));
    if (nodeName == "payload") { /// Found a payload, now parse the needed information.
      child_node = xml->GetChild(payload_node);
      while (child_node) { // Search for the module
        nodeName = xml->GetNodeName(child_node);

        if (nodeName == "payloadUrl") {
          PayloadURL = xml->GetNodeContent(child_node);
        }
        if (nodeName == "checksum") {
          Checksum = xml->GetNodeContent(child_node);
        }
        if (nodeName == "basf2Module") { // module found
          XMLNodePointer_t module_node = child_node;
          child_node = xml->GetChild(child_node);
          while (child_node) { // Search for the package
            nodeName = xml->GetNodeName(child_node);
            if (nodeName == "name") {
              // Get the module name.
              ModuleName = xml->GetNodeContent(child_node);
            } else if (nodeName == "basf2Package") {
              XMLNodePointer_t package_node = child_node;
              child_node = xml->GetChild(child_node);
              while (child_node) { // Search for the package name
                nodeName = xml->GetNodeName(child_node);
                if (nodeName == "name") {
                  // Get the module name.
                  PackageName = xml->GetNodeContent(child_node);
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

      if (PackageName.size() == 0 || ModuleName.size() == 0 || PayloadURL.size() == 0) {
        B2ERROR("ConditionsService::parse_payload Payload not parsed correctly.");
      } else {
        std::string payloadKey = PackageName + ModuleName;
        if (ConditionsService::getInstance()->payloadExists(payloadKey)) {
          B2FATAL("Found duplicate payload key " << payloadKey << " while parsing conditions payloads. ");
        }
        B2INFO("Found payload for module " << ModuleName << " in package " << PackageName << " at URL " << PayloadURL <<
               ".  Storing with key: " << payloadKey << " and checksum: " << Checksum);
        ConditionsService::getInstance()->addPayloadURL(payloadKey, PayloadURL);
        ConditionsService::getInstance()->addChecksum(payloadKey, Checksum);
      }
    }
    iov_node = xml->GetNext(iov_node);
    payload_node = xml->GetChild(iov_node);
  }

  // Release memory before exit
  xml->FreeDoc(xmldoc);
  delete xml;


}


void ConditionsService::writePayloadFile(std::string payloadFileName,
                                         std::string packageName,
                                         std::string moduleName)
{

  TMD5* checksum = TMD5::FileChecksum(payloadFileName.c_str());

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
      B2ERROR("curl_easy_perform() failed: " << curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    parse_return(m_buffer);
  }

}

size_t ConditionsService:: write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
}


std::string ConditionsService::getPayloadFileURL(std::string packageName, std::string moduleName)
{

  if (!payloadExists(packageName + moduleName)) {
    B2FATAL("Unable to find conditions payload for requested package " << packageName << "\t and module " << moduleName);
  }
  std::string remote_file = m_runPayloads[packageName + moduleName];
  std::string local_file = m_FILEbase + remote_file; // This will work for local files and CVMFS.



  if (m_FILEbase.substr(0, 7) == "http://") { // May need to transfer files locally.
    local_file = m_FILElocal + boost::filesystem::path(remote_file).filename().string();
    TMD5* checksum = TMD5::FileChecksum(local_file.c_str()); // check if the file exists

    remote_file = m_FILEbase + remote_file;
    if (!checksum) { // file isn't there.  Start downloading.

      B2INFO("Did not find file " << local_file << " starting download from " << remote_file);

      CURL* curl;
      FILE* fp;
      CURLcode res;

      curl = curl_easy_init();
      if (curl) {
        fp = fopen(local_file.c_str(), "wb");
        curl_easy_setopt(curl, CURLOPT_URL, remote_file.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
        //  curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        fclose(fp);

        if (res != CURLE_OK) {
          B2ERROR("libcurl error code " << res << " trying to download file.")
        }
      }
      checksum = TMD5::FileChecksum(local_file.c_str()); // check checksum
    } else if (checksum->AsString() == m_runChecksums[packageName + moduleName]) {
      B2INFO("Found file: " << local_file << " with correct MD5 checksum: " << checksum->AsString());
      return local_file;
    }


    TMD5* checksum_new;
    while (checksum->AsString() != m_runChecksums[packageName + moduleName]) { // then there was a checksum mis-match
      gSystem->Sleep(1000);
      checksum_new = TMD5::FileChecksum(local_file.c_str()); // check checksum again
      if (std::string(checksum->AsString()) != std::string(checksum_new->AsString())) {   // Then we are downloading the file already.
        B2INFO("File with incorrect checksum found, download appears to be occuring... waiting for file to complete ");
        B2INFO("checksum: " << checksum->AsString() << "\tchecksum_new: " << checksum_new->AsString());
        checksum = checksum_new;
      } else { // File isn't downloading, but checksums don't match.  Throw an error.
        B2FATAL("Error with file " << local_file.c_str() << " checksum expected: " << m_runChecksums[packageName + moduleName] <<
                " found: " << checksum->AsString());
        return NULL;
      }

    }
  }

  TMD5* checksum = TMD5::FileChecksum(local_file.c_str()); // check if the file exists
  if (!checksum) { // file isn't there.  Toss an error.
    B2ERROR("Did not find file " << local_file << " check inputs.");
  } else if (std::string(checksum->AsString()) != m_runChecksums[packageName +
             moduleName]) { // MD5 checksum doesn't match the database entry.
    B2FATAL("Conditions file " << local_file << " error! Expected MD5 checksum " << m_runChecksums[packageName + moduleName] <<
            " and calculated checksum " << checksum->AsString());
    return NULL;
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
