//+
// File : RawCOPPER.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawCOPPER.h>

#include <framework/utilities/HTML.h>


using namespace std;
using namespace Belle2;

//#define DESY
//#define NO_DATA_CHECK
//#define WO_FIRST_EVENUM_CHECK

RawCOPPER::RawCOPPER()
{
  m_access = NULL;
  m_version = -1;
}

RawCOPPER::~RawCOPPER()
{
  if (m_access != NULL) delete m_access;
  m_access = NULL;
}

void RawCOPPER::SetVersion()
{

  if (m_buffer == NULL) {
    perror("m_buffer is NULL. Exiting...");
    exit(1);
  }

  if (m_access != NULL) {
    delete m_access;
  }

  m_version = ((m_buffer[ POS_FORMAT_VERSION ]) & FORMAT_MASK) >> 8;
  switch (m_version) {
    case LATEST_POSTREDUCTION_FORMAT_VER :
      m_access = new PostRawCOPPERFormat_latest;
      //            printf("Calling PostRawCOPPERFormat_latest\n");
      break;
    case (0x80 + LATEST_POSTREDUCTION_FORMAT_VER) :
      m_access = new PreRawCOPPERFormat_latest;
      //            printf("Calling PreRawCOPPERFormat_latest\n");
      break;
    case 0x1 :
      m_access = new PostRawCOPPERFormat_v1;
      //      printf("Calling RawCOPPERFormat_v1\n");
      break;
    case (0x80 + 0x1) :
      m_access = new PreRawCOPPERFormat_v1;
      //            printf("Calling PreRawCOPPERFormat_latest\n");
      break;
    case 0 :
      m_access = new RawCOPPERFormat_v0;
      //      printf("Calling RawCOPPERFormat_v0\n");
      break;
    default : {
      char err_buf[500];
      sprintf(err_buf, "[FATAL] Invalid version of a data format(0x%.8x = 0x7f7f**..). Exiting...\n %s %s %d\n",
              m_buffer[ POS_FORMAT_VERSION ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("%s", err_buf); fflush(stdout);
      string err_str = err_buf; throw (err_str);
    }
    exit(1);
  }

  m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);



}

void RawCOPPER::SetVersion(string class_name)
{

  if (m_access != NULL) {
    delete m_access;
  }

  if (class_name == "PostRawCOPPERFormat_latest") {
    m_access = new PostRawCOPPERFormat_latest;
    m_version = (0 << 7) | LATEST_POSTREDUCTION_FORMAT_VER;
  } else if (class_name == "PreRawCOPPERFormat_latest") {
    m_access = new PreRawCOPPERFormat_latest;
    m_version = (1 << 7) | LATEST_POSTREDUCTION_FORMAT_VER;
  } else if (class_name == "PostRawCOPPERFormat_v1") {
    m_access = new PostRawCOPPERFormat_v1;
    m_version = (0 << 7) | 1;
  } else if (class_name == "PreRawCOPPERFormat_v1") {
    m_access = new PreRawCOPPERFormat_v1;
    m_version = (1 << 7) | 1;
  } else if (class_name == "RawCOPPERFormat_v0") {
    m_access = new RawCOPPERFormat_v0;
    m_version = (0 << 7) | 0;
  } else {
    char err_buf[500];
    sprintf(err_buf, "Invalid name of a data format class (%s). Exiting...\n %s %s %d\n",
            class_name.c_str(), __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    string err_str = err_buf;
    throw (err_str);
  }

//   if( class_name == "RawCOPPERFormat_v0" ){
//     m_access = new RawCOPPERFormat_v0;
//     m_version = ( 0 << 7 ) | 0;
//   }else if( class_name == "PreRawCOPPERFormat_v0" ){
//     m_access = new PreRawCOPPERFormat_v0;
//     m_version = ( 1 << 7 ) | 0;
//   }else if( class_name == "RawCOPPERFormat_v2" ){
//     m_access = new RawCOPPERFormat_v2;
//     m_version = ( 0 << 7 ) | 2;
//   }else if( class_name == "PreRawCOPPERFormat_v2" ){
//     m_access = new PreRawCOPPERFormat_v2;
//     m_version = ( 1 << 7 ) | 2;
//   }
  m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);

}



void RawCOPPER::SetBuffer(int* bufin, int nwords, int delete_flag, int num_events, int num_nodes)
{
  if (bufin == NULL) {
    printf("[DEBUG] bufin is NULL. Exting...\n");
    exit(1);
  }
  if (!m_use_prealloc_buf && m_buffer != NULL) delete[] m_buffer;

  if (delete_flag == 0) {
    m_use_prealloc_buf = true;
  } else {
    m_use_prealloc_buf = false;
  }

  m_nwords = nwords;
  m_buffer = bufin;

  m_num_nodes = num_nodes;
  m_num_events = num_events;

  SetVersion();

}


void RawCOPPER::ShowBuffer()
{

  printf("\n");
  printf("POINTER %p\n", m_buffer);
  for (int k = 0; k < 100; k++) {
    printf("%.8x ", m_buffer[ k ]);
    if ((k + 1) % 10 == 0) printf("\n%.8x : ", k);
  }
  printf("\n");

}


void RawCOPPER::PackDetectorBuf(
  int* detector_buf_1st, int nwords_1st,
  int* detector_buf_2nd, int nwords_2nd,
  int* detector_buf_3rd, int nwords_3rd,
  int* detector_buf_4th, int nwords_4th,
  RawCOPPERPackerInfo rawcprpacker_info)
{

  if (m_access != NULL) {
    delete m_access;
  }
  m_access = new PostRawCOPPERFormat_latest;
  m_version = LATEST_POSTREDUCTION_FORMAT_VER;
  m_num_events = 1;
  m_num_nodes = 1;

  int* packed_buf = NULL;
  packed_buf = m_access->PackDetectorBuf(&m_nwords,
                                         detector_buf_1st, nwords_1st,
                                         detector_buf_2nd, nwords_2nd,
                                         detector_buf_3rd, nwords_3rd,
                                         detector_buf_4th, nwords_4th,
                                         rawcprpacker_info);

  int delete_flag = 1; // Not use preallocated buffer. Delete m_buffer when destructer is called.
  SetBuffer(packed_buf, m_nwords, delete_flag, m_num_events, m_num_nodes);

  delete_flag = 0; // For m_access, need not to delete m_buffer
  m_access->SetBuffer(m_buffer, m_nwords, delete_flag, m_num_events, m_num_nodes);

  return;
}


void RawCOPPER::PackDetectorBuf4DummyData(
  int* detector_buf_1st, int nwords_1st,
  int* detector_buf_2nd, int nwords_2nd,
  int* detector_buf_3rd, int nwords_3rd,
  int* detector_buf_4th, int nwords_4th,
  RawCOPPERPackerInfo rawcprpacker_info)
{

  if (m_access != NULL) {
    delete m_access;
  }
  m_access = new PreRawCOPPERFormat_latest;
  m_version = LATEST_POSTREDUCTION_FORMAT_VER;
  m_num_events = 1;
  m_num_nodes = 1;

  int* packed_buf = NULL;
  packed_buf = m_access->PackDetectorBuf(&m_nwords,
                                         detector_buf_1st, nwords_1st,
                                         detector_buf_2nd, nwords_2nd,
                                         detector_buf_3rd, nwords_3rd,
                                         detector_buf_4th, nwords_4th,
                                         rawcprpacker_info);

  int delete_flag = 1; // Not use preallocated buffer. Delete m_buffer when destructer is called.
  SetBuffer(packed_buf, m_nwords, delete_flag, m_num_events, m_num_nodes);

  delete_flag = 0; // For m_access, need not to delete m_buffer
  m_access->SetBuffer(m_buffer, m_nwords, delete_flag, m_num_events, m_num_nodes);

  return;
}

std::string RawCOPPER::getInfoHTML() const
{
  std::string s;
  //TODO: All these methods should be const instead.
  RawCOPPER* nonconst_this = const_cast<RawCOPPER*>(this);
  nonconst_this->CheckVersionSetBuffer();

  const int nEntries = nonconst_this->GetNumEntries();
  s += "Entries: " + std::to_string(nEntries);
  s += ", Total size (32bit words): " + std::to_string(m_nwords) + "<br>";

  s += "COPPER version: " + std::to_string(m_version);

  for (int iEntry = 0; iEntry < nEntries; ++iEntry) {
    s += "<h4>Entry " + std::to_string(iEntry) + "</h4>";
    for (int iFinesse = 0; iFinesse < 4; iFinesse++) {
      const int nWords = nonconst_this->GetDetectorNwords(iEntry, iFinesse);
      const int* buf = nonconst_this->GetDetectorBuffer(iEntry, iFinesse);
      s += "<p>Finesse " + std::to_string(iFinesse) + ", Size: " + std::to_string(nWords) + "</p>";
      s += HTML::getHexDump(buf, nWords);
      s += "<br>";
    }
  }

  return s;
}
