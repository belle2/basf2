/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <map>
#include <Geant4/G4Transform3D.hh>
#include <root/TMatrixD.h>

#include <alignment/GlobalLabel.h>

#include <genfit/StateOnPlane.h>
#include <../vxd/geometry/include/GeoCache.h>


namespace Belle2 {
  namespace alignment {
    typedef std::pair<unsigned short, unsigned short> DetectorLevelElement;
    typedef std::pair<std::vector<int>, TMatrixD> GlobalDerivativeSet;
    typedef std::vector<std::pair<int, double>> Constraint;
    typedef std::map<int, Constraint> Constraints;
    //! Class for alignment/calibration parameter hierarchy
    class GlobalDerivativesHierarchy {

    public:

      //! Constructor
      GlobalDerivativesHierarchy() {}
      //! Destructor (virtual)
      virtual ~GlobalDerivativesHierarchy() {}


      void buildConstraints(Constraints& constraints)
      {
        for (auto& parent_childs : m_hierarchy) {
          auto parent = parent_childs.first;
          auto childs = parent_childs.second;

          auto parentLabels = getElementLabels(parent);
          for (unsigned int iCon = 0; iCon < parentLabels.size(); iCon++) {
            auto& constraint = constraints.insert(std::make_pair(parentLabels[iCon], Constraint())).first->second;
            //auto & constraint = constraints[parentLabels[iCon]];

            for (unsigned int j = 0; j < childs.size(); j++) {
              auto child = childs[j];
              auto childLabels = getElementLabels(child);
              for (unsigned int iPar = 0; iPar < childLabels.size(); iPar++) {
                double coefficient = getChildToMotherTransform(child).second(iCon, iPar);
                if (fabs(coefficient) > 1.0e-14) {
                  constraint.push_back(std::make_pair(childLabels[iPar], coefficient));
                }
              }
            }
          }
        }
      }

      GlobalDerivativeSet buildGlobalDerivativesHierarchy(TMatrixD matrixChain, DetectorLevelElement child)
      {
        auto loc2glo = getChildToMotherTransform(child);

        if (loc2glo.first == std::make_pair((unsigned short)0, (unsigned short)0))
          return std::make_pair(std::vector<int>(), TMatrixD());


        TMatrixD glo2loc(loc2glo.second.Invert());
        TMatrixD drdparent = matrixChain * glo2loc;


        GlobalDerivativeSet retVal = std::make_pair(getElementLabels(loc2glo.first), drdparent);
        mergeGlobals(retVal, buildGlobalDerivativesHierarchy(drdparent, loc2glo.first));

        return retVal;
      }

      template<class ChildDBObjectType, class MotherDBObjectType>
      void insert(unsigned short child, unsigned short mother, TMatrixD childToMotherParamTransform)
      {
        auto childUID = std::make_pair(ChildDBObjectType::getGlobalUniqueID(), child);
        auto parentUID = std::make_pair(MotherDBObjectType::getGlobalUniqueID(), mother);

        if (m_lookup.insert(std::make_pair(childUID, std::make_pair(parentUID, childToMotherParamTransform))).second) {
          // Just inserted an child element (so each child is inserted exactly once)
          // Try to insert parent with empty vect (does nothing if parent already in map)
          m_hierarchy.insert(std::make_pair(parentUID, std::vector<DetectorLevelElement>()));
          // insert child (can happen only once, so the vect is unique)
          m_hierarchy[parentUID].push_back(childUID);
        } else {
          // Update element transformation if inserted again
          m_lookup[childUID].second = childToMotherParamTransform;
        }
      }

      static void mergeGlobals(GlobalDerivativeSet& main, GlobalDerivativeSet additional)
      {
        if (additional.first.empty())
          return;

        // Create composed matrix of derivatives
        //TODO: check main and additional matrix has the same number of rows
        TMatrixD allDerivatives(main.second.GetNrows(), main.second.GetNcols() + additional.second.GetNcols());
        allDerivatives.Zero();
        allDerivatives.SetSub(0, 0, main.second);
        allDerivatives.SetSub(0, main.second.GetNcols(), additional.second);

        // Merge labels
        main.first.insert(main.first.end(), additional.first.begin(), additional.first.end());
        // Update matrix
        main.second.ResizeTo(allDerivatives);
        main.second = allDerivatives;

      }

      void printHierarchy()
      {
        for (auto& entry : m_lookup) {
          std::cout << "Child  : " << entry.first.second << std::endl;
          std::cout << "Mother :" << entry.second.first.second << std::endl;
          entry.second.second.Print();
        }
      }

