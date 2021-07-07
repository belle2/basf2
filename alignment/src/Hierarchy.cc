/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/Hierarchy.h>

#include <alignment/GlobalLabel.h>

#include <boost/crc.hpp>

namespace Belle2 {
  namespace alignment {

    // ------------------ GlobalDerivativesHierarchy ----------------------------

    void GlobalDerivativesHierarchy::buildConstraints(Constraints& constraints)
    {
      for (auto& parent_childs : m_hierarchy) {
        auto parent = parent_childs.first;
        auto childs = parent_childs.second;

        // We need to check if such constraint entry already exists.
        // For timedep parameters, some to all labels (and coefficients optionally, too) can change and
        // in such case, we need a new constraint entry.
        // Note the checksum for a constraint does not depend on the parent element. If parent object changes,
        // the relative transofrmation to its children are unchanged. While if (some of) the childs change,
        // for each time interval they are constant, there must a new constraint entry as this is an independent
        // linear combination of parameters (because there are other parameters).
        // Continued bellow...
        boost::crc_32_type crc32;

        auto parentLabels = getElementLabels(parent);
        for (unsigned int iCon = 0; iCon < parentLabels.size(); iCon++) {
          auto constraint = Constraint();
          //auto & constraint = constraints[parentLabels[iCon]];

          // No constraints if parent is global reference frame
          // All subdetectors are actually nominally placed at constant position and rotation
          // and rotating the detectors could only happen due cohherent movements of sub-structures (CDC layers, VXD half-shells)
          if (parentLabels[iCon] == 0) continue;

          for (unsigned int j = 0; j < childs.size(); j++) {
            auto child = childs[j];
            auto childLabels = getElementLabels(child);

            for (unsigned int iPar = 0; iPar < childLabels.size(); iPar++) {
              double coefficient = getChildToMotherTransform(child).second(iCon, iPar);
              if (fabs(coefficient) > 1.0e-14) {
                auto childLabel = childLabels[iPar];
                constraint.push_back(std::make_pair(childLabel, coefficient));
                // On the other hand, if the labels do not change, the checksum should not change.
                // In future I should add second checksum and warn user if this happens. It means user has ignored the fact,
                // that the objects already change in the input global tag. The point here is, it still might be reasonable
                // to use the constraint coefficients we already have, as the alignment changes are usually small and do not change
                // the constraint coefficients in first order for most use-cases. The warning should be enough -> TODO
                crc32.process_bytes(&childLabel, sizeof(childLabel));
              }
            }
          }

          //constraints.insert(std::make_pair(parentLabels[iCon], constraint));
          constraints.insert(std::make_pair(crc32.checksum(), constraint));

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

    TMatrixD LorentShiftHierarchy::getLorentzShiftDerivatives(const genfit::StateOnPlane* sop, B2Vector3D bField)
    {
      // values for global derivatives
      //TMatrixD derGlobal(2, 6);
      TMatrixD derGlobal(2, 1);
      derGlobal.Zero();

      // electrons in device go in local w-direction to P-side
      B2Vector3D v = sop->getPlane()->getNormal();
      // Lorentz force (without E-field) direction
      B2Vector3D F_dir = v.Cross(bField);
      // ... projected to sensor coordinates:
      genfit::StateOnPlane localForce = *sop;
      localForce.setPosMom(sop->getPos(), F_dir);
      B2Vector3D lorentzLocal(localForce.getState()[3], localForce.getState()[4], 0); // or 0,1?
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


//       // TODO: constants instead of numbers
//       label.construct(element.first, element.second, 1);
//       labels.push_back(label.label());
//       label.construct(element.first, element.second, 2);
//       labels.push_back(label.label());
//       label.construct(element.first, element.second, 3);
//       labels.push_back(label.label());
//       label.construct(element.first, element.second, 4);
//       labels.push_back(label.label());
//       label.construct(element.first, element.second, 5);
//       labels.push_back(label.label());
//       label.construct(element.first, element.second, 6);
//       labels.push_back(label.label());
//
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
