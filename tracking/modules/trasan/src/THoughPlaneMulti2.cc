//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : THoughPlaneMulti2.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent multi Hough parameter planes (version 2)
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------


#include <iostream>

#include "tracking/modules/trasan/Strings.h"
#include "tracking/modules/trasan/THoughPlaneMulti2.h"


namespace Belle {

  THoughPlaneMulti2::THoughPlaneMulti2(const std::string& name,
                                       unsigned nX,
                                       float xMin,
                                       float xMax,
                                       unsigned nY,
                                       float yMin,
                                       float yMax,
                                       unsigned nLayers)
    : THoughPlane(name, nX, xMin, xMax, nY, yMin, yMax),
      _nLayers(nLayers)
  {
//  bzero(_usage, N_LAYERS * sizeof(unsigned));
    for (unsigned i = 0; i < N_LAYERS; i++)
      _usage[i] = false;
    for (unsigned i = 0; i < _nLayers; i++)
      _layers[i] = new THoughPlaneBoolean(name + ":layer" + itostring(i),
                                          nX, xMin, xMax,
                                          nY, yMin, yMax);
    if (nLayers > N_LAYERS)
      std::cout
          << "Too many layers requested(" << _nLayers << ") : "
          << "max #layers = " << N_LAYERS << std::endl;
  }

  THoughPlaneMulti2::~THoughPlaneMulti2()
  {
    for (unsigned i = 0; i < _nLayers; i++)
      delete _layers[i];
  }

  void
  THoughPlaneMulti2::merge(void)
  {
    const unsigned n = _layers[0]->_n;
    for (unsigned i = 0; i < _nLayers; i++) {

      //...Use _usage here...
      if (! _usage[i]) continue;

      for (unsigned j = 0; j < n; j++) {
        if (! _layers[i]->_cell[j]) continue;
        for (unsigned k = 0; k < 32; k++) {
          if ((_layers[i]->_cell[j] >> k) & 1) {
            add(j * 32 + k, 1);
          }
        }
      }
    }
  }

  void
  THoughPlaneMulti2::registerPattern(unsigned layerId, unsigned id)
  {
    _layers[layerId]->registerPattern(id);
  }

} // namespace Belle
