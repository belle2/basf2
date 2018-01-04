#include "daq/slc/apps/errdiagd/ErrdiagCallback.h"

#include <daq/slc/system/LogFile.h>

#include <stdlib.h>

#include <iostream>



///////////////////////
// From daqlogget.cc //



#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
///////////////////////


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

ErrdiagCallback::ErrdiagCallback(const std::string& name, const std::string xmlpath, int timeout, int max, int offset)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);

  m_xmlpath = xmlpath;
  m_max = max;
  m_offset = offset;
  m_cnt = 0;
}

ErrdiagCallback::~ErrdiagCallback() throw()
{
}

void ErrdiagCallback::init(NSMCommunicator&) throw()
{
  add(new NSMVHandlerInt("ival", true, false, 10));
  add(new NSMVHandlerFloat("fval", true, true, 0.1));
  add(new ErrdiagVHandler("tval", "No information."));
  add(new ErrdiagVHandler("request", "skip"));

  //  read_xml("/home/usr/yamadas/slc/database/tools/temp2.xml", m_pt);
  read_xml(m_xmlpath, m_pt);

  ConfigFile config("slowcontrol");
  static PostgreSQLInterface db_temp(config.get("database.host"),
                                     config.get("database.dbname"),
                                     config.get("database.user"),
                                     config.get("database.password"),
                                     config.getInt("database.port"));
  db = &db_temp;
  //  db = &db_temp;
  // PostgreSQLInterface db(config.get("database.host"),
  //                        config.get("database.dbname"),
  //                        config.get("database.user"),
  //                        config.get("database.password"),
  //                        config.getInt("database.port"));

}

