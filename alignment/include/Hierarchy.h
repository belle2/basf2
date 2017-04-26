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
#include <genfit/StateOnPlane.h>
#include <root/TGeoMatrix.h>

#include <framework/logging/Logger.h>

#include <alignment/GlobalParam.h>


#include <framework/dbobjects/BeamParameters.h>
#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/dbobjects/CDCCalibration.h>
#include <alignment/dbobjects/BKLMAlignment.h>
#include "GlobalLabel.h"
#include <eklm/dbobjects/EKLMAlignment.h>

#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/dbobjects/CDCAlignment.h>
#include <cdc/dbobjects/CDCXtRelations.h>

#include <alignment/GlobalParam.h>

namespace Belle2 {
  class MillepedeAlgorithm;
  namespace alignment {

    /// pair of the global unique id from object with constants and element representing some rigid body in hierarchy
    typedef std::pair<unsigned short, unsigned short> DetectorLevelElement;
    /// pair with global labels and matrix with coresponding global derivatives
    typedef std::pair<std::vector<int>, TMatrixD> GlobalDerivativeSet;
    /// A (null) constraint, vector of pairs of global label and its factor in the constraint
    typedef std::vector<std::pair<int, double>> Constraint;
    /// vector of constraints
    typedef std::map<int, Constraint> Constraints;

    /// Class for alignment/calibration parameter hierarchy & constraints
    class GlobalDerivativesHierarchy {
    public:
      //! Constructor
      GlobalDerivativesHierarchy() {}
      //! Destructor (virtual)
      virtual ~GlobalDerivativesHierarchy() {}

      /// Adds constraints from current hierarchy to a constraints vector
      void buildConstraints(Constraints& constraints);

      /// Recursive function which adds labels and derivatives until top element in hierarchy is found
      GlobalDerivativeSet buildGlobalDerivativesHierarchy(TMatrixD matrixChain, DetectorLevelElement child);

      /// Template function to add relation between two elements (possibly in different objects with constants)
      /// First object is the child object, second its hierarchy parent
      /// @param child is the lement numeric id in child obj
      /// @param mother is the parent object
      /// @param childToMotherParamTransform is the transformation matrix (placement from geometry of the detector)
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
          m_usedUniqueIDs.insert(ChildDBObjectType::getGlobalUniqueID());
          m_usedUniqueIDs.insert(MotherDBObjectType::getGlobalUniqueID());
        } else {
          // Update element transformation if inserted again
          m_lookup[childUID].second = childToMotherParamTransform;
        }
      }

      /// Merge additional set into main set of global labels and derivatives
      ///TODO: move to some utilities
      static void mergeGlobals(GlobalDerivativeSet& main, GlobalDerivativeSet additional);

      /// print the lookup map
      void printHierarchy();

      /// The only function to implement: what are the global labels for the element?
      virtual std::vector<int> getElementLabels(DetectorLevelElement element) = 0;

      /// Get the global unique ids of DB objects used to construct hierarchy
      /// Usefull to update hierarchy only when those changed
      const std::set<unsigned short>& getUsedDBObjUniqueIDs() {return m_usedUniqueIDs;}

    private:
      /// Find the transformation in the lookup
      std::pair<DetectorLevelElement, TMatrixD> getChildToMotherTransform(DetectorLevelElement child);

      //! Map with all the parameter data (child -> (mother, transform_child2mother))
      std::map<DetectorLevelElement, std::pair<DetectorLevelElement, TMatrixD>> m_lookup;
      //! Map of hierarchy relations mother-> child
      std::map<DetectorLevelElement, std::vector<DetectorLevelElement>> m_hierarchy;

      /// The set of unique id of each DB object used for construction
      /// For more efficient updates of hierarchy only when used objects change
      std::set<unsigned short> m_usedUniqueIDs {};
    };

    /// 1D Hierarchy for Lorentz shift correction
    class LorentShiftHierarchy : public GlobalDerivativesHierarchy {
    public:
      /// Constructor
      LorentShiftHierarchy() : GlobalDerivativesHierarchy() {};

      /// Label for lorentz shift parameter
      std::vector<int> getElementLabels(DetectorLevelElement element) final;

      /// Template function to get globals for given db object and its element (and the rest of hierarchy)
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

      /// Derivatives for Lorentz shift in sensor plane
      TMatrixD getLorentzShiftDerivatives(const genfit::StateOnPlane* sop, TVector3 bField);

      /// Template function to insert hierarchy relation bewteen two DB objects and their elements
      template<class ChildDBObjectType, class MotherDBObjectType>
      void insertRelation(unsigned short child, unsigned short mother)
      {
        insert<ChildDBObjectType, MotherDBObjectType>(child, mother, TMatrixD(1, 1).UnitMatrix());
      }

    private:
    };

    /// 6D Hierarchy of rigid bodies
    class RigidBodyHierarchy : public GlobalDerivativesHierarchy {

    public:

      //! Constructor
      RigidBodyHierarchy() : GlobalDerivativesHierarchy() {}

      // Destructor
      ~RigidBodyHierarchy() {}

      /// Rigid body labels
      std::vector<int> getElementLabels(DetectorLevelElement element) override;

      /// Get globals for given db object (and the rest of hierarchy) and its element at StateOnPlane
      template<class LowestLevelDBObject>
      GlobalDerivativeSet getGlobalDerivatives(unsigned short sensor, const genfit::StateOnPlane* sop)
      {
        DetectorLevelElement elementUID = std::make_pair(LowestLevelDBObject::getGlobalUniqueID(), sensor);

        GlobalDerivativeSet sensorDerivs = std::make_pair(getElementLabels(elementUID), getRigidBodyDerivatives(sop));
        mergeGlobals(sensorDerivs, buildGlobalDerivativesHierarchy(getRigidBodyDerivatives(sop), elementUID));
        return sensorDerivs;
      }

      /// Insert hierarchy relation
      template<class ChildDBObjectType, class MotherDBObjectType>
      void insertG4Transform(unsigned short child, unsigned short mother, G4Transform3D childToMother)
      {
        insert<ChildDBObjectType, MotherDBObjectType>(child, mother, convertG4ToRigidBodyTransformation(childToMother));
      }

      /// Insert hierarchy relation
      template<class ChildDBObjectType, class MotherDBObjectType>
      void insertTGeoTransform(unsigned short child, unsigned short mother, TGeoHMatrix childToMother)
      {
        insert<ChildDBObjectType, MotherDBObjectType>(child, mother, convertTGeoToRigidBodyTransformation(childToMother));
      }

      /// 2x6 matrix of rigid body derivatives
      TMatrixD getRigidBodyDerivatives(const genfit::StateOnPlane* sop);

      /// Conversion from G4Transform3D to 6D rigid body transformation parametrization
      TMatrixD convertG4ToRigidBodyTransformation(G4Transform3D g4transform);

      /// Conversion from G4Transform3D to 6D rigid body transformation parametrization
      TMatrixD convertTGeoToRigidBodyTransformation(TGeoHMatrix tgeo);
    private:
    };
  }
}
