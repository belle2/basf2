/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

#include <map>
#include <string>

namespace Belle2 {

  // forward declarations

  class Track;

  /**
   * This class stores the Flavor Tagger information for a specific method and particle filled in the
   * Flavor Tagger Module.
   *
   *
   * The way the information is stored is into maps for each Level: Track, event and Combiner.
   * there are two maps per level: One saving the pointer to the tracks and the other saving the probabilities.
   * Each category has its own key.
   *
   */

  class FlavorTaggerInfoMap : public RelationsObject {

  public:

    /**
     * Default constructor.
     * All private members are set to 0 (all vectors are empty).
     */
    FlavorTaggerInfoMap():
      m_targetTrackLevel(),
      m_probTrackLevel(),
      m_targetEventLevel(),
      m_probEventLevel(),
      m_qpCategory(),
      m_hasTrueTarget(),
      m_isTrueCategory(),
      m_qrCombined(-2),
      m_B0Probability(-2),
      m_B0barProbability(-2)
    {}

    /**
     * SETTERS
     * The setters are expected to fill a map whose element corresponds to the probabilities and tracks of a
     * given category. The categories are set by the Flavor Tagger.
     * The setting of all elements need to be performed in a loop. For example, setTargetTrackLevel() need to be called
     * once per each category so that the vector is filled progressively. The same holds for all the other
     * setters described as map fillers.
     */

    /**
    * Map filler: Set the category name and the pointer of the track with the highest target track probability
    * for the corresponding category in Event Level.
    * @param category string name of the given category
    * @param track pointer to track object
    */
    void setTargetTrackLevel(const std::string& category, const Belle2::Track* track)
    {
      m_targetTrackLevel.insert(std::pair<std::string, const Track*>(category, track));
    }

    /**
    * Map filler: Set the category name and the corresponding highest target track probability.
    * @param category string name of the given category
    * @param probability highest target track probability
    */
    void setProbTrackLevel(const std::string& category, float probability)
    {
      m_probTrackLevel.insert(std::pair<std::string, float>(category, probability));
    }

    /**
    * Map filler: Set the category name and the pointer of the track with the highest category probability
    * for the corresponding category in Event Level.
    * @param category string name of the given category
    * @param track pointer to track object
    */
    void setTargetEventLevel(const std::string& category, const Belle2::Track* track)
    {
      m_targetEventLevel.insert(std::pair<std::string, const Track*>(category, track));
    }

    /**
    * Map filler: Set the category name and the highest category probability for the corresponding
    * category in Event Level.
    * @param category string name of the given category
    * @param probability highest category probability
    */
    void setProbEventLevel(const std::string& category, float probability)
    {
      m_probEventLevel.insert(std::pair<std::string, float>(category, probability));
    }

    /**
    * Map filler: Set the category name and the corresponding qr Output, i.e. the Combiner input value.
    * They could be used for independent tags.
    * @param category string name of the given category
    * @param qr output of the given category
    */
    void setQpCategory(const std::string& category, float qr)
    {
      m_qpCategory.insert(std::pair<std::string, float>(category, qr));
    }

    /**
    * Map filler: Set the category name and the corresponding MC target truth value.
    * @param category string name of the given category
    * @param isTrue output of the given category
    */
    void setHasTrueTarget(const std::string& category, float isTrue)
    {
      m_hasTrueTarget.insert(std::pair<std::string, float>(category, isTrue));
    }

    /**
    * Map filler: Set the category name and the corresponding truth MC value.
    * @param category string name of the given category
    * @param isTrue output of the given category
    */
    void setIsTrueCategory(const std::string& category, float isTrue)
    {
      m_isTrueCategory.insert(std::pair<std::string, float>(category, isTrue));
    }

    /**
    * Saves qr Output of the Combiner. Output of the FlavorTagger after the complete process.
    * @param qr final FlavorTagger output
    */
    void setQrCombined(float qr) { m_qrCombined = qr; }

