/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <map>
#include <string>


namespace Belle2 {


  /** The class for VTX cluster shape index payload
   */

  class VTXClusterShapeIndexPar: public TObject {
  public:
    /** Default constructor */
    VTXClusterShapeIndexPar() {}
    /** Destructor */
    ~VTXClusterShapeIndexPar() {}

    /** Add shape with name and index  */
    void addShape(const std::string& name, int index)
    {
      m_shapeIndexMap[index] = name;
    }

    /** Returns shape index from name. Returns -1 for invalid name. */
    int getShapeIndex(const std::string& name) const
    {
      for (auto it = m_shapeIndexMap.begin(); it != m_shapeIndexMap.end(); ++it) {
        if (it->second == name)
          return it->first;
      }
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

    ClassDef(VTXClusterShapeIndexPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