      virtual std::vector<int> getElementLabels(DetectorLevelElement element) = 0;
    private:
      std::pair<DetectorLevelElement, TMatrixD> getChildToMotherTransform(DetectorLevelElement child)
      {
        auto entry = m_lookup.find(child);
        if (entry == m_lookup.end())
          return std::make_pair(std::make_pair(0, 0), TMatrixD());

        return entry->second;
      }

      //! Map with all the parameter data (child -> (mother, transform_child2mother))
      std::map<DetectorLevelElement, std::pair<DetectorLevelElement, TMatrixD>> m_lookup;
      //! Map of hierarchy relations mother-> child
      std::map<DetectorLevelElement, std::vector<DetectorLevelElement>> m_hierarchy;
    };

    /**
    class GlobalDerivativesBase {
      virtual GlobalDerivativeSet getGlobalDerivatives() = 0;
    };

    template<class DBObjType>
    class GlobalDerivatives : public GlobalDerivativesBase {
      GlobalDerivativeSet getGlobalDerivatives(const genfit::StateOnPlane* sop) {
        std::vector<int> labels;
        GlobalLabel label;
        label.construct<DBObjType>(getElementID(), 0);
        for (auto paramID : getElementParameterIDs())
          labels.push_back(label.setParameterId(paramID));

        return {labels, getElementDerivatives()};
      }
      unsigned short getElementID() = 0;
      std::vector<unsigned short> getElementParameterIDs() = 0;
      TMatrixD getElementDerivatives() = 0;
    };

    template<class DBObjType>
    class RigidBodyDerivatives : public GlobalDerivatives<DBObjType> {
      std::vector<int> getElementParameterIDs() {
        return {1,2,3,4,5,6};
      }
      TMatrixD getElementDerivatives(const genfit::StateOnPlane* sop) {
        TMatrixD derGlobal(2, 6);

        // track u-slope in local sensor system
        double uSlope = sop->getState()[1];
        // track v-slope in local sensor system
        double vSlope = sop->getState()[2];
        // Predicted track u-position in local sensor system
        double uPos = sop->getState()[3];
        // Predicted track v-position in local sensor system
        double vPos = sop->getState()[4];

        //Global derivatives for alignment in sensor local coordinates

        derGlobal(0, 0) = 1.0;
        derGlobal(0, 1) = 0.0;
        derGlobal(0, 2) = - uSlope;
        derGlobal(0, 3) = vPos * uSlope;
        derGlobal(0, 4) = -uPos * uSlope;
        derGlobal(0, 5) = vPos;

        derGlobal(1, 0) = 0.0;
        derGlobal(1, 1) = 1.0;
        derGlobal(1, 2) = - vSlope;
        derGlobal(1, 3) = vPos * vSlope;
        derGlobal(1, 4) = -uPos * vSlope;
        derGlobal(1, 5) = -uPos;

        return derGlobal;
      }
    };

    template<class VXDRecoHitType, class DBObjType>
    class VXDRigidBodyDerivatives : public RigidBodyDerivatives<DBObjType> {
      VXDRigidBodyDerivatives(VXDRecoHitType* recoHit) : m_elementID(recoHit->getSensorID().getID()) {}
      unsigned short getElementID() {
        return m_elementID;
      }
      unsigned short m_elementID;
    };


    VXDRigidBodyDerivatives<PXDRecoHit, PXDSensorAlignment> pxdDerivs(this);
    VXDRigidBodyDerivatives<SVDRecoHit, SVDSensorAlignment> svdDerivs(this);
    VXDRigidBodyDerivatives<SVDRecoHit2D, SVDSensorAlignment> svd2dDerivs(this);
    CDCTímeZeroDerivatives<CDCRecoHit, CDCTimeZeros> cdcTimeZeroDerivs(this);
    VXDLorentzAngleDerivatives<PXDRecoHit, PXDLorentzAngles> las(this);
    **/
    class LorentShiftHierarchy : public GlobalDerivativesHierarchy {
    public:
      LorentShiftHierarchy() : GlobalDerivativesHierarchy() {};
      std::vector<int> getElementLabels(DetectorLevelElement element)
      {
        std::vector<int> labels;
        GlobalLabel label;
        label.construct(element.first, element.second, 0);
        // TODO: constants instead of numbers
        labels.push_back(label.setParameterId(20));

        return labels;
      }
      template<class LowestLevelDBObject>
      GlobalDerivativeSet getGlobalDerivatives(unsigned short sensor, const genfit::StateOnPlane* sop, TVector3 bField)
      {
        if (bField.Mag() < 1.e-10)
          return std::make_pair(std::vector<int>(), TMatrixD());

        DetectorLevelElement elementUID = std::make_pair(LowestLevelDBObject::getGlobalUniqueID(), sensor);

        GlobalDerivativeSet sensorDerivs = std::make_pair(getElementLabels(elementUID), getLorentzShiftDerivatives(sop, bField));
        mergeGlobals(sensorDerivs, buildGlobalDerivativesHierarchy(getLorentzShiftDerivatives(sop, bField), elementUID));
        return sensorDerivs;
      }
      TMatrixD getLorentzShiftDerivatives(const genfit::StateOnPlane* sop, TVector3 bField)
      {
        // values for global derivatives
        //TMatrixD derGlobal(2, 6);
        TMatrixD derGlobal(2, 1);
        derGlobal.Zero();

        // electrons in device go in local w-direction to P-side
        TVector3 v = sop->getPlane()->getNormal();
        // Lorentz force (without E-field) direction
        TVector3 F_dir = v.Cross(bField);
        // ... projected to sensor coordinates:
        genfit::StateOnPlane localForce(*sop);
        localForce.setPosMom(sop->getPos(), F_dir);
        TVector3 lorentzLocal(localForce.getState()[3], localForce.getState()[4], 0); // or 0,1?
        // Lorentz shift = parameter(layer) * B_local
        derGlobal(0, 0) = lorentzLocal(0);
        derGlobal(1, 0) = lorentzLocal(1);

        return derGlobal;
      }

