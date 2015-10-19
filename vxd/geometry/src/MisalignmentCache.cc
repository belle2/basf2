/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <vxd/geometry/MisalignmentCache.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;
using namespace boost::property_tree;

namespace Belle2 {
  namespace VXD {

    const TGeoHMatrix& MisalignmentCache::getMisalignment(VxdID id) const
    {
      static const TGeoHMatrix unity;

      id.setSegmentNumber(0);
      MisalignmentMap::const_iterator info = m_misalignments.find(id);
      if (info == m_misalignments.end())
        return unity;
      else return info->second;
    }

    void MisalignmentCache::readMisalignmentsFromXml(const string& filename)
    {
      const double mradToDeg = 0.18 / 3.1415926;
      ptree propertyTree;

      // Identify the location of the xml file.
      string xmlFullPath = FileSystem::findFile(filename);

      if (! FileSystem::fileExists(xmlFullPath)) {
        B2ERROR("The filename: " << filename << endl <<
                "resolved to: " << xmlFullPath << endl <<
                "by FileSystem::findFile but has no file with it." << endl <<
                "Misaligner cache cannot be initialized." << endl <<
                "No misalignment will be applied." << endl
               );
        return;
      }

      try {
        read_xml(xmlFullPath, propertyTree);
      } catch (std::exception const& ex) {
        B2ERROR("Excpetion raised during xml parsing " << ex.what() << endl <<
                "Misaligner cache cannot be initialized." << endl <<
                "No misalignment will be applied." << endl);
        return;
      } catch (...) {
        B2ERROR("Unknown excpetion raised during xml parsing "
                "Misaligner cache cannot be initialized." << endl <<
                "No misalignment will be applied." << endl);
        return;
      }

      try {
        // traverse pt: let us navigate through the daughters of <SVD>
        for (ptree::value_type const& detector : propertyTree.get_child("SVDMisalignment")) {
          for (ptree::value_type const& layer : detector.second.get_child("layer"))
            for (ptree::value_type const& ladder : layer.second.get_child("ladder"))
              for (ptree::value_type const& sensor : ladder.second.get_child("sensor")) {
                // Only here we have some data
                TGeoHMatrix transform;
                transform.SetDx(sensor.second.get<double>("du") * Unit::um);
                transform.SetDy(sensor.second.get<double>("dv") * Unit::um);
                transform.SetDz(sensor.second.get<double>("dw") * Unit::um);
                transform.RotateX(sensor.second.get<double>("dalpha") * mradToDeg);
                transform.RotateY(sensor.second.get<double>("dbeta") * mradToDeg);
                transform.RotateZ(sensor.second.get<double>("dgamma") * mradToDeg);
                VxdID sensorID(sensor.second.get<VxdID::baseType>("<xmlattr>.id"));
                m_misalignments[sensorID] = transform;
              }
        }
      } catch (...) {
        B2ERROR("Unknown excpetion raised during map initialization! "
                "Misalignment data corrupted." << endl <<
                "No misalignment will be applied." << endl);
        m_misalignments.clear();
        return;
      }
      m_isAlive = true;
    }

    MisalignmentCache& MisalignmentCache::getInstance()
    {
      static unique_ptr<MisalignmentCache> instance(new MisalignmentCache());
      return *instance;
    }

  } // VXD namespace
} //Belle2 namespace
