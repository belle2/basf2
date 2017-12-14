#include "daq/slc/apps/errdiagd/ErrdiagCallback.h"

#include <daq/slc/system/LogFile.h>

#include <stdlib.h>

#include <iostream>

///////////////////////
// From daqlogget.cc //

#include <daq/slc/database/DAQLogDB.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
///////////////////////

#define ENV_SLC_PATH        "BELLE2_DAQ_SLC"


namespace Belle2 {
  class ErrdiagVHandler : public NSMVHandlerText {
  public:
    ErrdiagVHandler(const std::string& name, const std::string& val)
      : NSMVHandlerText(name, true, true, val) {}
    bool handleGetText(std::string& val)
    {
      NSMVHandlerText::handleGetText(val);
      LogFile::info("%s is read : %s", getName().c_str(), val.c_str());
      return true;
    }
    bool handleSetText(const std::string& val)
    {
      LogFile::info("%s is written : %s", getName().c_str(), val.c_str());
      return NSMVHandlerText::handleSetText(val);
    }
  };
}

using namespace Belle2;

ErrdiagCallback::ErrdiagCallback(const std::string& name, int timeout)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
}

ErrdiagCallback::~ErrdiagCallback() throw()
{
}

void ErrdiagCallback::init(NSMCommunicator&) throw()
{
  add(new NSMVHandlerInt("ival", true, false, 10));
  add(new NSMVHandlerFloat("fval", true, true, 0.1));
  add(new ErrdiagVHandler("tval", "example"));

  char temp_path[256];
  const std::string slc_path = getenv(ENV_SLC_PATH);

  sprintf(temp_path, "%s/data/errdiag/HLT_Diagnosis.xml", slc_path.c_str());
  read_xml(temp_path, m_pt_hlt);

  sprintf(temp_path, "%s/data/errdiag/CPRROPC_Diagnosis.xml", slc_path.c_str());
  read_xml(temp_path, m_pt_cprropc);

  sprintf(temp_path, "%s/data/errdiag/EB_Diagnosis.xml", slc_path.c_str());
  read_xml(temp_path, m_pt_eb);

  sprintf(temp_path, "%s/data/errdiag/SLC_Diagnosis.xml", slc_path.c_str());
  read_xml(temp_path, m_pt_slc);

  sprintf(temp_path, "%s/data/errdiag/TTD_Diagnosis.xml", slc_path.c_str());
  read_xml(temp_path, m_pt_ttd);

  sprintf(temp_path, "%s/data/errdiag/FEE_Diagnosis.xml", slc_path.c_str());
  read_xml(temp_path, m_pt_fee);

  sprintf(temp_path, "%s/data/errdiag/STORAGE_Diagnosis.xml", slc_path.c_str());
  read_xml(temp_path, m_pt_storage);

  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));

  m_db = &db;

}

void ErrdiagCallback::timeout(NSMCommunicator&) throw()
{

  const std::string tablename = "logdaq";
  int max = 3;

  std::string nodename;
  std::stringstream ss_begin;
  std::stringstream ss_end;
  bool colored = true;


  DAQLogMessageList logs = DAQLogDB::getLogs(*m_db, tablename, nodename,
                                             ss_begin.str(), ss_end.str(), max);


  for (size_t i = 0; i < logs.size(); i++) {
    // if (colored) {
    //   switch (logs[i].getPriority()) {
    //     case LogFile::DEBUG:   std::cout << "\x1b[49m\x1b[39m"; break;
    //     case LogFile::INFO:    std::cout << "\x1b[49m\x1b[32m"; break;
    //     case LogFile::NOTICE:  std::cout << "\x1b[49m\x1b[34m"; break;
    //     case LogFile::WARNING: std::cout << "\x1b[49m\x1b[35m"; break;
    //     case LogFile::ERROR:   std::cout << "\x1b[49m\x1b[31m"; break;
    //     case LogFile::FATAL:   std::cout << "\x1b[41m\x1b[37m"; break;
    //     default: break;
    //   }
    // }
    std::cout << "[" << logs[i].getNodeName() << "] ["
              << logs[i].getDate().toString() << "] ["
              << logs[i].getPriorityText() << "] "
              << logs[i].getMessage() << std::endl;

    show(m_pt_fee, logs[i].getMessage());
    show(m_pt_ttd, logs[i].getMessage());
    show(m_pt_cprropc, logs[i].getMessage());
    show(m_pt_hlt, logs[i].getMessage());
    show(m_pt_eb, logs[i].getMessage());
    show(m_pt_storage, logs[i].getMessage());
    show(m_pt_slc, logs[i].getMessage());
    // if (colored) {
    //   std::cout << "\x1b[49m\x1b[39m";
    // }
    //    std::cout << std::endl;
  }
}


int ErrdiagCallback::test1() throw()
{
  printf("Test1 function is called\n");
  return 0;
}

int ErrdiagCallback::test2() throw()
{
  printf("Test2 function is called\n");
  return 0;
}


void ErrdiagCallback::show(ptree pt, std::string str) throw()
{
  printf("mon 1 %s\n", str.c_str());
  std::string tval = "I have no idea.";

  BOOST_FOREACH(const ptree::value_type & child, pt.get_child("ErrorMessageDiagnosis")) {
    printf("mon 1.5\n");
    //    const boost::property_tree::ptree &pt_child = child.second();
    boost::optional<std::string> str2 = child.second.get_optional<std::string>("Message");
    printf("mon 2 : %s\n", str2->c_str());
    if ((int)(str.find(str2->c_str()))  >= 0) {    // string::nopos = -1
      printf("mon 3\n");

      boost::optional<std::string> str3 = child.second.get_optional<std::string>("Diagnosis");
      boost::optional<std::string> str4 = child.second.get_optional<std::string>("Function");

      //      std::cout << str << std::endl;
      //      std::cout << str3 << std::endl;
      //      std::cout << str4 << std::endl;
      //      printf("mon 4 %s %s\n", tval.c_str());
      tval = *str3;
      printf("mon 5\n");

      //      strcpy( const_cast<char*>(tval.c_str()), str3->c_str() );

      //      sprintf( tval.c_str(), "%s", str4->c_str() );


      if (str4 != boost::none) {
        printf("mon 5.1\n");
        if (std::strcmp(str4->c_str(), "TEST1") == 0) {
          printf("mon 5.2\n");
          test1();
        } else if (std::strcmp(str4->c_str(), "TEST2") == 0) {
          printf("mon 5.3\n");
          test2();
        }
      }
      printf("mon 6\n");
      set("tval", tval.c_str());
      LogFile::debug("tval updated: %s", tval.c_str());

    } else {
      //      std::cout << str2 << std::endl;
    }
  }
  printf("mon 7\n");
  //  printf("Set\n");fflush(stdout);
  printf("mon 9\n");
  return;
}
