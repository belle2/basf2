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

    /** Add shape with name and index  */
    void addShape(const std::string& name, int index)
    {
      m_shapeIndexMap[index] = name;
    }

    /** Returns shape index from name. Returns -1 for invalid name. */
    int getShapeIndex(const std::string& name) const
    {
      for (auto it = m_shapeIndexMap.begin(); it != m_shapeIndexMap.end(); ++it)
        if (it->second == name)
          return it->first;
      return -1;
    }

    /** Returns shape name from index. Return 'None' for invalid index */
    const std::string& getShapeName(int index) const
    {
      return m_shapeIndexMap.at(index);
    }

    /** Returns shape index from shape string */
    const std::map<int, std::string>&  getIndexMap() const
    {
      return m_shapeIndexMap;
    }

  private:
    /**  Map of shape names and indexes */
    std::map<int, std::string> m_shapeIndexMap;

    ClassDef(PXDClusterShapeIndexPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
