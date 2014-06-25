/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef IWIRETYPE_H
#define IWIRETYPE_H



namespace Belle2 {

  namespace CDCLocalTracking {

    /// The type of the wire ids enumerating wires within a given layer
    typedef signed short IWireType;

    /// Constant making an invalid wire id
    const IWireType INVALID_IWIRE = 32767;

    /// Legacy constant
    const IWireType INVALIDWIRE = INVALID_IWIRE;

  } // namespace CDCLocalTracking

} // namespace Belle2
#endif // WIRETYPE_H
