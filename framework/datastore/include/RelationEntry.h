/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

class TObject;

namespace Belle2 {

  /** Struct for relations.
   *
   *  This class is supposed to be used by the data store to answer queries
   *  for objects that are pointed to by relations.
   */
  struct RelationEntry {
    /** Constructor.
     *
     *  @param obj  The object that is pointed to by the relation.
     *  @param w    The weight of the relation.
     */
    RelationEntry(TObject* obj = nullptr, float w = 1.0): object(obj), weight(w) {}

    TObject* object;  /**< Pointer to the object. */
    float   weight;  /**< Weight of the relation. */
  };
}
