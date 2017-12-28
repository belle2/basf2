#ifndef _Belle2_ErrdiagCallback_h
#define _Belle2_ErrdiagCallback_h

#include "daq/slc/nsm/NSMCallback.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DAQLogDB.h>

using namespace boost::property_tree;

namespace Belle2 {

  class ErrdiagCallback : public NSMCallback {

  public:
    ErrdiagCallback(const std::string& nodename, const std::string xmlpath, int timout = 5, int max = 10, int offset = 0);
    virtual ~ErrdiagCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();
    virtual void show(ptree apt, std::string str) throw();
    virtual void analysis(ptree pt, DAQLogMessageList& logs, std::vector<int>& err_line) throw();
    virtual int test2() throw();
    virtual void map_init() throw();


    static int cpr_eagain(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int cpr_read_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int cpr_read_less(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int cpr_read_more(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int cpr_bad_slot_arg(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int cpr_open_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();


    static int rpc_no_resolve(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rpc_sendhdr_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rpc_b2l_hdrtrl_mismatch(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rpc_redsize_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int no_cprdata(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int no_resolve(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int failed_to_bind(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int failed_to_accept(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int failed_toset_timeout(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int post_nocpr_ctr(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int post_nofunc(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rcpr_invalid_format_ver(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rftsw_event_jump(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rftsw_invalid_ftsw_length(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rftsw_invalid_magic(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rcpr_bad_slot_arg(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rcpr_nohslb_indata(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rcpr_diff_evenum_hslbs(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rcpr_nofunc(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rdblk_nolength(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rdblk_invalid_blocknum(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rdblk_strange_length(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int rdblk_invalid_length(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_invalid_cpr_magic_1(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_bad_slot_argument(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_no_hslb_data(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_diff_eve_overhslb(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_old_format(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_invalid_cpr_magic_2(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_diff_length(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_eve_jump(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_cprcounter_jump(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_invalidevenum_runstart(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_cprdrv_chksum_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_rcpr_chksum_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_mismatch_hdr_over_hslbs(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_mismatch_b2lhdr_trl(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_invalid_blocknum(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_nohslb(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_invalid_length(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_cprdrb_chksum_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_invalid_magic(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_event_jump(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_oldftsw_firmware(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_nohslb_data(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_hslb_datasize_small(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_xor_chksum_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_CRC16_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_nofunc(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int pre_invalid_slot_arg(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int post_strange_nwords(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int post_noevenum(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int post_xor_chksum_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int post_no_magic_cprhdr(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();

    static int post_nodata(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();
    static int post_CrC16_err(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval) throw();



    PostgreSQLInterface* db;

    std::string m_xmlpath;
    ptree m_pt;

    int m_max;
    int m_offset;

    typedef struct entry {    // w
      int state;
      int line;
    } run_state;

    typedef int(*function)(DAQLogMessageList& logs, std::vector<int>& err_line, std::string& tval);

    std::map<std::string, function> m_map;

  };

}

#endif