void ErrdiagCallback::timeout(NSMCommunicator&) throw()
{
  //  if( m_cnt > 0 )return;
  m_cnt++;
  int ival = rand() % 256;

  std::string request;
  get("request", request);
  std::cout << "State " << request << std::endl;
  LogFile::debug("ivaldfsdfdsfse updated: %d", ival);
  if (request != "check") return;
  std::cout << "Checking" << std::endl;

  //  read_xml("/home/usr/yamadas/slc/database/tools/temp2.xml", m_pt);

  //  const std::string tablename = argv[1];
  const std::string tablename = "logtest";
  std::string nodename;
  std::stringstream ss_begin;
  std::stringstream ss_end;
  bool colored = true;
  ///06/03 00:00:00f, eYYYY/MM/DD HH24:MI:SSf)
  //(timestamp '2000-09-01 01:00:00')

  ss_begin << "(timestamp '2017-12-23 17:46:41')";
  ss_end << "(timestamp '2017-12-23 17:49:17')"; // "2017-12-23 17:49:17";

  // ss_begin << "2017-12-23 17:46:41";
  // ss_end << "2017-12-23 17:49:17"; // "2017-12-23 17:49:17";

  // ConfigFile config("slowcontrol");
  // PostgreSQLInterface db1(config.get("database.host"),
  //                        config.get("database.dbname"),
  //                        config.get("database.user"),
  //                         config.get("database.password"),
  //                        config.getInt("database.port"));
  //  printf("Check 1 db %p db1=%p\n",db, &db1);fflush(stdout);

  map_init();

  DAQLogMessageList logs = DAQLogDB::getLogs(*db, tablename, nodename,
                                             //                "","2017-12-10 18:00:00" , max );
                                             m_max, m_offset);
  //                                             ss_begin.str(), ss_end.str(), max);

  int running = -1;
  int log_size = logs.size();

  std::vector< run_state > state;
  std::vector<int> err_state;

  std::vector< std::vector<int> > HLT_fatal_linenum;
  std::vector< std::vector<int> > ROPC_fatal_linenum;
  std::vector< std::vector<int> > CPR_fatal_linenum;

  //
  // Store error messages
  //

  for (int i = log_size - 1 ; i >= 0; i--) {
    std::string str_mes = logs[i].getMessage();
    std::string str_pri = logs[i].getPriorityText();
    std::string str_nod = logs[i].getNodeName();

    //
    // Store run-state transition from log messages
    //
    if (str_mes.find("STARTING") != std::string::npos) {
      if (running != 2) {
        // std::cout << "[" << logs[i].getDate().toString() << "] STARTED : " <<  str_mes << std::endl;
        running = 2;
        run_state temp;
        temp.state = running;
        temp.line = i;
        state.push_back(temp);
      }
    } else if (str_mes.find("ABORTING") != std::string::npos) {
      if (running != 0) {
        // std::cout << "[" << logs[i].getDate().toString() << "] ABORTED : " <<  str_mes << std::endl;
        running = 0;
        run_state temp;
        temp.state = running;
        temp.line = i;
        state.push_back(temp);
      }

    } else if (str_mes.find("LOADING") != std::string::npos) {
      if (running != 1) {
        // std::cout << "[" << logs[i].getDate().toString() << "] LOADED : " <<  str_mes << std::endl;
        running = 1;
        for (int j = 0; j < err_state.size(); j++) {
          err_state[j] = -1;
        }
        run_state temp;
        temp.state = running;
        temp.line = i;
        state.push_back(temp);

      }
    }

    // std::cout << i << " " << running << "[" << logs[i].getNodeName() << "] ["
    //              << logs[i].getDate().toString() << "] ["
    //              << logs[i].getPriorityText() << "] "
    //              << logs[i].getMessage()  << std::endl;

    //
    // Store error messages
    //
    int same_node = 0;
    for (int j = 0; j < err_state.size(); j++) {
      if (err_state[j] < 0) continue;
      if (str_nod == logs[ err_state[ j ] ].getNodeName()) {
        ROPC_fatal_linenum[j].push_back(i);
        same_node = 1;
      }
    }
    if (same_node == 0) {
      if (str_pri.find("FATAL") != std::string::npos) {
        err_state.push_back(i);
        std::vector<int> temp;
        temp.push_back(i);
        ROPC_fatal_linenum.push_back(temp);
        // for( int i = 0;  i < ROPC_fatal_linenum.size(); i++ ){
        //   std::cout << "Filled : " << i << " " << ROPC_fatal_linenum[i].size() << " " << temp.size() << std::endl;
        // }
      }
    }
  }

  //    show( m_pt, logs[i].getMessage() );

  // if (colored) {
  //   std::cout << "\x1b[49m\x1b[39m";
  // }
  // std::cout << std::endl;

  //  std::cout << "ROPC error" << std::endl;

  //
  // Analyse error messages
  //
  int state_pos = 0;
  for (int i = 0;  i < ROPC_fatal_linenum.size(); i++) {
    // std::cout << "RUN STATE :err line " << ROPC_fatal_linenum[i][0] << " statesize " << state.size() << " stateline " << state[state_pos].line << " pos " << state_pos << std::endl;
    for (int j = state_pos;  j < state.size(); j++) {
      if (ROPC_fatal_linenum[i][0] > state[state_pos].line) {
        break;
      } else {
        std::cout << "RUN STATE " << state[state_pos].state << std::endl;
        state_pos++;
      }
    }

    // int l = ROPC_fatal_linenum[i][0];
    // std::cout << l << " " << running << "[" << logs[l].getNodeName() << "] ["
    //        << logs[l].getDate().toString() << "] ["
    //        << logs[l].getPriorityText() << "] "
    //        << logs[l].getMessage()  << std::endl;

    //
    // Check log messages of the error node
    //
    for (int j = 0;  j < ROPC_fatal_linenum[i].size(); j++) {
      int k = ROPC_fatal_linenum[i][j];
      std::cout << "err " << i << " : " << j << " [" << logs[k].getNodeName() << "] ["
                << logs[k].getDate().toString() << "] ["
                << logs[k].getPriorityText() << "] "
                << logs[k].getMessage()  << std::endl;
    }
    analysis(m_pt, logs, ROPC_fatal_linenum[i]);
  }
  request = "skip";
  set("request", request);

  return;
}



int ErrdiagCallback::test2() throw()
{
  printf("Test2 function is called\n");
}

