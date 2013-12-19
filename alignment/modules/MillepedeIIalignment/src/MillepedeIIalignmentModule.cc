/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka and Sergey Yashchenko                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/MillepedeIIalignment/MillepedeIIalignmentModule.h>

#include <boost/foreach.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace Belle2;
using boost::property_tree::ptree;

REG_MODULE(MillepedeIIalignment)

MillepedeIIalignmentModule::MillepedeIIalignmentModule() :
  Module()
{
  // addParam("GBLBinaryFile", &GBLBinaryFile,
  //          "GBLBinaryFile", GBLBinaryFile);
  MillepedeIIalignmentExecutePede();
}

bool MillepedeIIalignmentModule::MillepedeIIalignmentExecutePede()
{
  bool ok(true);
  std::system("pede steer.txt");
  return ok;
}

bool MillepedeIIalignmentModule::MillepedeIIalignmentReadXML(const string& xml_filename)
{

  bool ok(true);
  // Create an empty property tree object

  // ptree propertyTree;

  // Load the XML file into the property tree. If reading fails
  // (cannot open file, parse error), an exception is thrown.
  // read_xml(xml_filename, propertyTree);
  return ok;
}

bool MillepedeIIalignmentModule::MillepedeIIalignmentWriteXML(const string& xml_filename)
{
  bool ok(true);
  return ok;
}

MillepedeIIalignmentModule::~MillepedeIIalignmentModule()
{
}
