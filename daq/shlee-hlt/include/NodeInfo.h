/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef NODEINFO_H
#define NODEINFO_H

#include <string>
#include <vector>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>

namespace Belle2 {

  //! NodeInfo class
  /*! This class is to encapsulate the individual node information
  */
  class NodeInfo {
  public:
    //! Constructor
    NodeInfo();
    //! Constructor
    NodeInfo(const NodeInfo& nodeinfo);
    //! Destructor
    ~NodeInfo();

    //! Overriding for = operator
    NodeInfo& operator=(const NodeInfo& nodeinfo);

    //! Clear the information
    void clear();

    //! Modifier to unit number
    void unitNo(int unitNo);
    //! Accessor to unit number
    int unitNo();
    //! Modifier to node number
    void nodeNo(int nodeNo);
    //! Accessor to node number
    int nodeNo();
    //! Generate the unique key based on unit and node number
    int generateKey();

    //! Modifier to experiment number
    void expNo(int expNo);
    //! Accessor to experiment number
    int expNo();
    //! Modifier to run start number
    void runStart(int runStart);
    //! Accessor to run start number
    int runStart();
    //! Modifier to run end number
    void runEnd(int runEnd);
    //! Accessor to run end number
    int runEnd();

    //! Modifier to type of node
    void type(std::string type);
    //! Accessor to type of node
    const std::string type();

    //! Modifier to IP address of manager node
    void managerIP(std::string managerIP);
    //! Accessor to IP address of manager node
    const std::string managerIP();
    //! Modifier to the IP address of this node
    void selfIP(std::string selfIP);
    //! Accessor to the IP address of this node
    const std::string selfIP();
    //! Add an IP address of data source
    void sourceIP(std::string sourceIP);
    //! Accessor to the container for IP addresses of data sources
    const std::vector<std::string> sourceIP();
    //! Add an IP address of data target
    void targetIP(std::string targetIP);
    //! Accessor to the container for IP addresses of data targets
    const std::vector<std::string> targetIP();

    //! Display information (Development purpose)
    void display();

  private:
    friend class boost::serialization::access;        /**< Friend class for boost::serialization */
    //! Function for boost::serialization
    template<class Archive>
    void serialize(Archive& ar, unsigned int ver) {
      ar& m_unitNo;
      ar& m_nodeNo;
      ar& m_expNo;
      ar& m_runStart;
      ar& m_runEnd;
      ar& m_type;
      ar& m_managerIP;
      ar& m_selfIP;
      ar& m_sourceIP;
      ar& m_targetIP;
    }

    int m_unitNo;                         /**< Unit number of this node */
    int m_nodeNo;                         /**< Node number of this node */
    int m_expNo;                          /**< Experiment number */
    int m_runStart;                       /**< Run start number */
    int m_runEnd;                         /**< Run end number */

    std::string m_type;                   /**< Type of this node */

    std::string m_managerIP;              /**< IP address of manager node */
    std::string m_selfIP;                 /**< IP address of this node */
    std::vector<std::string> m_sourceIP;  /**< Container for IP addresses of data sources */
    std::vector<std::string> m_targetIP;  /**< Container for IP addresses of data targets */
  };
}

#endif
