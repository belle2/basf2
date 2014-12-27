#include "framework/xmldb/db.h"
#include "framework/xmldb/db_format.h"
#include "framework/xmldb/dbfactory.h"
#include "framework/xmldb/reader_db.h"
#include "framework/xmldb/leaf_cdata.h"
#include "framework/xmldb/leaf_double.h"
#include "framework/xmldb/leaf_int.h"
#include "framework/xmldb/leaf_string.h"
#include "framework/xmldb/tree.h"
#include "framework/xmldb/connection.h"
#include "framework/xmldb/query.h"
#include "framework/xmldb/query_builder.h"

#include <bzlib.h>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>

namespace Belle2 {
  namespace xmldb {

    Reader_DB::Reader_DB() : work_tree_(nullptr)
    {
    } // Reader_DB constructor

    Tree* Reader_DB::read(const std::string& _file)
    {
      // split at the first ';'
      auto split = _file.find(';');
      std::string filename = _file;
      std::string branchname = "trunk";
      if (split != std::string::npos) {
        filename = _file.substr(0, split);
        branchname = _file.substr(split + 1);
      }

      // search for the file
      auto conn = DBFactory::connect();
      auto filelist = DB::listFiles(conn.get(), filename);
      if (filelist.size() > 1) {
        std::cerr << "ambiguous file name" << std::endl;
        return nullptr;
      } else if (filelist.empty()) {
        std::cerr << "no matching file found" << std::endl;
        return nullptr;
      }

      // search for the branch
      auto branchlist = DB::find(conn.get(), filelist.begin()->second.id,
                                 branchname);
      if (branchlist.empty()) {
        std::cerr << "no matching branch found" << std::endl;
        return nullptr;
      }
      // the returned list is ordered so that the latest commit is first.
      return read(branchlist.begin()->second.id);
    } // Reader_DB::read

    Tree* Reader_DB::read(int64_t _commitid)
    {
      Belle2::xmldb::QueryBuilder g;
      g.setStatement("SELECT comment, datetime, rootnodeid, branchid, "
                     "b.name AS branchname, f.id AS fileid, f.name AS filename "
                     "FROM commit_t c, branch_t b, file_t f "
                     "WHERE c.id=$1 AND b.id = c.branchid AND b.fileid = f.id",
                     1 /* # parameters */);
      g.setParameterInt(1, _commitid);
      Belle2::xmldb::Query q(DBFactory::connect().get());
      if (!q.executeGenerated(g)) {
        std::cout << "query failed" << std::endl;
        return NULL;
      } // if
      if (q.numRows() == 0) {
        return NULL;
      } // if
      const int comment_index = q.columnIndex("comment");
      const int created_index = q.columnIndex("datetime");
      const int rootnodeid_index = q.columnIndex("rootnodeid");
      const int branchid_index = q.columnIndex("branchid");
      const int branchname_index = q.columnIndex("branchname");
      const int fileid_index = q.columnIndex("fileid");
      const int filename_index = q.columnIndex("filename");
      work_tree_ = new Tree;
      work_tree_->id_.id = _commitid;
      work_tree_->id_.comment = q.getValueAsString(0, comment_index);
      work_tree_->id_.creation_date = q.getValueAsTimestamp(0, created_index);
      work_tree_->id_.branch.id = q.getValueAsInt(0, branchid_index);
      work_tree_->id_.branch.name = q.getValueAsString(0, branchname_index);
      work_tree_->id_.branch.file.id = q.getValueAsInt(0, fileid_index);
      work_tree_->id_.branch.file.name = q.getValueAsString(0, filename_index);
      int64_t rootnode_id = q.getValueAsInt(0, rootnodeid_index);

      std::unique_ptr<Belle2::xmldb::Query> q2(queryTree(rootnode_id));
      if (!q2) {
        return nullptr;
      }
      int rows = q2->numRows();
      // there is a REFERENCES constraint between the tables, so this query
      // has to return at least one row, if all is well.
      assert(rows > 0);
      work_tree_->doc_ = ::xmlNewDoc(reinterpret_cast<const ::xmlChar*>("1.0"));
      if (!work_tree_->doc_) {
        return nullptr;
      }

      for (int row = 0; row < rows; ++row) {
        buildNode(q2.get(), row);
      }
      return work_tree_;
    } // Reader_DB::read

