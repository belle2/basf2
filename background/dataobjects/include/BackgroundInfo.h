/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/pcore/Mergeable.h>
#include <simulation/dataobjects/SimHitBase.h>
#include <background/dataobjects/BackgroundMetaData.h>
#include <framework/core/FrameworkExceptions.h>
#include <string>
#include <vector>

namespace Belle2 {

  /**
   * This class stores the information about what background was mixed or overlayed.
   */
  class BackgroundInfo: public Mergeable {

  public:

    /**
     * Exception definition
     */
    BELLE2_DEFINE_EXCEPTION(BackgroundInfoNotMergeable,
                            "BackgroundInfo: objects cannot be merged");

    /**
     * enum for methods used to add BG
     */
    enum EMethod {c_Unknown = 0, /**< unknown */
                  c_Mixing  = 1, /**< BG mixing */
                  c_Overlay = 2  /**< BG overlay */
                 };

    /**
     * Structure for background description
     */
    struct BackgroundDescr {
      SimHitBase::BG_TAG tag = SimHitBase::bg_none;  /**< background tag denoting type */
      std::string type; /**< background type */
      BackgroundMetaData::EFileType fileType = BackgroundMetaData::c_Usual; /**< file type */
      std::vector<std::string> fileNames;     /**< file names */
      double realTime = 0;         /**< real time of BG samlpe */
      unsigned numEvents = 0;      /**< number of events (tree entries) in the sample */
      double scaleFactor = 1;      /**< scale factor for the rate */
      double rate = 0;             /**< background rate of the sample */
      unsigned reused = 0;         /**< number of times the sample is reused */
    };

    /**
     * Default constructor.
     */
    BackgroundInfo()
    {}

    /**
     * Destructor.
     */
    ~BackgroundInfo() {}

    /**
     * Set method that is used to add BG
     * @param method enum for method
     */
    void setMethod(EMethod method) {m_method = method;}

    /**
     * Append background description of a sample
     * @param bgDescr description
     * @return index of appended element in std::vector
     */
    unsigned appendBackgroundDescr(const BackgroundDescr& bgDescr)
    {
      m_backgrounds.push_back(bgDescr);
      return m_backgrounds.size() - 1;
    }

    /**
     * Set components included
     * @param components vector of component names
     */
    void setComponents(const std::vector<std::string>& components)
    {
      m_components = components;
    }

    /**
     * Set lower edge of the narrow time window
     * @param minTime lower edge
     */
    void setMinTime(double minTime) {m_minTime = minTime;}

    /**
     * Set upper edge of the narrow time window
     * @param maxTime upper edge
     */
    void setMaxTime(double maxTime) {m_maxTime = maxTime;}

    /**
     * Set lower edge of ECL time window
     * @param minTimeECL lower edge
     */
    void setMinTimeECL(double minTimeECL) {m_minTimeECL = minTimeECL;}

    /**
     * Set upper edge of ECL time window
     * @param maxTimeECL upper edge
     */
    void setMaxTimeECL(double maxTimeECL) {m_maxTimeECL = maxTimeECL;}

    /**
     * Set lower edge of PXD time window
     * @param minTimePXD lower edge
     */
    void setMinTimePXD(double minTimePXD) {m_minTimePXD = minTimePXD;}

    /**
     * Set upper edge of PXD time window
     * @param maxTimePXD upper edge
     */
    void setMaxTimePXD(double maxTimePXD) {m_maxTimePXD = maxTimePXD;}

    /**
     * Set wrap-around flag
     * @param wrapAround flag
     */
    void setWrapAround(bool wrapAround) {m_wrapAround = wrapAround;}

    /**
     * Set maximal alowed energy deposited in ECL to use BG events
     * @param maxEdepECL energy cut [GeV]
     */
    void setMaxEdepECL(double maxEdepECL) {m_maxEdepECL = maxEdepECL;}

    /**
     * Set name that is added to default branch names of background collections
     * Used primarily to pass this name from BGOverlayInput to BGOverlayExecutor module
     * @param name extension name
     */
    void setExtensionName(const std::string& name) {m_extensionName = name;}

    /**
     * Increments sample reused counter
     * @param index element index in std::vector
     */
    void incrementReusedCounter(unsigned index)
    {
      if (index < m_backgrounds.size()) m_backgrounds[index].reused++;
    }

    /**
     * Returns method enum used to add BG
     * @return method
     */
    EMethod getMethod() const {return m_method;}

    /**
     * Returns background descriptions
     * @return descriptions
     */
    const std::vector<BackgroundDescr>& getBackgrounds() const {return m_backgrounds;}

    /**
     * Returns included components
     * @return vector of component names
     */
    const std::vector<std::string>& getComponents() const {return m_components;}

    /**
     * Returns lower edge of the narrow time window
     * @return lower edge
     */
    double getMinTime() const {return m_minTime;}

    /**
     * Returns upper edge of the narrow time window
     * @return upper edge
     */
    double getMaxTime() const {return m_maxTime;}

    /**
     * Returns lower edge of ECL time window
     * @return lower edge
     */
    double getMinTimeECL() const {return m_minTimeECL;}

    /**
     * Returns upper edge of ECL time window
     * @return upper edge
     */
    double getMaxTimeECL() const {return m_maxTimeECL;}

    /**
     * Returns lower edge of PXD time window
     * @return lower edge
     */
    double getMinTimePXD() const {return m_minTimePXD;}

    /**
     * Returns upper edge of PXD time window
     * @return upper edge
     */
    double getMaxTimePXD() const {return m_maxTimePXD;}

    /**
     * Returns wrap-around flag
     * @return flag
     */
    bool getWrapAround() const {return m_wrapAround;}

    /**
     * Returns maximal alowed energy deposited in ECL to use BG events
     * @return energy cut [GeV]
     */
    double getMaxEdepECL() const {return m_maxEdepECL;}

    /**
     * Returns name added to default branch names of background collections
     * Used primarily to pass this name from BGOverlayInput to BGOverlayExecutor module
     * @return extension name
     */
    const std::string& getExtensionName() const {return m_extensionName;}

    /**
     * Implementation of abstract class function
     */
    virtual void merge(const Mergeable* other);

    /**
     * Implementation of abstract class function
     */
    virtual void clear();

    /**
     * Print the info
     */
    void print() const;

  private:

    /**
     * Checks if other object can be merged with this object
     * @param other object to be merged with this object
     * @return true, if can be merged
     */
    bool canBeMerged(const BackgroundInfo* other);

    /**
     * Print info when BG mixing is used
     */
    void printForMixing() const;

    /**
     * Print info when BG overlay is used
     */
    void printForOverlay() const;

    EMethod m_method = c_Unknown; /**< method */
    std::vector<BackgroundDescr> m_backgrounds; /**< background descriptions */
    std::vector<std::string> m_components; /**< detector components included */
    double m_minTime = 0;  /**< minimal time shift of background event */
    double m_maxTime = 0;  /**< maximal time shift of background event */
    double m_minTimeECL = 0;  /**< minimal time shift of background event for ECL */
    double m_maxTimeECL = 0;  /**< maximal time shift of background event for ECL */
    double m_minTimePXD = 0;  /**< minimal time shift of background event for PXD */
    double m_maxTimePXD = 0;  /**< maximal time shift of background event for PXD */
    bool m_wrapAround = false; /**< wrap around events in the tail after maxTime */
    double m_maxEdepECL = 0;  /**< maximal allowed deposited energy in ECL */
    std::string m_extensionName; /**< name added to default branch names of background */

    ClassDef(BackgroundInfo, 4); /**< Class definition */
  };
}


