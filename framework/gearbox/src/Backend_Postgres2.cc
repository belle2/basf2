#include "framework/gearbox/Backend_Postgres2.h"

#include <boost/regex.hpp>

namespace Belle2 {
  namespace gearbox {

    Backend_Postgres2::Backend_Postgres2(const BackendConfigParamSet& params) : Backend(params)
    {

      this->DBConnection = new pqxx::connection(boost::get<std::string>(this->params["connectionString"]));

      pqxx::work readTransaction(*this->DBConnection);

      pqxx::result res = readTransaction.exec("SELECT * FROM " + boost::get<std::string>(this->params["meta_table"]));

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


    }

    Backend_Postgres2::~Backend_Postgres2()
    {
      delete this->DBConnection;
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


    std::string Backend_Postgres2::generateQueryRead1(std::string nodename, std::string conditions, queryParams_t /*queryParameters*/)
    {

      std::string tablename;
      std::string valuecolumn;

      std::string condString;

      if (nodename.length() == 0 or nodename == "*") {
        throw ("Backend_Postgres2::generateQueryRead1: Use join mode via table alias for multi table access");
      }

      if (!(this->aliasTablename.find(nodename) == this->aliasTablename.end()))
        tablename = this->aliasTablename[nodename];
      else
        tablename = nodename;



      if (conditions.length() > 0) {
        //condString = "WHERE "+boost::replace_all(conditions,"@","");
      }

      return std::string("SELECT * FROM " + tablename + condString);

    }

    void Backend_Postgres2::query(const std::string& xPath, QryResultSet& /*connectNode*/, std::string /*internalUpToPath*/, GBResult& /*theGBResult*/)
    {

      // all previous xpath labels are removed by the mount code -> remaining label corresponds to
      // table name and acts as "key" for query modifications

      B2ERROR("Gearbox::Backend_Postgres2::query: '" + xPath + "'");

      std::string qry;
      boost::smatch extraction;

      std::string tables;
      std::string conditions;
      std::string columns;

      std::string nodename;
      std::string tablename;

      std::string valuecolumn;
      std::string uniqueIdentGroup;

      boost::regex expr("^/(.*(/@(.+))?)(\\[(.+)\\])?(\\{.*\\})?$");

      if (boost::regex_search(xPath, extraction, expr)) {

        auto queryParameters = this->parseQueryParams(extraction[6]);

        // TODO: Query Parameter parse

        // certain attribute requested?
        // -> conditions make sense regarding attribute name
        if (extraction[3].length() > 0) {

          // TODO: select certain attributes only

        } else {

          qry = this->generateQueryRead1(extraction[1], extraction[5], queryParameters);

        }

        nodename = extraction[1];
        //conditions = boost::replace_all(extraction[3],"@","");

      } else {
        throw (std::string("Backend_Postgres2: Could not parse XPath"));
      }



      pqxx::work readTransaction(*this->DBConnection, "readTransaction");

      // this query has to return the nodes in an order so that each node
      // is before its children. From the recursive query, I can't see how
      // a DB server could possibly return any other order.
      // Should this ever change in the future, the logic to read the result
      // has to be changed to handle nodes in any order.
      pqxx::result res = readTransaction.exec(qry);

      for (pqxx::result::const_iterator i = res.begin(); i != res.end(); ++i) {

        //res[i];

        //this->buildAndAddNode1(res[i],theGBResult);

      }


    }

    void Backend_Postgres2::buildAndAddNode1(pqxx::tuple /*row*/, GBResult& /*theGBResult*/)
    {

      std::string valuecolumn;
      std::string uniqueIdentGroup;
      std::string uniqueIdentColumn;

      /*
            if (!(this->aliasValuecolumn.find(nodename) == this->aliasValuecolumn.end()))
              valuecolumn = this->aliasValuecolumn[nodename];
            else
              valuecolumn = "value";

            if (!(this->uniqueIdentGroup.find(nodename) == this->uniqueIdentGroup.end()))
              uniqueIdentGroup = this->uniqueIdentGroup[nodename];
            else
              uniqueIdentGroup = nodename;

            if (!(this->uniqueIdentColumn.find(nodename) == this->uniqueIdentColumn.end()))
              uniqueIdentColumn = this->uniqueIdentColumn[nodename];
            else
              uniqueIdentColumn = "id";

      */

    }

    bool Backend_Postgres2::checkPathExists(std::string /*path*/)
    {

      return true;

    }

  } // namespace gearbox
} // namespace Belle2
