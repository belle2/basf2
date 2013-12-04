/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <genfit/AbsBField.h>
#include <geometry/bfieldmap/BFieldMap.h>

/** Interface of the Belle II B-field with GenFit.
 */
class GFGeant4Field : public genfit::AbsBField {
public:
  GFGeant4Field(): genfit::AbsBField(), m_bField(Belle2::BFieldMap::Instance()) {}
  TVector3 get(const TVector3& pos) const;


protected:
  Belle2::BFieldMap& m_bField; /**< Reference to the central magnetic field map.*/

};

