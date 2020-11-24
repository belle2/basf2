/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca and Fernando Abudinen                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/FlavorTaggerInfoMap.h>

namespace Belle2 {

  // forward declarations

  class FlavorTaggerInfoMap;

  /**
   * This class stores the relevant information for the TagV vertex fit, extracted mainly from the
   * Flavor Tagging Module. It also stores MC true information that could be used to check the goodness
   * of the fit.
   *
   *
   * The way the information is stored is into vectors with the same length. Every position
   * in all vectors correspond to the same event-category information, so that every category
   * is uniquely correlated to one, e.g, track. Variables stored:
   *
   * - Track's probability of being a Btag daughter for every category
   * - Highest probability track's pointer
   * - Track's probability to belong to a given category
   * - MC information obtained in the TagV module
   *
   * This class is still in an early phase, thus some changes may be needed.
   */

  class FlavorTaggerInfo : public RelationsObject {

  public:

    /** Default constructor */
    FlavorTaggerInfo() {};

    /**
    * Saves the usemode of the FlavorTagger
    * @param mode "Teacher" or "Expert"
    */
    void setUseModeFlavorTagger(const std::string& mode) { m_useModeFlavorTagger = mode ;}

    /**
    * Saves the method and the corresponding FlavorTaggerMap
    * @param method "TMVA" or "FANN".
    */
    void addMethodMap(const std::string& method);

    /**
    * usemode of the FlavorTagger
    * @return mode "Teacher" or "Expert"
    */
    std::string getUseModeFlavorTagger() const { return m_useModeFlavorTagger; }

    /** method used for the FlavorTagger and corresponding FlavorTaggerInfoMap
    * @param method "TMVA" or "FANN".
    * @return FlavorTaggerInfoMap with all FlavorTagger Information
    */
    FlavorTaggerInfoMap* getMethodMap(const std::string& method) const;

  private:

    StoreArray<FlavorTaggerInfoMap> m_flavTagInfoMap; /**< StoreArray of FlavorTaggerInfoMaps */
    std::string m_useModeFlavorTagger; /**< Usemode of the FlavorTagger: "Teacher" or "Expert".*/

    /** Map containing the methods used for the FlavorTagger: "TMVA" or "FANN", and the corresponding map.*/
    std::map<std::string, FlavorTaggerInfoMap*> m_methodMap;

    ClassDef(FlavorTaggerInfo, 3) /**< class definition */

  };

} // end namespace Belle2

