/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Moritz Nadler                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/


#ifndef LayerWiseData_H
#define LayerWiseData_H

#include <vector>
#include <TObject.h>
namespace Belle2 {

  struct LayerWiseData {
  public:
    LayerWiseData() {
    }
    LayerWiseData(int nLayers, int nParameters) {
      layerVecData.resize(nLayers);
      for (int l = 0; l != nLayers; ++l) {
        layerVecData[l].resize(nParameters);
      }
    }

    float getElement(int layerIndex, int parameterIndex) {
      return layerVecData[layerIndex][parameterIndex];
    }
    std::vector<std::vector<float> > layerVecData;
    ClassDef(LayerWiseData, 1);
  };
}
#endif
