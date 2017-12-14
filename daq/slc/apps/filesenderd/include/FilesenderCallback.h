#ifndef _Belle2_FilesenderCallback_h
#define _Belle2_FilesenderCallback_h

#include "daq/slc/nsm/NSMCallback.h"

#include "daq/slc/database/DBInterface.h"

#include <map>

namespace Belle2 {

  class FilesenderCallback : public NSMCallback {

  public:
    FilesenderCallback(const std::string& nodename, int expno, int runno,
                       const std::string& path_ipnp, const std::string& path_pxd,
                       const std::string& dirs, const std::string& excludes,
                       DBInterface& db);
    virtual ~FilesenderCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  private:
    DBInterface& m_db;
    std::vector<std::string> m_dirs;
    std::vector<std::string> m_excludes;
    int m_expno;
    int m_runno;
    std::string m_path_ipnp;
    std::string m_path_pxd;
    std::map<std::string, int> m_files;

  };

}

#endif