      template<class ChildDBObjectType, class MotherDBObjectType>
      void insertRelation(unsigned short child, unsigned short mother)
      {
        insert<ChildDBObjectType, MotherDBObjectType>(child, mother, TMatrixD(1, 1).UnitMatrix());
      }

    private:
    };

    class RigidBodyHierarchy : public GlobalDerivativesHierarchy {

    public:

      //! Constructor
      RigidBodyHierarchy() : GlobalDerivativesHierarchy() {}

      // Destructor
      ~RigidBodyHierarchy() {}

      std::vector<int> getElementLabels(DetectorLevelElement element) override
      {
        std::vector<int> labels;
        GlobalLabel label;
        label.construct(element.first, element.second, 0);
        // TODO: constants instead of numbers
        labels.push_back(label.setParameterId(1));
        labels.push_back(label.setParameterId(2));
        labels.push_back(label.setParameterId(3));
        labels.push_back(label.setParameterId(4));
        labels.push_back(label.setParameterId(5));
        labels.push_back(label.setParameterId(6));

        return labels;
      }

      template<class LowestLevelDBObject>
      GlobalDerivativeSet getGlobalDerivatives(unsigned short sensor, const genfit::StateOnPlane* sop)
      {
        DetectorLevelElement elementUID = std::make_pair(LowestLevelDBObject::getGlobalUniqueID(), sensor);

        GlobalDerivativeSet sensorDerivs = std::make_pair(getElementLabels(elementUID), getRigidBodyDerivatives(sop));
        mergeGlobals(sensorDerivs, buildGlobalDerivativesHierarchy(getRigidBodyDerivatives(sop), elementUID));
        return sensorDerivs;
      }

      template<class ChildDBObjectType, class MotherDBObjectType>
      void insertG4Transform(unsigned short child, unsigned short mother, G4Transform3D childToMother)
      {
        insert<ChildDBObjectType, MotherDBObjectType>(child, mother, convertG4ToRigidBodyTransformation(childToMother));
      }

      TMatrixD getRigidBodyDerivatives(const genfit::StateOnPlane* sop)
      {
        TMatrixD derGlobal(2, 6);

        // track u-slope in local sensor system
        double uSlope = sop->getState()[1];
        // track v-slope in local sensor system
        double vSlope = sop->getState()[2];
        // Predicted track u-position in local sensor system
        double uPos = sop->getState()[3];
        // Predicted track v-position in local sensor system
        double vPos = sop->getState()[4];

        //Global derivatives for alignment in sensor local coordinates

        derGlobal(0, 0) = 1.0;
        derGlobal(0, 1) = 0.0;
        derGlobal(0, 2) = - uSlope;
        derGlobal(0, 3) = vPos * uSlope;
        derGlobal(0, 4) = -uPos * uSlope;
        derGlobal(0, 5) = vPos;

        derGlobal(1, 0) = 0.0;
        derGlobal(1, 1) = 1.0;
        derGlobal(1, 2) = - vSlope;
        derGlobal(1, 3) = vPos * vSlope;
        derGlobal(1, 4) = -uPos * vSlope;
        derGlobal(1, 5) = -uPos;

        return derGlobal;
      }

