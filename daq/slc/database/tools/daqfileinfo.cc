#include <daq/slc/psql/PostgreSQLInterface.h>

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
  if (argc < 2 || strcmp(argv[1], "--help") == 0) {
    printf("%s : [--list|--help] [-h hostname] [-t runtype] [-e expnumber] [-r runnumber] [-f fileid] [-m number_of_file ] \n",
           argv[0]);
    return 1;
  }
  std::string hostname;
  std::string runtype;
  int expno = 0;
  int runno = 0;
  int max = 0;
  int fileid = -1;
  bool listmode = false;
  bool htmlmode = false;
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
    } else if (strcmp(argv[i], "-m") == 0) {
      i++;
      if (i < argc) max = atoi(argv[i]);
    } else if (strcmp(argv[i], "-f") == 0) {
      i++;
      if (i < argc) fileid = atoi(argv[i]);
    } else if (strcmp(argv[i], "--list") == 0) {
      listmode = true;
    } else if (strcmp(argv[i], "--html") == 0) {
      htmlmode = true;
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
     << "where runno > 0 ";
  //   << "where time_copy is null and time_close is not null ";
  if (hostname.size() > 0) ss << "and path_sroot like '" << hostname << ":_%%' ";
  if (runtype.size() > 0) ss << "and runtype = '" << runtype << "' ";
  if (expno > 0) ss << "and expno = " << expno << " ";
  if (runno > 0) ss << "and runno = " << runno << " ";
  if (fileid >= 0) ss << "and fileid = " << fileid << " ";
  ss << " order by id desc";
  if (max > 0) ss << " limit " << max << " ";
  try {
    db.connect();
    db.execute(ss.str().c_str());
    DBRecordList record_v(db.loadRecords());
    db.close();
    if (htmlmode) {
      printf("<table>\n");
      printf("<tbody>\n");
    }
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      if (!listmode) {
        if (!htmlmode) {
          std::cout << std::endl;
          std::cout << "# " << std::endl;
          std::cout << "# ----------------------------------------------" << std::endl;
          std::cout << "# " << std::endl;
          std::cout << std::endl;
          std::cout << "# " << std::endl;
          std::cout << "# File infomation" << std::endl;
          std::cout << "# " << std::endl;
          std::cout << std::endl;
          std::cout << "File ID            : " << record.get("id") << std::endl;
          std::cout << "Sroot file   : " << record.get("path_sroot") << std::endl;
          std::cout << "Root file   : " << record.get("root") << std::endl;
          std::cout << std::endl;
          std::cout << "# " << std::endl;
          std::cout << "# Run information" << std::endl;
          std::cout << "# " << std::endl;
          std::cout << std::endl;
          std::cout << "Run type            : " << record.get("runtype") << std::endl;
          std::cout << "Experiment number   : " << record.get("expno") << std::endl;
          std::cout << "Run number          : " << record.get("runno") << std::endl;
          std::cout << std::endl;
          std::cout << "# " << std::endl;
          std::cout << "# Belle2 lib revisons" << std::endl;
          std::cout << "# " << std::endl;
          std::cout << "Reviosn             : " << record.get("rev") << std::endl;
          std::cout << "Revision (daq)      : " << record.get("rev_daq") << std::endl;
          std::cout << "Revision (rawdata)  : " << record.get("rev_rawdata") << std::endl;
          std::cout << "Revision (svd)      : " << record.get("rev_svd") << std::endl;
          std::cout << "Revision (pxd)      : " << record.get("rev_pxd") << std::endl;
          std::cout << std::endl;
          std::cout << "# " << std::endl;
          std::cout << "# Access dates" << std::endl;
          std::cout << "# " << std::endl;
          std::cout << std::endl;
          std::cout << "File created         : " << record.get("time_create") << std::endl;
          std::cout << "File closed          : " << record.get("time_close") << std::endl;
          std::cout << "Run end              : " << record.get("time_runend") << std::endl;
          std::cout << "Process started      : " << record.get("time_process") << std::endl;
          std::cout << "Process end          : " << record.get("time_convert") << std::endl;
          std::cout << std::endl;
          std::cout << "# " << std::endl;
          std::cout << "# ----------------------------------------------" << std::endl;
          std::cout << "# " << std::endl;
          std::cout << std::endl;
        } else {
          std::cout << "<tr><td>File infomation</td></tr>" << std::endl;
          std::cout << "<tr><td>File ID</td><td>" << record.get("id") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Sroot file</td><td>" << record.get("path_sroot") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Root file</td><td>" << record.get("root") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Run information" << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Run type</td><td>" << record.get("runtype") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Experiment number</td><td>" << record.get("expno") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Run number</td><td>" << record.get("runno") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Belle2 lib revisons" << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Reviosn</td><td>" << record.get("rev") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Revision (daq)</td><td>" << record.get("rev_daq") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Revision (rawdata)</td><td>" << record.get("rev_rawdata") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Revision (svd)</td><td>" << record.get("rev_svd") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Revision (pxd)</td><td>" << record.get("rev_pxd") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Access dates" << "</td></tr>" << std::endl;
          std::cout << "<tr><td>File created</td><td>" << record.get("time_create") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>File closed</td><td>" << record.get("time_close") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Run end</td><td>" << record.get("time_runend") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Process started</td><td>" << record.get("time_process") << "</td></tr>" << std::endl;
          std::cout << "<tr><td>Process end</td><td>" << record.get("time_convert") << "</td></tr>" << std::endl;
        }
      } else {
        std::cout << record.get("path_sroot") << std::endl;
      }
    }
    if (htmlmode) {
      printf("</tbody>\n");
      printf("</table>\n");
    }
  } catch (const DBHandlerException& e) {
    LogFile::error("Failed to access db for read: %s", e.what());
    return -1;
  }

  return 0;
}