void ErrdiagCallback::map_init() throw()
{
  // COPPER
  m_map["cpr_eagain"] = &cpr_eagain;
  m_map["cpr_read_err"] = &cpr_read_err;
  m_map["cpr_read_less"] = &cpr_read_less;
  m_map["cpr_read_more"] = &cpr_read_more;
  m_map["cpr_bad_slot_arg"] = &cpr_bad_slot_arg;
  m_map["cpr_open_err"] = &cpr_open_err;
  m_map["cpr_eagain"] = &cpr_eagain;
  m_map["cpr_read_err"] = &cpr_read_err;
  m_map["rpc_no_resolve"] = &rpc_no_resolve;
  m_map["rpc_sendhdr_err"] = &rpc_sendhdr_err;
  m_map["rpc_sendhdr_err"] = &rpc_sendhdr_err;
  m_map["rpc_b2l_hdrtrl_mismatch"] = &rpc_b2l_hdrtrl_mismatch;
  m_map["rpc_redsize_err"] = &rpc_redsize_err;
  m_map["no_cprdata"] = &no_cprdata;
  m_map["no_resolve"] = &no_resolve;
  m_map["failed_to_bind"] = &failed_to_bind;
  m_map["failed_to_accept"] = &failed_to_accept;
  m_map["failed_toset_timeout"] = &failed_toset_timeout;
  m_map["post_nocpr_ctr"] = &post_nocpr_ctr;
  m_map["post_nofunc"] = &post_nofunc;
  m_map["post_nofunc"] = &post_nofunc;
  m_map["rcpr_invalid_format_ver"] = &rcpr_invalid_format_ver;
  m_map["rftsw_event_jump"] = &rftsw_event_jump;
  m_map["rftsw_invalid_ftsw_length"] = &rftsw_invalid_ftsw_length;
  m_map["rftsw_invalid_magic"] = &rftsw_invalid_magic;
  m_map["rcpr_bad_slot_arg"] = &rcpr_bad_slot_arg;
  m_map["rcpr_bad_slot_arg"] = &rcpr_bad_slot_arg;
  m_map["rcpr_bad_slot_arg"] = &rcpr_bad_slot_arg;
  m_map["rcpr_nohslb_indata"] = &rcpr_nohslb_indata;
  m_map["rcpr_diff_evenum_hslbs"] = &rcpr_diff_evenum_hslbs;
  m_map["rcpr_nofunc"] = &rcpr_nofunc;
  m_map["rcpr_nofunc"] = &rcpr_nofunc;
  m_map["rdblk_nolength"] = &rdblk_nolength;
  m_map["rdblk_invalid_blocknum"] = &rdblk_invalid_blocknum;
  m_map["rdblk_strange_length"] = &rdblk_strange_length;
  m_map["rdblk_invalid_length"] = &rdblk_invalid_length;
  m_map["pre_invalid_cpr_magic_1"] = &pre_invalid_cpr_magic_1;
  m_map["pre_bad_slot_argument"] = &pre_bad_slot_argument;
  m_map["pre_no_hslb_data"] = &pre_no_hslb_data;
  m_map["pre_diff_eve_overhslb"] = &pre_diff_eve_overhslb;
  m_map["pre_old_format"] = &pre_old_format;
  m_map["pre_invalid_cpr_magic_2"] = &pre_invalid_cpr_magic_2;
  m_map["pre_diff_length"] = &pre_diff_length;
  m_map["pre_eve_jump"] = &pre_eve_jump;
  m_map["pre_cprcounter_jump"] = &pre_cprcounter_jump;
  m_map["pre_invalidevenum_runstart"] = &pre_invalidevenum_runstart;
  m_map["pre_cprdrv_chksum_err"] = &pre_cprdrv_chksum_err;
  m_map["pre_rcpr_chksum_err"] = &pre_rcpr_chksum_err;
  m_map["pre_mismatch_hdr_over_hslbs"] = &pre_mismatch_hdr_over_hslbs;
  m_map["pre_mismatch_b2lhdr_trl"] = &pre_mismatch_b2lhdr_trl;
  m_map["pre_invalid_blocknum"] = &pre_invalid_blocknum;
  m_map["pre_nohslb"] = &pre_nohslb;
  m_map["pre_invalid_length"] = &pre_invalid_length;
  m_map["pre_cprdrb_chksum_err"] = &pre_cprdrb_chksum_err;
  m_map["pre_invalid_magic"] = &pre_invalid_magic;
  m_map["pre_event_jump"] = &pre_event_jump;
  m_map["pre_oldftsw_firmware"] = &pre_oldftsw_firmware;
  m_map["pre_nohslb_data"] = &pre_nohslb_data;
  m_map["pre_hslb_datasize_small"] = &pre_hslb_datasize_small;
  m_map["pre_xor_chksum_err"] = &pre_xor_chksum_err;
  m_map["pre_CRC16_err"] = &pre_CRC16_err;
  m_map["pre_CRC16_err"] = &pre_CRC16_err;
  m_map["pre_nofunc"] = &pre_nofunc;
  m_map["pre_invalid_slot_arg"] = &pre_invalid_slot_arg;
  m_map["post_strange_nwords"] = &post_strange_nwords;
  m_map["post_noevenum"] = &post_noevenum;
  m_map["post_xor_chksum_err"] = &post_xor_chksum_err;
  m_map["post_no_magic_cprhdr"] = &post_no_magic_cprhdr;
  m_map["post_nofunc"] = &post_nofunc;
  m_map["post_nofunc"] = &post_nofunc;
  m_map["post_nofunc"] = &post_nofunc;
  m_map["post_nodata"] = &post_nodata;
  m_map["post_CrC16_err"] = &post_CrC16_err;
  m_map["post_CrC16_err"] = &post_CrC16_err;

  //  m_func[0] = &invalid_event_num;

  //   // ROPC

  //   //
}