      TMatrixD convertG4ToRigidBodyTransformation(G4Transform3D g4transform)
      {
        TMatrixD rotationT(3, 3);
        TMatrixD offset(3, 3);
        for (int i = 0; i < 3; i++) {
          for (int j = 0; j < 3; j++) {
            rotationT(i, j) = g4transform.getRotation()(i, j);//trafo(j, i);
            // or transposed???? have to check
          }
        }
        double xDet = g4transform.getTranslation()(0);
        double yDet = g4transform.getTranslation()(1);
        double zDet = g4transform.getTranslation()(2);
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

        return loc2glo;
      }

    private:


    };

    class GlobalParamSetBase {
    public:
      virtual unsigned short getGlobalUniqueID() = 0;
      virtual double getGlobalParam(unsigned short, unsigned short) = 0;
      virtual void setGlobalParam(double, unsigned short, unsigned short) = 0;
      virtual std::vector<DetectorLevelElement> listGlobalParams() = 0;

      virtual void construct() = 0;
    };

    template<class DBObjType>
    class GlobalParamSet : public GlobalParamSetBase {
    public:
      GlobalParamSet() {}
      ~GlobalParamSet() {m_object.reset();}

      virtual unsigned short getGlobalUniqueID() final {return DBObjType::getGlobalUniqueID();}
      virtual double getGlobalParam(unsigned short element, unsigned short param) final {ensureConstructed(); return m_object->getGlobalParam(element, param);}
      virtual void setGlobalParam(double value, unsigned short element, unsigned short param) final {ensureConstructed(); m_object->setGlobalParam(value, element, param);}
      virtual std::vector<DetectorLevelElement> listGlobalParams() final {ensureConstructed(); return m_object->listGlobalParams();}

      virtual void construct() final {m_object.reset(new DBObjType());}
    private:
      std::shared_ptr<DBObjType> m_object {};
      void ensureConstructed() {if (!m_object) construct();}
    };

    class Belle2Alignment {
    public:
      static unsigned short getGlobalUniqueID() {return 1;}
      double getGlobalParam(unsigned short, unsigned short) {return 0.;}
      void setGlobalParam(double, unsigned short, unsigned short) {}
      std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams() {return {};}
    };

    class HierarchyManager {

    public:
      ~HierarchyManager() {}

      static HierarchyManager& getInstance()
      {
        static std::unique_ptr<HierarchyManager> instance(new HierarchyManager());
        return *instance;
      }

      RigidBodyHierarchy& getAlignmentHierarchy() { return *m_alignment; }
      LorentShiftHierarchy& getLorentzShiftHierarchy() { return *m_lorentzShift; }

      bool initialize()
      {
        /**
        auto & geo = VXD::GeoCache::getInstance();
        for (auto layer : geo.getLayers(Belle2::VXD::SensorInfoBase::PXD))
          for (auto ladder : geo.getLadders(layer))
            for (auto sensor : geo.getSensors(ladder))
              m_alignment->insertG4Transform<PXDSensorAlignment, Belle2Alignment>(sensor.getID(), 0, geo.get(sensor).getTransformation());

        for (auto layer : geo.getLayers(Belle2::VXD::SensorInfoBase::SVD))
          for (auto ladder : geo.getLadders(layer))
            for (auto sensor : geo.getSensors(ladder))
              m_alignment->insertG4Transform<SVDSensorAlignment, Belle2Alignment>(sensor.getID(), 0, geo.get(sensor).getTransformation());

            **/
        return true;
      }

      void initializeGlobalVector()
      {
        m_globalVector.emplace_back(new GlobalParamSet<Belle2Alignment>());
      }

    private:
      /** Singleton class, hidden constructor */
      HierarchyManager() {};
      /** Singleton class, hidden copy constructor */
      HierarchyManager(const HierarchyManager&);
      /** Singleton class, hidden assignment operator */
      HierarchyManager& operator=(const HierarchyManager&);

      std::unique_ptr<RigidBodyHierarchy> m_alignment {new RigidBodyHierarchy()};
      std::unique_ptr<LorentShiftHierarchy> m_lorentzShift {new LorentShiftHierarchy()};

      std::vector<std::shared_ptr<GlobalParamSetBase>> m_globalVector {};

    };
  }
}
