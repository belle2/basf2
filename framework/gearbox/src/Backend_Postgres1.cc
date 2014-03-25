#include "framework/gearbox/Backend_Postgres1.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

namespace Belle2 {
  namespace gearbox {

    Backend_Postgres1::Backend_Postgres1(const BackendConfigParamSet& params) : Backend(params)
    {

      this->DBConnection = new pqxx::connection(boost::get<std::string>(this->params["connectionString"]));

      pqxx::work readTransaction(*this->DBConnection);

      pqxx::result res = readTransaction.exec("SELECT * FROM config_t");

      for (pqxx::result::size_type i = 0; i != res.size(); ++i) {
        if (!strcmp(res[i]["param"].c_str(), "uniqueHash")) {
          res[i]["val"].to(this->uniqueHash);
        }
        // other config parameters may go here
      }

      readTransaction.commit();

      if (this->uniqueHash.empty()) {
        pqxx::work writeTransaction(*this->DBConnection);
        writeTransaction.exec("INSERT INTO config_t (param, val) VALUES ('uniqueHash','" + (this->uniqueHash = genRandomString(GEARBOX_BACKEND_UNIQUEHASH_AUTOLENGTH)) + "')");
        writeTransaction.commit();
      }

    }

    Backend_Postgres1::~Backend_Postgres1()
    {
      delete this->DBConnection;
    }

    // TODO:: merge write support branch
    void Backend_Postgres1::writeCommit(std::string comment)
    {

      //TODO: reuse DB transaction here
      pqxx::work writeTransaction(*this->DBConnection, "writeTransaction");

      pqxx::result res =  writeTransaction.exec("INSERT INTO commit_t(comment, branchid) VALUES(" + comment + ", 1) RETURNING id");

      uint64_t commitID;
      res[0][0].to(commitID);

      // TODO: Iterate Tree!

      res = writeTransaction.exec("INSERT INTO node_t(name, commitid) VALUES($1, " + std::to_string(commitID) + ") RETURNING id");

      uint64_t rootNodeID;
      res[0][0].to(rootNodeID);

      writeTransaction.exec("UPDATE commit_t SET rootnodeid=$1 WHERE id=$2");


      writeTransaction.commit();

    }


    void Backend_Postgres1::processParams()
    {


    }

    std::string Backend_Postgres1::convertXPath2LQuery(std::string xPath)
    {

      // magic :-)
      boost::replace_all(xPath, "//*", ".*");

      boost::replace_all(xPath, "/*", "/*{1}");

      boost::replace_all(xPath, "//", "/*/");

      // TODO: clean attribute handling
      boost::replace_all(xPath, "/@", "/");

      boost::replace_all(xPath, "/", ".");

      // remove first char (not needed as separator)
      xPath.erase(0, 1);

      return xPath;

    }

