#include "framework/gearbox/Backend_Postgres2.h"
#include <framework/gearbox/GBResult.h>
#include <framework/logging/Logger.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

namespace Belle2 {
  namespace gearbox {

    Backend_Postgres2::Backend_Postgres2(const BackendConfigParamSet& params) : Backend(params)
    {

      this->DBConnection = new pqxx::connection(boost::get<std::string>(this->params["connectionString"]));

      this->processParams();

      pqxx::read_transaction ta(*this->DBConnection);

      pqxx::result res = ta.exec("SELECT * FROM " + boost::get<std::string>(this->params["meta_table"]));
      ta.commit();

      for (pqxx::result::size_type i = 0; i != res.size(); ++i) {

        if (!strcmp(res[i]["prop_type"].c_str(), "unique_backend_hash")) {
          res[i]["value"].to(this->uniqueHash);
        }

        if (!strcmp(res[i]["prop_type"].c_str(), "alias_tablename")) {
          this->aliasTablename[res[i]["key"].as<std::string>()] = res[i]["value"].as<std::string>();
        }

        if (!strcmp(res[i]["prop_type"].c_str(), "alias_valuecolumn")) {
          this->aliasValuecolumn[res[i]["key"].as<std::string>()] = res[i]["value"].as<std::string>();
        }

        if (!strcmp(res[i]["prop_type"].c_str(), "unique_ident_group")) {
          this->uniqueIdentGroup[res[i]["key"].as<std::string>()] = res[i]["value"].as<std::string>();
        }

        if (!strcmp(res[i]["prop_type"].c_str(), "unique_ident_column")) {
          this->uniqueIdentColumn[res[i]["key"].as<std::string>()] = res[i]["value"].as<std::string>();
        }


      }

      if (this->uniqueHash.empty()) {

        this->uniqueHash = genRandomString(GEARBOX_BACKEND_UNIQUEHASH_AUTOLENGTH);

        pqxx::work wta(*this->DBConnection);
        wta.exec("INSERT INTO " + boost::get<std::string>(this->params["meta_table"]) + " (prop_type, key, value) VALUES ('unique_backend_hash','','" + this->uniqueHash + "')");
        wta.commit();

      }


    }

    Backend_Postgres2::~Backend_Postgres2()
    {
      delete this->DBConnection;
    }



    void Backend_Postgres2::processParams()
    {

      {

        if (this->params.find("schema") == this->params.end()) {
          this->params.insert(std::make_pair("schema", BackendConfigParam("public")));
        }

        pqxx::read_transaction ta(*this->DBConnection);
        pqxx::result res = ta.exec("SELECT schema_name FROM information_schema.schemata WHERE schema_name = '" + boost::get<std::string>(this->params["schema"]) + "'");
        ta.commit();

        if (res.size() < 1) {
          throw (std::string("Gearbox::Backend_Postgres2: schema '" + boost::get<std::string>(this->params["schema"]) + "' not found in database"));
        }

      }

      this->DBConnection->set_variable("search_path", boost::get<std::string>(this->params["schema"]));

      {

        if (this->params.find("meta_table") == this->params.end()) {
          this->params.insert(std::make_pair("meta_table", BackendConfigParam("config_t")));
        }

        pqxx::read_transaction ta(*this->DBConnection);
        pqxx::result res = ta.exec("SELECT * FROM information_schema.tables WHERE table_schema = current_schema() and table_name = '" + boost::get<std::string>(this->params["meta_table"]) + "'");
        ta.commit();

        if (res.size() < 1) {
          throw (std::string("Gearbox::Backend_Postgres2: backend configuration table '" + boost::get<std::string>(this->params["meta_table"]) + "' not found in database"));
        }

      }


    }


    // TODO: merge write branch
    void Backend_Postgres2::writeCommit(std::string comment)
    {

      pqxx::work writeTransaction(*this->DBConnection, "writeTransaction");

      pqxx::result res = writeTransaction.exec("INSERT INTO commit_t(comment, branchid) VALUES(" + comment + ", 1) RETURNING id");

      uint64_t commitID;
      res[0][0].to(commitID);

      // TODO: Iterate Tree!

      res = writeTransaction.exec("INSERT INTO node_t(name, commitid) VALUES($1, " + std::to_string(commitID) + ") RETURNING id");

      uint64_t rootNodeID;
      res[0][0].to(rootNodeID);

      writeTransaction.exec("UPDATE commit_t SET rootnodeid=$1 WHERE id=$2");


      writeTransaction.commit();

    }

