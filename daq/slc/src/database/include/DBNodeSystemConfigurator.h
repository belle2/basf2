#ifndef _Belle2_DBNodeSystemConfigurator_hh
#define _Belle2_DBNodeSystemConfigurator_hh

#include "DBInterface.h"

#include "base/NodeSystem.h"

namespace Belle2 {

  class DBNodeSystemConfigurator {

  public:
    DBNodeSystemConfigurator(DBInterface* db = NULL,
                             NodeSystem* node_system = NULL) throw()
      : _db(db), _system(node_system) {}
    virtual ~DBNodeSystemConfigurator() throw() {}

  public:
    DBInterface* getDB() { return _db; }
    NodeSystem* getNodeSystem() { return _system; }
    void createTables() throw(DBHandlerException);
    DBRecord readTables(int version) throw(DBHandlerException);
    void writeTables() throw(DBHandlerException);
    virtual void createVersionControlTable() throw(DBHandlerException);
    virtual void createHostTable() throw(DBHandlerException);
    virtual void createCOPPERNodeTable() throw(DBHandlerException);
    virtual void createHSLBTable() throw(DBHandlerException);
    virtual void createFEEModuleTable(const std::string& module_class,
                                      std::vector<FEEModule*>& module_v) throw(DBHandlerException);
    virtual void createTTDNodeTable() throw(DBHandlerException);
    virtual void createFTSWTable() throw(DBHandlerException);
    virtual void createRONodeTable() throw(DBHandlerException);
    virtual DBRecord readVersionControlTable(int version) throw(DBHandlerException);
    virtual void readHostTable(int version) throw(DBHandlerException);
    virtual void readCOPPERNodeTable(int version) throw(DBHandlerException);
    virtual void readHSLBTable(int version) throw(DBHandlerException);
    virtual void readFEEModuleTable(const std::string& module_class,
                                    std::vector<FEEModule*>& module_v,
                                    int version) throw(DBHandlerException);
    virtual void readTTDNodeTable(int version) throw(DBHandlerException);
    virtual void readFTSWTable(int version) throw(DBHandlerException);
    virtual void readRONodeTable(int version) throw(DBHandlerException);
    virtual void writeVersionControlTable() throw(DBHandlerException);
    virtual void writeHostTable(int version) throw(DBHandlerException);
    virtual void writeCOPPERNodeTable(int version) throw(DBHandlerException);
    virtual void writeHSLBTable(int version) throw(DBHandlerException);
    virtual void writeFEEModuleTable(const std::string& module_class,
                                     std::vector<FEEModule*>& module_v,
                                     int version) throw(DBHandlerException);
    virtual void writeTTDNodeTable(int version) throw(DBHandlerException);
    virtual void writeFTSWTable(int version) throw(DBHandlerException);
    virtual void writeRONodeTable(int version) throw(DBHandlerException);

  protected:
    DBInterface* _db;
    NodeSystem* _system;

  };

}

#endif