    void Backend_Postgres1::query(const std::string& xPath, QryResultSet& connectNode, std::string internalUpToPath, GBResult& theGBResult)
    {

      std::vector<std::string> connectPathVec;

      // TODO: deliver map as argument -> get rid of annoying separator character
      if (internalUpToPath != "/" && internalUpToPath != "")
        boost::algorithm::split(connectPathVec, internalUpToPath, boost::is_any_of("/"));


      boost::smatch extraction;
      boost::regex expr("^([^\\[]+)(\\[.*\\])?(\\{.*\\})?$");
      boost::regex_search(xPath, extraction, expr);

      auto queryParams = this->parseQueryParams(extraction[3]);


      std::string lQuery(this->convertXPath2LQuery(extraction[1]));

      B2INFO("Gearbox::Backend_Postgres1::query: '" + xPath + "'using lquery '" + lQuery + "'");

      pqxx::work readTransaction(*this->DBConnection, "readTransaction");

      pqxx::result res;

      // general requirement for queries - return parent
      // nodes before their children

      if (false) {

        // Michael Ritzert's recursive query

        // this query has to return the nodes in an order so that each node
        // is before its children. From the recursive query, I can't see how
        // a DB server could possibly return any other order.
        // Should this ever change in the future, the logic to read the result
        // has to be changed to handle nodes in any order.
        res = readTransaction.exec("WITH RECURSIVE root(id, name, parent, float_value, int_value, text_value) AS ("
                                   "SELECT id, name, NULL::BIGINT, float_value, int_value, text_value FROM node_t WHERE id=" + std::to_string(1) + " "
                                   "UNION "
                                   "SELECT c.id, c.name, t.nodeid_parent, c.float_value, c.int_value, c.text_value "
                                   "FROM link_t t, node_t c, root r "
                                   "WHERE t.nodeid_parent = r.id AND t.nodeid = c.id "
                                   ") SELECT root.id, root.name, parent, float_value, int_value, text_value, attrib_t.name AS attrib_name, attrib_t.value AS attrib_value FROM root "
                                   "LEFT JOIN attrib_t on attrib_t.nodeid = root.id");

      } else {

        res =  readTransaction.exec("SELECT lkt.path, lkt.nodeid_path, lkt.nodeid,"
                                    "ndt.name, ndt.is_attrib, ndt.value_float, ndt.value_int, ndt.value_string, ndt.valuetype "
                                    "FROM lookup_t lkt LEFT OUTER JOIN node_t ndt ON (lkt.nodeid = ndt.id) "
                                    "WHERE path ~ '" + lQuery + "' ORDER BY path ASC");

      }


      nodeExistenceState existState;

      std::string nodeIDPathList;
      std::vector<std::string> nodeIDPathVec;

      std::string nodePathList;
      std::vector<std::string> nodePathVec;

      for (pqxx::result::const_iterator i = res.begin(); i != res.end(); ++i) {

        // TODO: do we want existstates other than "add" in Postgres1 backend?
        existState = nodeExistenceState::add;

        // contains ID-Path of node
        (*i)["nodeid_path"].to(nodeIDPathList);
        nodeIDPathList.pop_back(); // remove beginning { from pgsql array
        nodeIDPathList.erase(0, 1); // remove ending } from pgsql array
        boost::split(nodeIDPathVec, nodeIDPathList, boost::is_any_of(","));

        // contains path of node
        (*i)["path"].to(nodePathList);
        boost::split(nodePathVec, nodePathList, boost::is_any_of("."));

        // ensure parents only
        nodeIDPathVec.pop_back();
        nodePathVec.pop_back();
        QryResultSet& parent = this->ensurePath(nodePathVec, nodeIDPathVec, connectPathVec, connectNode, theGBResult);

        B2DEBUG(4, "Gearbox::Backend_Postgres1::query: processing '" + (*i)["path"].as<std::string>() + "', id '" + this->uniqueHash + (*i)["nodeid"].as<std::string>() + "', parentID '" + getUniqueNodeId(parent) + "', value '" + (*i)["value_" + (*i)["valuetype"].as<std::string>()].as<std::string>() + "'");

        theGBResult.submitNode(parent,
                               existState,
                               (*i)["name"].as<std::string>(),
                               (*i)["value_" + (*i)["valuetype"].as<std::string>()].as<std::string>(),
                               this->uniqueHash + (*i)["nodeid"].as<std::string>(),
                               (existState == nodeExistenceState::modify || existState == nodeExistenceState::remove) ? (*i)["refNodeId"].as<std::string>() : "",
                               (*i)["is_attrib"].as<bool>()
                              );

      }

    }


    QryResultSet& Backend_Postgres1::ensurePath(std::vector<std::string>& nodePathVec, std::vector<std::string>& nodeIDPathVec, const std::vector<std::string>& connectPathVec, QryResultSet& upperNode, GBResult& theGBResult)
    {

      QryResultSet* nodeAtPath;

      // connect node is found
      if (nodePathVec == connectPathVec) {
        return upperNode;
      }

      // get object of last node of path, that is to be ensured
      try {

        nodeAtPath = &theGBResult.getNodeByUniqueId(this->uniqueHash + nodeIDPathVec.back());

        // if last not of path not found, create it after ensure, that its parent exists
      } catch (GBResult::NodeNotFoundException&) {

        // Build Unique ID and Label for this child
        std::string childUniqueID(this->uniqueHash + nodeIDPathVec.back());
        std::string childLabel(nodePathVec.back());

        // prepare path vector and id vector for parent
        nodeIDPathVec.pop_back();
        nodePathVec.pop_back();

        QryResultSet& parent = this->ensurePath(nodePathVec, nodeIDPathVec, connectPathVec, upperNode, theGBResult);

        nodeAtPath = &theGBResult.createEmptyNode(parent, childLabel, childUniqueID);

      }

      return *(nodeAtPath);

    }



    bool Backend_Postgres1::checkPathExists(std::string path)
    {
      // TODO: merge write branch
      path = "foo";
      return true;
    }

  } // namespace gearbox
} // namespace Belle2
