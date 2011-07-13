/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef RELATION_H
#define RELATION_H

#include <framework/dataobjects/RelationT.h>

namespace Belle2 {

  /** This implementation of Relations is soon to be obsolete. Please use the new Relations system.
   */
  typedef RelationT<UShort_t> Relation;
} // namespace Belle2
#endif // RELATION