    Belle2::xmldb::Query* Reader_DB::queryTree(int _rootnodeid)
    {
      Belle2::xmldb::QueryBuilder g;
      // this query has to return the nodes in an order so that each node
      // is before its children. From the recursive query, I can't see how
      // a DB server could possibly return any other order.
      // Should this ever change in the future, the logic to read the result
      // has to be changed to handle nodes in any order.
      g.setStatement("WITH RECURSIVE root(id, name, parent, linktype, float_value, int_value, text_value, cdata_id, cdata_size, commitid, level) AS ("
                     "SELECT id, name, NULL::BIGINT, NULL::INT, float_value, int_value, text_value, cdata_id, cdata_size, commitid, 1 AS level FROM node_t WHERE id=$1 "
                     "UNION "
                     "SELECT c.id, c.name, t.nodeid_parent, t.type, c.float_value, c.int_value, c.text_value, c.cdata_id, c.cdata_size, c.commitid, r.level+1 "
                     "FROM link_t t, node_t c, root r "
                     "WHERE t.nodeid_parent = r.id AND t.nodeid = c.id "
                     ") SELECT root.id, root.name, parent, linktype, float_value, int_value, text_value, cdata_id, cdata_size, attrib_t.name AS attrib_name, attrib_t.value AS attrib_value, commitid FROM root "
                     "LEFT JOIN attrib_t on attrib_t.nodeid = root.id "
                     "ORDER BY level",
                     1 /* # parameters */);
      g.setParameterInt(1, _rootnodeid);
      Belle2::xmldb::Query* q = new Belle2::xmldb::Query(DBFactory::connect().get());
      if (!q->executeGenerated(g)) {
        std::cout << "query failed" << std::endl;
        delete q;
        return nullptr;
      } // if

      id_index_ = q->columnIndex("id");
      name_index_ = q->columnIndex("name");
      parent_index_ = q->columnIndex("parent");
      linktype_index_ = q->columnIndex("linktype");
      float_value_index_ = q->columnIndex("float_value");
      int_value_index_ = q->columnIndex("int_value");
      text_value_index_ = q->columnIndex("text_value");
      attrib_name_index_ = q->columnIndex("attrib_name");
      attrib_value_index_ = q->columnIndex("attrib_value");
      commitid_index_ = q->columnIndex("commitid");
      cdata_id_index_ = q->columnIndex("cdata_id");
      cdata_size_index_ = q->columnIndex("cdata_size");

      return q;
    }

