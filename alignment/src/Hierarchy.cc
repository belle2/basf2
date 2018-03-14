/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/Hierarchy.h>
#include <alignment/GlobalLabel.h>
#include <fstream>

namespace Belle2 {
  namespace alignment {

    // ------------------ GlobalDerivativesHierarchy ----------------------------

    void GlobalDerivativesHierarchy::buildConstraints(Constraints& constraints)
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

    Belle2::alignment::GlobalDerivativeSet GlobalDerivativesHierarchy::buildGlobalDerivativesHierarchy(TMatrixD matrixChain,
        Belle2::alignment::DetectorLevelElement child)
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

    void GlobalDerivativesHierarchy::mergeGlobals(Belle2::alignment::GlobalDerivativeSet& main,
                                                  Belle2::alignment::GlobalDerivativeSet additional)
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

    void GlobalDerivativesHierarchy::printHierarchy()
    {
      for (auto& entry : m_lookup) {
        std::cout << "Child  : " << entry.first.second << std::endl;
        std::cout << "Mother :" << entry.second.first.second << std::endl;
        entry.second.second.Print();
      }
    }

    std::pair< Belle2::alignment::DetectorLevelElement, TMatrixD > GlobalDerivativesHierarchy::getChildToMotherTransform(
      Belle2::alignment::DetectorLevelElement child)
    {
      auto entry = m_lookup.find(child);
      if (entry == m_lookup.end())
        return std::make_pair(std::make_pair(0, 0), TMatrixD());

      return entry->second;
    }

    // ------------------ LorentShiftHierarchy ----------------------------

    std::vector< int > LorentShiftHierarchy::getElementLabels(Belle2::alignment::DetectorLevelElement element)
    {
      std::vector<int> labels;
      GlobalLabel label;
      label.construct(element.first, element.second, 0);
      // TODO: constants instead of numbers
      labels.push_back(label.setParameterId(20));

      return labels;
    }

    TMatrixD LorentShiftHierarchy::getLorentzShiftDerivatives(const genfit::StateOnPlane* sop, TVector3 bField)
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

    // ------------------ RigidBodyHierarchy ----------------------------

    std::vector< int > RigidBodyHierarchy::getElementLabels(Belle2::alignment::DetectorLevelElement element)
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

    TMatrixD RigidBodyHierarchy::getRigidBodyDerivatives(const genfit::StateOnPlane* sop)
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

    TMatrixD RigidBodyHierarchy::convertG4ToRigidBodyTransformation(G4Transform3D g4transform)
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

    TMatrixD RigidBodyHierarchy::convertTGeoToRigidBodyTransformation(TGeoHMatrix tgeo)
    {
      TMatrixD rotationT(3, 3);
      TMatrixD offset(3, 3);
      for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
          rotationT(i, j) = tgeo.GetRotationMatrix()[i * 3 + j];
          // or transposed???? have to check
        }
      }
      double xDet = tgeo.GetTranslation()[0];
      double yDet = tgeo.GetTranslation()[1];
      double zDet = tgeo.GetTranslation()[2];
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
  }
}