void ErrdiagCallback::analysis(ptree pt, DAQLogMessageList& logs, std::vector<int>& err_line) throw()
{

  BOOST_FOREACH(const ptree::value_type & child, pt.get_child("ErrorMessageDiagnosis")) {
    //    const boost::proxperty_tree::ptree &pt_child = child.second();
    boost::optional<std::string> mesg = child.second.get_optional<std::string>("Message");
    // std::cout << "MES :[" << logs[err_line[0]].getMessage() << "]" << std::endl;
    // std::cout << "XML :[" << mesg  << "]"<< std::endl;

    if ((int)((logs[err_line[0]].getMessage()).find(mesg->c_str()))  >= 0) {       // string::nopos = -1

      boost::optional<std::string> diag = child.second.get_optional<std::string>("Diagnosis");
      boost::optional<std::string> funcname = child.second.get_optional<std::string>("Function");
      //      std::string str5 = str4->c_str();
      //      printf( "%p\n",m_map[*str4]);
      function func = m_map[*funcname];
      if (func != NULL) {
        std::string temp_str;
        func(logs, err_line, temp_str);
        std::cout << "FUNCTION : " << funcname << " " << temp_str << std::endl;
        temp_str = logs[err_line[0]].getNodeName() + " : " + temp_str;
        set("tval", temp_str);
      }


      //      temp_func[0](logs, err_line);

      //      std::cout << str << std::endl;
      //      std::cout << str3 << std::endl;
      // std::cout << str3 << std::endl;
      // set( "tval", str3->c_str() );
      // if( std::strcmp( str4->c_str(), "TEST1" ) == 0 ){
      //  printf("Check 4.1\n"); fflush(stdout);
      //  test1();
      //  printf("Check 4.2\n"); fflush(stdout);
      // }else if( std::strcmp( str4->c_str(), "TEST2" ) == 0 ){
      //  printf("Check 4.3\n"); fflush(stdout);
      //  test2();
      //  printf("Check 4.4\n"); fflush(stdout);
      // }
    } else {
      //      std::cout << str2 << std::endl;
    }
  }
  int ival = rand() % 256;
  set("ival", ival);
  //  LogFile::debug("ival updated: %d", ival);
  return;

}


void ErrdiagCallback::show(ptree pt, std::string str) throw()
{

  BOOST_FOREACH(const ptree::value_type & child, pt.get_child("ErrorMessageDiagnosis")) {
    //    const boost::proxperty_tree::ptree &pt_child = child.second();
    boost::optional<std::string> str2 = child.second.get_optional<std::string>("Message");
    if ((int)(str.find(str2->c_str()))  >= 0) {    // string::nopos = -1
      boost::optional<std::string> str3 = child.second.get_optional<std::string>("Diagnosis");
      boost::optional<std::string> str4 = child.second.get_optional<std::string>("Function");
      //      std::cout << str << std::endl;
      //      std::cout << str3 << std::endl;
      //      std::cout << str3 << std::endl;
      //      set( "tval", str3->c_str() );
      // if( std::strcmp( str4->c_str(), "TEST1" ) == 0 ){
      //  printf("Check 4.1\n"); fflush(stdout);
      //  test1();
      //  printf("Check 4.2\n"); fflush(stdout);
      // }else if( std::strcmp( str4->c_str(), "TEST2" ) == 0 ){
      //  printf("Check 4.3\n"); fflush(stdout);
      //  test2();
      //  printf("Check 4.4\n"); fflush(stdout);
      // }
    } else {
      //      std::cout << str2 << std::endl;
    }
  }
  int ival = rand() % 256;
  set("ival", ival);
  //  LogFile::debug("ival updated: %d", ival);
  return;
}

// check whether CRC error occurred or not
int ErrdiagCallback::check_crcerrlog(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  std::string err;
  for (int i = 0 ; i < logs.size(); i++) {
    err = logs[ i ].getMessage();
    if (err.find("POST B2link event CRC16 error") != std::string::npos ||
        err.find("PRE CRC16 error") != std::string::npos ||
        err.find("ERROR_EVENT : B2LCRC16") != std::string::npos) {   // std::string::npos = -1
      return 1; // CRC error !!
    }
  }
  return 0; // No CRC error. FEE data are strange.
}

