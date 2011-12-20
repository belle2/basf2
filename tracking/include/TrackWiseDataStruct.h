/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Moritz Nadler                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <TObject.h>

#ifndef TrackWiseDataStruct_H
#define TrackWiseDataStruct_H
namespace Belle2 {

  struct TrackWiseDataStruct {
  public:
    TrackWiseDataStruct();
    float data;
    ClassDef(TrackWiseDataStruct, 1);
  };

}

#endif
