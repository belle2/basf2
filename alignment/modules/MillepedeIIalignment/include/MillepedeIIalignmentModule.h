/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka and Sergey Yashchenko                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <vxd/dataobjects/VxdID.h>
#include <TMatrixD.h>

#include <string>
#include <vector>

using namespace std;
using boost::property_tree::ptree;

namespace Belle2 {
  /**
   * MillepedeII alignment module. The purpose is to define a steering file for
   * MillepedeII using input parameters, execute pede via exernal call,
   * and write obtained alignment parameters to the corresponding xml files
   */

  class MillepedeIIalignmentModule : public Module {

  public:

    /**
     * Default constructor
     */
    MillepedeIIalignmentModule();

    /**
     * Default destructor
     */
    virtual ~MillepedeIIalignmentModule();

    /**
     * endRun procedure to start alignment
     */
    virtual void endRun();

    /**
     * A function to execute pede
     */
    bool MillepedeIIalignmentExecutePede();

    /**
     * A function to read xml alignment files
     */
    bool MillepedeIIalignmentReadXML(const string& xml_filename);

    /**
     * A function to write xml alignment files
     */
    bool MillepedeIIalignmentWriteXML(const string& xml_filename);

    /**
     * A function to read results write xml alignment files
     */
    void readResWriteXml(const string& xml_filename, int type = 0);

  private:
    // time_t MP2startTime; /**< MP2 start time */

    /** struct to hold constraint trafos */
    struct constraintData {
      unsigned int vxdId; /**< vxd id of the sensor */
      TMatrixD loc2gloMatrix; /**< sensor matrix for trafo from local to global alignment parameters */
    };
    std::vector<constraintData> matList; /**< list of constraints */


    std::string m_steeringFileName;  /**< Name of the steering file for running Millepede II alignment */
    std::string m_resultXmlFileName;  /**< Name of xml with results of Millepede II alignment */

  };


}