//
// Functions : COPPER/ROPC subsystem
//
int ErrdiagCallback::cpr_bad_slot_arg(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

//
// Rare errors
//
int ErrdiagCallback::cpr_eagain(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::cpr_read_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::cpr_read_less(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::cpr_read_more(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::cpr_open_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rpc_no_resolve(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rpc_redsize_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rpc_sendhdr_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}


int ErrdiagCallback::no_cprdata(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::no_resolve(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::failed_to_bind(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::failed_to_accept(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::failed_toset_timeout(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::post_nofunc(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

//
// FTSW errors
//

int ErrdiagCallback::rftsw_event_jump(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rftsw_invalid_ftsw_length(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rftsw_invalid_magic(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::post_nocpr_ctr(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}


int ErrdiagCallback::pre_invalid_cpr_magic_1(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

//
// CRC check errors
//

//
// event # jump
//
int ErrdiagCallback::rcpr_diff_evenum_hslbs(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  if (check_crcerrlog(logs, err_line, tval) == 0) {
    tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  } else {
    tval = "sub-detector FEE side error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the sub-detector/DAQ expert shifter.";
  }
  return 0;
}

int ErrdiagCallback::pre_diff_eve_overhslb(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  if (check_crcerrlog(logs, err_line, tval) == 0) {
    tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  } else {
    tval = "sub-detector FEE side error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the sub-detector/DAQ expert shifter.";
  }

  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_eve_jump(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  if (check_crcerrlog(logs, err_line, tval) == 0) {
    tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  } else {
    tval = "sub-detector FEE side error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the sub-detector/DAQ expert shifter.";
  }

  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_invalidevenum_runstart(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  if (check_crcerrlog(logs, err_line, tval) == 0) {
    tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  } else {
    tval = "sub-detector FEE side error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the sub-detector/DAQ expert shifter.";
  }

  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_event_jump(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  if (check_crcerrlog(logs, err_line, tval) == 0) {
    tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  } else {
    tval = "sub-detector FEE side error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the sub-detector/DAQ expert shifter.";
  }

  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

//
// hdr/trl mismatch
//
int ErrdiagCallback::rpc_b2l_hdrtrl_mismatch(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  if (check_crcerrlog(logs, err_line, tval) == 0) {
    tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  } else {
    tval = "sub-detector FEE side error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the sub-detector/DAQ expert shifter.";
  }
  return 0;
}

int ErrdiagCallback::pre_mismatch_hdr_over_hslbs(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  if (check_crcerrlog(logs, err_line, tval) == 0) {
    tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  } else {
    tval = "sub-detector FEE side error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the sub-detector/DAQ expert shifter.";
  }
  return 0;
}

int ErrdiagCallback::pre_mismatch_b2lhdr_trl(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  if (check_crcerrlog(logs, err_line, tval) == 0) {
    tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  } else {
    tval = "sub-detector FEE side error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the sub-detector/DAQ expert shifter.";
  }
  return 0;
}


int ErrdiagCallback::pre_CRC16_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{

  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::post_CrC16_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

//
// non CRC errors
//
int ErrdiagCallback::pre_invalid_magic(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rcpr_invalid_format_ver(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rcpr_bad_slot_arg(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rcpr_nohslb_indata(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rcpr_nofunc(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rdblk_nolength(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rdblk_invalid_blocknum(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rdblk_strange_length(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::rdblk_invalid_length(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_bad_slot_argument(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_no_hslb_data(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_old_format(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_invalid_cpr_magic_2(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_diff_length(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_cprcounter_jump(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_cprdrv_chksum_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_rcpr_chksum_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}


int ErrdiagCallback::pre_invalid_blocknum(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_nohslb(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_invalid_length(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_cprdrb_chksum_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}


int ErrdiagCallback::pre_oldftsw_firmware(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_nohslb_data(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_hslb_datasize_small(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_xor_chksum_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_nofunc(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::pre_invalid_slot_arg(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::post_strange_nwords(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::post_noevenum(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::post_xor_chksum_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::post_no_magic_cprhdr(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

int ErrdiagCallback::post_nodata(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw()
{
  tval = "DAQ error. Please retry data-taking for a few more times by ABORT-LOAD-START. If it does not work, call the DAQ expert shifter.";
  return 0;
}

