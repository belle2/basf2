/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ILAYERTYPE_H
#define ILAYERTYPE_H



namespace Belle2 {

  namespace CDCLocalTracking {

    ///The type of the layer ids enumerating layers within a superlayer
    typedef signed short ILayerType;

    /// Constant making an invalid layer id
    const ILayerType INVALID_ILAYER = 127;

    ///Legacy constant
    const ILayerType INVALIDLAYER = INVALID_ILAYER;

  } // namespace CDCLocalTracking

} // namespace Belle2

#endif // ILAYERTYPE_H
