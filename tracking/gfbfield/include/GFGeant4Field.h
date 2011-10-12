/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GFGEANT4FIELD_H
#define GFGEANT4FIELD_H

#include <GFAbsBField.h>
#include <geometry/bfieldmap/BFieldMap.h>


class GFGeant4Field : public GFAbsBField {
public:

  GFGeant4Field(): GFAbsBField(), m_bField(Belle2::BFieldMap::Instance()) {}
  TVector3 get(const TVector3& pos) const;


protected:

  Belle2::BFieldMap& m_bField; /**< Reference to the central magnetic field map.*/

};



#endif /* GFGEANT4FIELD_H */