    void Backend_Postgres2::query(const std::string& xPath, QryResultSet& connectNode, std::string /*internalUpToPath*/, GBResult& theGBResult)
    {

      // all previous xpath labels are removed by the mount code -> remaining label corresponds to
      // table name and acts as "key" for query modifications

      B2INFO("Gearbox::Backend_Postgres2::query: '" + xPath + "'");

      boost::regex expr("^/([^/\\[\\{]*)(/@(.+))?(\\[(.+)\\])?(\\{(.+)\\})?$");
      boost::smatch extraction;
      /*
       *  e.g. "/node/@attrib[conditions]{queryParams}" results in
       *
       * [1] node
       * [2] /@attrib
       * [3] attrib
       * [4] [conditions]
       * [5] conditions
       * [6] {queryParams}
       * [7] queryParams
       *
       */

      if (!boost::regex_search(xPath, extraction, expr)) {
        throw (std::string("Gearbox::Backend_Postgres2::query: Could not parse XPath"));
      }


      std::string tables;
      std::string conditions;
      std::string columns;

      auto queryParameters = this->parseQueryParams(extraction[7]);

      // certain attribute requested?
      // -> conditions should refer to attribute name
      if (extraction[3].matched) {

        // TODO: select certain attributes only
        // qry = this->generateQueryRead2(extraction[1], extraction[5], queryParameters);

      } else {

        if (extraction[1].length() == 0 or extraction[1] == "*") {
          throw ("Gearbox::Backend_Postgres2::query: Use join mode via table alias for multi table access");
        }

        // find out table names
        if (!(this->aliasTablename.find(extraction[1]) == this->aliasTablename.end()))
          tables = this->aliasTablename[extraction[1]];
        else
          tables = extraction[1];

        // for now, we select node and all attributes => means all columns
        columns = "*";

        if (extraction[5].matched) {
          conditions = " WHERE " + extraction[5];
          // keep it simple -> better converter needed here for special xpath functions
          boost::replace_all(conditions, "@", "");
        }


      }


      pqxx::work readTransaction(*this->DBConnection, "readTransaction");

      pqxx::result res = readTransaction.exec("SELECT " + columns + " FROM " + tables + conditions);

      std::string valuecolumn;
      if (!(this->aliasValuecolumn.find(extraction[1]) == this->aliasValuecolumn.end()))
        valuecolumn = this->aliasValuecolumn[extraction[1]];
      else
        valuecolumn = "value";


      std::string uniqueID_base(this->uniqueHash);
      if (!(this->uniqueIdentGroup.find(extraction[1]) == this->uniqueIdentGroup.end()))
        uniqueID_base.append(this->uniqueIdentGroup[extraction[1]]);
      else
        uniqueID_base.append(extraction[1]);


      std::string uniqueID_col("id");
      if (!(this->uniqueIdentColumn.find(extraction[1]) == this->uniqueIdentColumn.end()))
        uniqueID_col = this->uniqueIdentColumn[extraction[1]];

      bool isAttrib;


      for (auto rowIt = res.begin(); rowIt != res.end(); ++rowIt) {

        // certain attribute requested?
        if (extraction[3].matched) {

          // res[i];
          /*
                   theGBResult.submitNode(connectNode,
                                         nodeExistenceState::add,
                                         extraction[3],
                                         fieldIt->as<std::string>(),
                                         this->getNodeId(rowIt,fielIt),rrt
                                         "",
                                         true
                                        );

                                    }
          */


        } else {

          QryResultSet& thisnode = theGBResult.createEmptyNode(connectNode, extraction[1] , uniqueID_base + rowIt[uniqueID_col].as<std::string>());

          for (auto fieldIt = rowIt->begin(); fieldIt != rowIt.end(); ++fieldIt) {

            isAttrib = (std::strcmp(valuecolumn.c_str(), fieldIt->name()) ? true : false);

            theGBResult.submitNode((isAttrib ? thisnode : connectNode),
                                   nodeExistenceState::add,
                                   std::string(fieldIt->name()),
                                   (fieldIt->is_null() ? "<null>" : fieldIt->as<std::string>()),
                                   uniqueID_base + rowIt[uniqueID_col].as<std::string>() + (isAttrib ? "_" + std::string(fieldIt->name()) : ""),
                                   "",
                                   isAttrib);

          }


        }


      }


    }

    bool Backend_Postgres2::checkPathExists(std::string /*path*/)
    {

      return true;

    }

  } // namespace gearbox
} // namespace Belle2
