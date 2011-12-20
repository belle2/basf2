/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Moritz Nadler                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/


#ifndef TrackWiseVecDataStruct_H
#define TrackWiseVecDataStruct_H

#include <vector>
#include <TObject.h>
namespace Belle2 {

  struct TrackWiseVecDataStruct {
  public:
    TrackWiseVecDataStruct() {
    }
    TrackWiseVecDataStruct(int n) {
      vecData.resize(n);
    }
    float at(int i) {
      return vecData[i];
    }
    std::vector<float> vecData;
    ClassDef(TrackWiseVecDataStruct, 1);
  };
}
#endif
