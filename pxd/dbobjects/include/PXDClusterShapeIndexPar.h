/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <map>
#include <vector>
#include <string>

namespace Belle2 {


  /** The class for PXD cluster shape index payload
   */

  class PXDClusterShapeIndexPar: public TObject {
  public:
    /** Default constructor */
    PXDClusterShapeIndexPar() {}
    /** Destructor */
    ~PXDClusterShapeIndexPar() {}

    /** Add shape to index  */
    void addShape(const std::string& shape, int index)
    {
      m_shapeIndex[index] = shape;
    }

    /** Returns shape index from shape string */
    int getShapeIndex(const std::string& shape)
    {
      for (auto it = m_shapeIndex.begin(); it != m_shapeIndex.end(); ++it)
        if (it->second == shape)
          return it->first;
      return -1;
    }

    /** Returns shape index from shape string */
    std::string& getShapeString(int index)
    {
      return m_shapeIndex[index];
    }

    /** Returns shape index from shape string */
    const std::map<int, std::string>&  getIndexMap() const
    {
      return m_shapeIndex;
    }

  private:

    std::map<int, std::string> m_shapeIndex;

    ClassDef(PXDClusterShapeIndexPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
