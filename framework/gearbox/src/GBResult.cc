#include <boost/algorithm/string/find.hpp>

#include "framework/gearbox/GBResult.h"
#include "framework/logging/Logger.h"


namespace Belle2 {
  namespace gearbox {

    GBResult::GBResult() : resultSet(new QryResultSet(QryResultNode(nodeId_t(genRandomString(16)), nodeValue_t())))
    {

    }

    GBResult::~GBResult()
    {

    }

    boost::shared_ptr<QryResultSet> GBResult::getResultSetPtr()
    {
      return this->resultSet;
    }

    void GBResult::submitNodeVariant(QryResultSet& parent, nodeExistenceState existState, const std::string label, const nodeValue_t value, nodeId_t uniqueId, nodeId_t refNodeId, bool isAttrib)
    {

      std::map<nodeId_t, QryResultSet*>::iterator idIt = this->nodeIdMap.end();

      if (existState == nodeExistenceState::add)
        if ((idIt = this->nodeIdMap.find(refNodeId)) != this->nodeIdMap.end()) {
          existState = nodeExistenceState::modify;
          refNodeId = uniqueId;
        }

      switch (existState) {

        case nodeExistenceState::add:

          this->createNodeVariant(parent, std::move(label), value, uniqueId, isAttrib);

          break;
        case nodeExistenceState::modify:

          if (idIt != this->nodeIdMap.end() || (idIt = this->nodeIdMap.find(refNodeId)) != this->nodeIdMap.end())
            idIt->second->put_value(QryResultNode(refNodeId, value));

          break;
        case nodeExistenceState::remove:

          // TODO: make this more efficient
          // unfortunately, we have to access the node, that is to be deleted, through the parent,
          // because erasing as node with boost::property_tree requires an iterator
          for (auto childIt = parent.begin(); childIt != parent.end();  ++childIt) {

            if (getUniqueNodeId(childIt->second) == refNodeId) {
              parent.erase(childIt);
              break;
            }

          }

          break;
        case nodeExistenceState::remove_all:

          parent.erase(label);

          break;

        default: throw (std::string("Invalid node existence state!"));

      }


    }

    QryResultSet& GBResult::createNodeVariant(QryResultSet& parent, std::string label, nodeValue_t value, nodeId_t uniqueId = genRandomString(16), bool isAttrib = false)
    {


      if (isAttrib)
        label.insert(0, "@");

      this->nodeIdMap[uniqueId] = &parent.add_child(std::move(label),
                                                    QryResultSet(QryResultNode(nodeId_t(uniqueId), nodeValue_t(value)))
                                                   );

      /*
      QryResultSet::const_iterator iter =  parent.push_back(
          std::pair<std::string,QryResultSet>(
                    std::move(label),
                    QryResultSet(QryResultNode(nodeId_t(uniqueId),nodeValue_t(value)))
            )
      );
      this->nodeIdMap[uniqueId] = const_cast<QryResultSet*>(&iter->second);
      */

      return *this->nodeIdMap[uniqueId];
    }


    QryResultSet& GBResult::createEmptyNode(QryResultSet& parent, std::string label, nodeId_t uniqueId)
    {
      return this->createNodeVariant(parent, std::move(label), nodeValue_t(0), std::move(uniqueId), false);
    }

    QryResultSet& GBResult::getNodeByPath(std::string path)
    {
      return this->resultSet->get_child(QryResultSet::path_type(path, '/'));
    }

    QryResultSet& GBResult::getNodeByUniqueId(std::string nodeId)
    {

      std::map<nodeId_t, QryResultSet*>::iterator idIt = this->nodeIdMap.end();

      if ((idIt = this->nodeIdMap.find(nodeId)) == this->nodeIdMap.end())
        throw NodeNotFoundException();

      return *(idIt->second);
    }

    QryResultSet& GBResult::ensurePath(std::string path)
    {

      this->makeNodePath(path);

      QryResultSet* nodeAtPath;

      try {

        nodeAtPath = &this->getNodeByPath(path);

        // if path not found, create the last label after ensure, that its parent exists
      } catch (const boost::property_tree::ptree_error&) {

        boost::iterator_range<std::string::iterator> newPath = boost::find_last(path, "/");

        QryResultSet& parent = this->ensurePath(std::string(newPath.begin(), newPath.end()));

        std::string childLabel;

        if (newPath.end() == path.end())
          childLabel = std::string(path.begin(), path.end());
        else
          childLabel = std::string(newPath.end(), path.end());

        nodeAtPath = &this->createEmptyNode(parent, childLabel);

      }

      return *(nodeAtPath);

    }

    QryResultSet::size_type GBResult::erasePath(std::string path)
    {
      this->makeNodePath(path);
      return this->resultSet->erase(path);
    }

    void GBResult::makeNodePath(std::string& path)
    {

      if (path.length() > 0 && path[0] == '/') path.erase(0, 1);

      if (path.length() > 0 && path[path.length() - 1] == '/') path.pop_back();

    }

    std::string getUniqueNodeId(QryResultSet& res)
    {
      auto ret = res.get_value_optional<std::string>(UniqueIdTranslator());
      if (!ret)throw("Could not read node unique ID");
      return ret.get_value_or("");
    }

    void printResultSet(QryResultSet& res, const std::string& prefix, bool header)
    {

      if (header) {
        B2INFO("                                                         ")
        B2INFO("=========================================================")
        B2INFO("Printing boost::property_tree using the following format:");
        B2INFO("Node: '<path>' : '<value>' (<uniqueNodeID>)");
        B2INFO("=========================================================");

        // The Result-Component Root-Node
        B2INFO("Node: '' : '" + res.get_value<std::string>("<unspecified>") + "': (" + getUniqueNodeId(res) + ") [THE_RESULT_COMPONENT_ROOT_NODE]");
      }

      for (auto childIt = res.begin(); childIt != res.end();  ++childIt) {

        // a child of the node
        B2INFO("Node: '" + prefix + childIt->first + "' : '" + childIt->second.get_value<std::string>("<unspecified>") + "' (" + getUniqueNodeId(childIt->second) + ")");

        // children of this child node
        printResultSet(childIt->second, prefix + childIt->first + ".", false);

      }

    }

  } /* namespace gearbox */
} /* namespace Belle2 */
