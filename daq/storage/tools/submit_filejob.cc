#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/statvfs.h>

using namespace Belle2;

const char* g_table = "fileinfo";

int main(int argc, char** argv)
{
  if (argc < 2 || strcmp(argv[1], "-help") == 0) {
    printf("%s : [-all] [-h hostname] [-t runtype] [-e expnumber] [-r runnumber] [-n number_of_file ] [-f] \n", argv[0]);
    return 1;
  }
  std::string hostname;
  std::string runtype;
  int expno = 0;
  int runno = 0;
  int max = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      i++;
      if (i < argc) hostname = argv[i];
    } else if (strcmp(argv[i], "-t") == 0) {
      i++;
      if (i < argc) runtype = argv[i];
    } else if (strcmp(argv[i], "-e") == 0) {
      i++;
      if (i < argc) expno = atoi(argv[i]);
    } else if (strcmp(argv[i], "-r") == 0) {
      i++;
      if (i < argc) runno = atoi(argv[i]);
    } else if (strcmp(argv[i], "-n") == 0) {
      i++;
      if (i < argc) max = atoi(argv[i]);
    }
  }
  ConfigFile config("slowcontrol");
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  std::stringstream ss;
  ss << "select * from " << g_table << " "
     << "where time_convert is null and time_process is null and time_close is not null and time_runend is not null ";
  if (hostname.size() > 0) ss << "and path_sroot like '" << hostname << ":_%%' ";
  if (runtype.size() > 0) ss << "and runtype = '" << runtype << "' ";
  if (expno > 0) ss << "and expno = " << expno << " ";
  if (runno > 0) ss << "and runno = " << expno << " ";
  ss << "and runno > 0 ";
  ss << "and fileid = " << 0 << " ";
  ss << "and time_convert is null ";
  ss << "order by id";
  if (max > 0) ss << " limit " << max << " ";
  try {
    db.connect();
    db.execute(ss.str().c_str());
    DBRecordList record_v(db.loadRecords());
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      StringList s = StringUtil::split(record.get("path_sroot"), ':');
      std::string srootfile = s[1];
      std::string rootfile = StringUtil::replace(StringUtil::replace(srootfile, ".sroot", ".root"), "storage/", "rootfiles/");
      std::stringstream ss;
      std::string script = record.get("runtype") + "." + StringUtil::form("%04d.%06d", record.getInt("expno"), record.getInt("runno"));
      std::string scriptpath = "/x02/disk01/scripts/" + record.get("runtype") + "/" + StringUtil::form("%04d/%06d/",
                               record.getInt("expno"), record.getInt("runno"));;
      ss << "#!/bin/bash" << std::endl
         << "#$ -S /bin/bash" << std::endl
         << "#$ -q b2daq" << std::endl
         << "#$ -N daq." << StringUtil::form("%d.%d", record.getInt("expno"),
                                             record.getInt("runno")) << "." << record.get("runtype") << std::endl
         << "#$ -o " << scriptpath << "out" << std::endl
         << "#$ -e " << scriptpath << "err" << std::endl
         << std::endl
         << "export BELLE2_NO_TOOLS_CHECK=1" << std::endl
         << "cd ~/storage/belle2/releases/d160226/" << std::endl
         << "source ../../tools/setup_belle2" << std::endl
         << "setuprel > /dev/null" << std::endl
         << "setoption opt" << std::endl
         << "cd " << scriptpath << std::endl
         << "basf2 --no-stat ~/storage/convert.py -i " << srootfile << " -o " << rootfile << std::endl
         << "if [ $? -eq 0 ]; then" << std::endl
         << "update_fileinfo " << record.getInt("id") << " " << s[0] << ":" << rootfile << std::endl
         << "else" << std::endl
         << "echo basf2 failed" << std::endl
         << "fi" << std::endl;
      //LogFile::debug("\n%s", ss.str().c_str());
      std::string cmd = "mkdir -p " + scriptpath;
      system(cmd.c_str());
      std::string scriptfile = scriptpath + script + ".sh";
      std::ofstream fout(scriptfile.c_str());
      fout << ss.str();
      fout.close();
      cmd = "qsub " + scriptfile;
      system(cmd.c_str());
      ss.str("");
      ss << "update " << g_table << " set time_process = current_timestamp where id = " << record.getInt("id") << ";";
      db.execute(ss.str().c_str());
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("Failed to access db for read: %s", e.what());
    return -1;
  }

  return 0;
}

