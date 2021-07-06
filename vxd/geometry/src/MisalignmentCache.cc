/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <vxd/geometry/MisalignmentCache.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;
using namespace boost::property_tree;

namespace Belle2 {
  namespace VXD {

    const TGeoHMatrix& MisalignmentCache::getMisalignmentTransform(VxdID id) const
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

    MisalignmentCache::MisalignmentShiftType MisalignmentCache::getMisalignmentShift(const VXDTrueHit* hit)
    {
      if (!m_isAlive || !hit) return make_tuple(false, 0.0, 0.0);
      VxdID sensorID = hit->getSensorID();
      const TGeoHMatrix& transform = getMisalignmentTransform(sensorID);
      // We need entry point as a reference - the point on the original track unaffected by passage through the sensor. We also don't care for w and set it to zero.
      const double xea[3] = {hit->getEntryU(), hit->getEntryV(), 0.0};
      TVector3 tev(hit->getEntryMomentum().Unit());
      const double tea[3] = {tev.X(), tev.Y(), tev.Z()};
      double xca[3], tca[3];
      transform.MasterToLocal(xea, xca);
      transform.MasterToLocalVect(tea, tca);
      if (abs(tca[2]) > 0.0) {
        double factor = - xca[2] / tca[2];
        double dx = xca[0] + factor * tca[0] - xea[0];
        double dy = xca[1] + factor * tca[1] - xea[1];
        return make_tuple(true, dx, dy);
      } else {
        return make_tuple(false, 0.0, 0.0);
      }
    }

    MisalignmentCache& MisalignmentCache::getInstance()
    {
      static unique_ptr<MisalignmentCache> instance(new MisalignmentCache());
      return *instance;
    }

  } // VXD namespace
} //Belle2 namespace
