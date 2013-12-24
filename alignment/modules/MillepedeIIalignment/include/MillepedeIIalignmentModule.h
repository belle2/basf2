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

    // MP2 start time
    time_t MP2startTime;
    // to hold constraint trafo
    struct constraintData {
      unsigned int vxdId;
      TMatrixD loc2gloMatrix;
    };
    // list of constraints
    std::vector<constraintData> matList;
  };


}