    ::xmlNodePtr Reader_DB::buildNode(Belle2::xmldb::Query* _q, int64_t _row)
    {
      const int64_t id = _q->getValueAsInt(_row, id_index_);
      bool attrib_name_null;
      const std::string attrib_name = _q->getValueAsString(_row,
                                                           attrib_name_index_,
                                                           &attrib_name_null);
      id_to_node_map_t::iterator i = id_to_node_map_.find(id);
      ::xmlNodePtr node;
      // nodes are returned multiple times if they have multiple attributes.
      if (i == id_to_node_map_.end()) {
        // new node
        const std::string name = _q->getValueAsString(_row, name_index_);

        int is_leaf = 0;
        bool float_value_null;
        const double float_value = _q->getValueAsFloat(_row,
                                                       float_value_index_,
                                                       &float_value_null);
        if (!float_value_null) ++is_leaf;
        bool int_value_null;
        const int64_t int_value = _q->getValueAsInt(_row,
                                                    int_value_index_,
                                                    &int_value_null);
        if (!int_value_null) ++is_leaf;
        bool text_value_null;
        const std::string text_value = _q->getValueAsString(_row,
                                                            text_value_index_,
                                                            &text_value_null);
        if (!text_value_null) ++is_leaf;
        bool cdata_id_null;
        int32_t cdata_id = _q->getValueAsInt(_row, cdata_id_index_,
                                             &cdata_id_null);
        if (!cdata_id_null) ++is_leaf;

        // only one data type may be stored in a node
        assert(is_leaf < 2);

        bool parent_null;
        const int64_t parentid = _q->getValueAsInt(_row, parent_index_,
                                                   &parent_null);

        ::xmlNodePtr parent = nullptr;
        if (!parent_null) {
          id_to_node_map_t::iterator j = id_to_node_map_.find(parentid);
          assert(j != id_to_node_map_.end());
          int32_t link_type = _q->getValueAsInt(_row, linktype_index_);
          if (link_type == (int)Node::LinkType::sibling) {
            parent = j->second->parent;
            assert(parent);
          } else if (link_type == (int)Node::LinkType::parent) {
            parent = j->second;
          } else {
            assert(false);
          }
        }

        if (is_leaf == 1) {
          assert(!parent_null);

          node = ::xmlNewTextChild(parent, NULL /* ns */,
                                   reinterpret_cast<const ::xmlChar*>(name.c_str()),
                                   reinterpret_cast<const ::xmlChar*>(""));
          if (!node) { /* TODO */; }
          work_tree_->getNodeData(parent)->num_children_++;
          Leaf* l = nullptr;

          if (!float_value_null) {
            l = new LeafDouble(node, work_tree_, float_value);
            l->updateXMLNode();
          } else if (!int_value_null) {
            l = new LeafInt(node, work_tree_, int_value);
            l->updateXMLNode();
          } else if (!text_value_null) {
            LeafString* ls = new LeafString(node, work_tree_);
            ls->setValue(text_value, false);
            l = ls;
          } else if (!cdata_id_null) {
            bool size_null;
            int64_t cs = _q->getValueAsInt(_row, cdata_size_index_,
                                           &size_null);
            assert(!size_null);
            LeafCdata* lc = new LeafCdata(node, work_tree_, cdata_id, cs);
            l = lc;
          }
          l->id_ = id;
          l->link_invalid = false;
          l->last_changed_commit_ = _q->getValueAsInt(_row, commitid_index_);
          work_tree_->attachNodeData(node, l);
        } else {
          if (parent_null) {
            // root comes first
            assert(!id_to_node_map_.size());
            // set as root node
            node = ::xmlNewNode(NULL /* ns */,
                                reinterpret_cast<const ::xmlChar*>(name.c_str()));
            // TODO: proper error handling
            assert(!::xmlDocSetRootElement(work_tree_->doc_, node));
            // TODO: move to tree:
            work_tree_->configNs_ = ::xmlNewNs(node,
                                               reinterpret_cast<const ::xmlChar*>("urn:X-belle2:database-control"),
                                               reinterpret_cast<const ::xmlChar*>("dbctrl"));
          } else {
            work_tree_->getNodeData(parent)->num_children_++;
            node = ::xmlNewChild(parent, NULL /* ns */,
                                 reinterpret_cast<const ::xmlChar*>(name.c_str()),
                                 NULL /* content */);
          }
          Node* n = new Node(node, work_tree_);

          n->id_ = id;
          n->link_invalid = false;
          n->last_changed_commit_ = _q->getValueAsInt(_row, commitid_index_);
          work_tree_->attachNodeData(node, n);
        } // else
        id_to_node_map_[id] = node;
      } else {
        // add attribute to already created node
        node = i->second;
        assert(!attrib_name_null);
      }

      if (!attrib_name_null) {
        const std::string attrib_value = _q->getValueAsString(
                                           _row,
                                           attrib_value_index_);
        assert(::xmlNewProp(node,
                            reinterpret_cast<const ::xmlChar*>(
                              attrib_name.c_str()),
                            reinterpret_cast<const ::xmlChar*>(
                              attrib_value.c_str())));
      }
      return node;
    }

    std::vector<uint8_t> Reader_DB::readCdata(int32_t _id, int64_t _size)
    {
      Belle2::xmldb::QueryBuilder g;
      g.setStatement("SELECT data, conversion FROM cdata_t "
                     "WHERE id=$1",
                     1 /* # parameters */);
      g.setParameterInt(1, _id);
      Belle2::xmldb::Query q(DBFactory::connect().get());
      if (!q.executeGenerated(g)) {
        std::cout << "query failed" << std::endl;
        return std::vector<uint8_t>();
      } // if
      assert(q.numRows() == 1);

      int conversion = q.getValueAsInt(0, q.columnIndex("conversion"));
      std::vector<uint8_t> data = q.getValueAsBlob(0, q.columnIndex("data"));

      // undoing the conversions as a chain. after each block, data must hold
      // the output of the conversions done so far.
      if (conversion == 0) {
        assert(data.size() == _size);
      }
      if (conversion & CONVERSION_BZIP2) {
        std::vector<uint8_t> decompressed(_size);
        unsigned destLength = _size;
        int result = BZ2_bzBuffToBuffDecompress((char*)decompressed.data(),
                                                &destLength,
                                                (char*)data.data(),
                                                data.size(), 0 /* small */,
                                                0 /* verbosity */);
        assert(result == BZ_OK);
        data.swap(decompressed);
      }
      return data;
    }
  } // namespace xmldb
} // namespace Belle2