    /**
    * Saves the B0Probability output of the Combiner.
    * @param B0Probability probability of being a B0
    */
    void setB0Probability(float B0Probability) { m_B0Probability = B0Probability; }

    /**
    * Saves the B0barProbability output of the Combiner.
    * @param B0barProbability Probability of being a B0bar
    */
    void setB0barProbability(float B0barProbability) { m_B0barProbability = B0barProbability; }

    /**
     * get map containing the category name and the corresponding pointer to the track with highest target probability in Track Level
    * @return map
    */
    std::map<std::string, const Belle2::Track*> getTargetTrackLevel() const { return m_targetTrackLevel; }

    /**
     * get map containing the category name and the corresponding highest target track probability in Track Level
    * @return map
    */
    std::map<std::string, float> getProbTrackLevel() const { return m_probTrackLevel; }

    /**
     * get map containing the category name and the corresponding pointer to the track with highest category probability in Event Level
    * @return map
    */
    std::map<std::string, const Belle2::Track*> getTargetEventLevel() const { return m_targetEventLevel; }

    /**
     * get map containing the category name and the corresponding highest category probability in Event Level
    * @return map
    */
    std::map<std::string, float> getProbEventLevel() const { return m_probEventLevel; }

    /**
     * get map containing the category name and the corresponding qr Output
    * @return map
    */
    std::map<std::string, float> getQpCategory() const { return m_qpCategory; }

    /**
     * get map containing the category truth. 1 if the Category has a target or 0 else.
    * @return map
    */
    std::map<std::string, float> getHasTrueTarget() const { return m_hasTrueTarget; }

    /**
     * get map containing the category truth. 1 if the Category tags the B0 MC flavor correctly 0 else.
     * @return map
     */
    std::map<std::string, float> getIsTrueCategory() const { return m_isTrueCategory; }

    /**
     * qr Output of the Combiner. Output of the FlavorTagger after the complete process.
    * @return qr
    */
    float getQrCombined() const { return m_qrCombined; }

    /**
     * Probability of being a B0. Output of the FlavorTagger after the complete process.
    * @return B0Probability
    */
    float getB0Probability() const { return m_B0Probability; }

    /**
     * Probability of being a B0bar. Output of the FlavorTagger after the complete process.
    * @return B0barProbability
    */
    float getB0barProbability() const { return m_B0barProbability; }

  private:

    // persistent data members

    // Track, Event and Combiner Level Flavor Tagger Info

    std::map<std::string, const Belle2::Track*>
    m_targetTrackLevel; /**< map containing the category name and the corresponding pointer to the track with highest target probability in Track Level*/
    std::map<std::string, float>
    m_probTrackLevel; /**< map containing the category name and the corresponding highest target track probability in Track Level*/
    std::map<std::string, const Belle2::Track*>
    m_targetEventLevel; /**< map containing the category name and the corresponding pointer to the track with highest category probability in Event Level*/
    std::map<std::string, float>
    m_probEventLevel; /**< map containing the category name and the corresponding highest category probability in Event Level*/
    std::map<std::string, float>
    m_qpCategory; /**< map containing the category name and the corresponding qr Output, i.e. the Combiner input value. They could be used for independent tags.*/
    std::map<std::string, float>
    m_hasTrueTarget; /**< map containing the category name and a float value which is 1 if the corresponding category has a target or 0 else.*/
    std::map<std::string, float>
    m_isTrueCategory; /**< map containing the category name and a float value which is 1 if the corresponding category tags the MC Flavor correctly or 0 else.*/

    float m_qrCombined; /**< qr Output of the Combiner. Output of the FlavorTagger after the complete process*/
    float m_B0Probability; /**< Direct Output of the Combiner: Probability of being a B0.*/
    float m_B0barProbability; /**< Direct Output of the Combiner: Probability of being a B0bar).*/

    ClassDef(FlavorTaggerInfoMap, 3) /**< class definition */
  };

} // end namespace Belle2
