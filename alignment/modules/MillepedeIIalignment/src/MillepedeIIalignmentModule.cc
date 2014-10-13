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
#include <TRandom.h>
#include <TMatrixDfwd.h>
#include <TMatrixT.h>
#include <TMath.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VxdID.h>

using namespace Belle2;
using boost::property_tree::ptree;

REG_MODULE(MillepedeIIalignment)

MillepedeIIalignmentModule::MillepedeIIalignmentModule() :
  Module()
{
  addParam("steeringFileName", m_steeringFileName, "Name of the steering file to run Millepede II alignment", string("steer.txt"));
  addParam("resultXmlFileName", m_resultXmlFileName, "Name of the xml with results of Millepede II alignment", string("displacements.txt"));

}

void MillepedeIIalignmentModule::endRun()
{

  MillepedeIIalignmentExecutePede();

  MillepedeIIalignmentWriteXML(m_resultXmlFileName);

}

bool MillepedeIIalignmentModule::MillepedeIIalignmentExecutePede()
{
  bool ok(true);
  //ofstream positions("sensor_positions.txt");
  // Write constraints file based on current geometry
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  // Find all sensors and create a matrix of tranformation from local to global
  // parameters for each sensor
  for (auto layer : geo.getLayers(VXD::SensorInfoBase::VXD)) {
    for (auto ladder : geo.getLadders(layer)) {
      for (auto id : geo.getSensors(ladder)) {
        TMatrixD rotationT(3, 3);
        TMatrixD offset(3, 3);
        TVector3 detPos = geo.get(id).pointToGlobal(TVector3(0., 0., 0.));
        //positions << (unsigned int) id << ": x = " << detPos[0] << " y = " << detPos[1] << " z = " << detPos[2] << endl;

        const double* trans = geo.get(id).getTransformation().GetRotationMatrix();
        for (int i = 0; i < 3; i++) {
          for (int j = 0; j < 3; j++) {
            rotationT(i, j) = trans[3 * i + j];
          }
        }
        //R.Print();
        double xDet = detPos[0];
        double yDet = detPos[1];
        double zDet = detPos[2];
        //double phi = atan2(yDet, xDet);
        //if (phi < 0.) phi = phi + TMath::Pi();
        //double sinPhi = sin(phi);
        //double cosPhi = cos(phi);

        //rotationT.Zero();
        //rotationT(0, 0) = - sinPhi;
        //rotationT(0, 2) = cosPhi;
        //rotationT(1, 0) = cosPhi;
        //rotationT(1, 2) = sinPhi;
        //rotationT(2, 1) = 1.;

        offset.Zero();
        offset(0, 1) = - zDet;
        offset(0, 2) = yDet;
        offset(1, 0) = zDet;
        offset(1, 2) = - xDet;
        offset(2, 0) = - yDet;
        offset(2, 1) = xDet;

        TMatrixD loc2glo(6, 6);
        loc2glo.Zero();
        loc2glo.SetSub(0, 0, rotationT);
        loc2glo.SetSub(0, 3, -1. * offset * rotationT);
        loc2glo.SetSub(3, 3, rotationT);
        //loc2glo.Print();

        constraintData cData;
        cData.vxdId = (unsigned int) id;
        cData.loc2gloMatrix.ResizeTo(6, 6);
        cData.loc2gloMatrix = loc2glo;
        matList.push_back(cData);
      }
    }
  }
  // output fortran file that can be directly linked from MP2 steering file
  ofstream f("constraints.txt");
  for (int iCon = 0; iCon < 6; iCon++) {
    f << endl;
    f << "Constraint 0. ! constraint for global parmeter " << iCon + 1 << endl;

    for (unsigned int j = 0; j < matList.size(); j++) {
      constraintData& data = matList.at(j);
      for (int iPar = 0; iPar < 6; iPar++)
        if (fabs(data.loc2gloMatrix(iCon, iPar)) > 1.0e-14)
          f << (data.vxdId * 10 + iPar + 1) << " " << data.loc2gloMatrix(iCon, iPar) << endl;
    }
  }
  f.close();

  std::cout << "Starting Millepede II Alignment..." << std::endl;
  // store start time of processing
  // time(&MP2startTime);
  // execute pede;
  std::string cmd("pede");
  cmd = cmd + " " + m_steeringFileName;
  std::system(cmd.c_str());
  // now open millepede.end file to see what happened (we need hiher pede version)
  return ok;
}
/*
bool MillepedeIIalignmentModule::MillepedeIIalignmentReadXML(const string& xml_filename)
{

  bool ok(true);
  ok = (xml_filename != "");
  // Create an empty property tree object

  // ptree propertyTree;

  // Load the XML file into the property tree. If reading fails
  // (cannot open file, parse error), an exception is thrown.
  // read_xml(xml_filename, propertyTree);
  return ok;
}
*/
void MillepedeIIalignmentModule::readResWriteXml(const string& xml_filename, int type)
{

  double ranSigmaShift = 0.01;
  double ranSigmaAngle = 0.001;

  ofstream xml(xml_filename);
  string line;
  ifstream res("millepede.res");

  std::map<int, std::string>paramNames;
  paramNames.insert(std::pair<int, std::string>(1, "du"));
  paramNames.insert(std::pair<int, std::string>(2, "dv"));
  paramNames.insert(std::pair<int, std::string>(3, "dw"));
  paramNames.insert(std::pair<int, std::string>(4, "alpha"));
  paramNames.insert(std::pair<int, std::string>(5, "beta"));
  paramNames.insert(std::pair<int, std::string>(6, "gamma"));
  // skip fortran line
  getline(res, line);

  xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  xml << "<Alignment xmlns:xi=\"http://www.w3.org/2001/XInclude\">" << endl;
  // xml << "  <Millepede>" << endl;
  // xml << "    <StartTime>" << endl << "      " << ctime(&MP2startTime) << "    </StartTime>" << endl;
  //
  // time_t now;
  // time(&now);
  //
  // xml << "    <EndTime>" << endl << "      " << ctime(&now) << "    </EndTime>" << endl;
  // xml << "    <ExitCode>N/A</ExitCode>" << endl;
  // xml << "    <ExitMessage>Not supported in this version of Millepede</ExitMessage>" << endl;
  // xml << "  </Millepede>" << endl;
  if (type == 2) xml << "<!-- WARNING: Generated IDEAL alignment. All params zero. -->" << endl;
  if (type == 1) xml << "<!-- WARNING: Randomly generated MISALIGNMENT. -->" << endl;
  unsigned int lastSensor = 0;
  bool startComp = false;
  while (getline(res, line)) {
    int label = -1;
    double param = 0.;
    double presigma = 0.;
    double differ = 0.;
    double error = 0.;
    stringstream lineSTream;
    lineSTream << line;
    lineSTream >> label >> param >> presigma >> differ >> error;

    // Now decode vxd id
    if (label < 10) continue;

    unsigned int id(floor(label / 10));
    unsigned int vxdId = id;
    unsigned int paramId = label - 10 * id;
    // skip segment (5 bits)
    id = id >> 5;
    unsigned int sensor = id & 7;
    id = id >> 3;
    unsigned int ladder = id & 31;
    id = id >> 5;
    unsigned int layer = id & 7;

    if (lastSensor != vxdId) {
      if (startComp) {
        xml << "  </Align>" << endl;
        startComp = false;
      }
      xml << "  <Align component=\"" << layer << "." << ladder << "." << sensor << "\">" << endl;
      lastSensor = vxdId;
      startComp = true;
    }
    xml << "    <" << paramNames.find(paramId)->second;
    if (presigma < 0.) xml << " fixed=\"true\"";

    if (type == 1) {
      if (paramId == 1 || paramId == 2) param = gRandom->Gaus(0., ranSigmaShift);
      else if (paramId == 6) param = gRandom->Gaus(0., ranSigmaAngle);
      else param = 0.;
    }

    if (paramId >= 1 && paramId <= 3)
      xml << " unit=\"cm\"";
    if (paramId >= 4 && paramId <= 6)
      xml << " unit=\"rad\"";

    if (error) xml << " error=\"" << error << "\"";

    if (type == 2) param = 0.;

    xml << ">" << param << "</" << paramNames.find(paramId)->second << ">" << endl;
  }
  if (startComp) {
    xml << "  </Align>" << endl;
  }
  xml << "</Alignment>" << endl;
}

bool MillepedeIIalignmentModule::MillepedeIIalignmentWriteXML(const string& xml_filename)
{
  bool ok(true);
  readResWriteXml(xml_filename, 0);
  //readResWriteXml("misalignment.xml", 1);
  //readResWriteXml("ideal.xml", 2);
  return ok;
}

MillepedeIIalignmentModule::~MillepedeIIalignmentModule()
{
}
