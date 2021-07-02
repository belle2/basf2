/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <map>
#include <set>

namespace Belle2 {

  /**
   * @brief Class to convert to/from global labels
   * for Millepede II to/from detector & parameter
   * identificators.
   * The labels are in following form
   * (the example shows maximal allowed labels):
   *
   *   |TIF|  EID   |PID|
   *   |  0|9999999 |99 |  &  TEID=0 & TID=0, or
   *
   *   |TIF|TID|TEID|PID|
   *   |  1|999|9999|99 |  &  (EID,PID)=dictionary(TEID,PID).
   *
   *
   * where
   * - TIF  is time flag (0 or 1) for time-dependent parameters,
   * - EID  is id of detector element (numeric VxdID or WireID + offset),
   * - PID  is id of alignment/calibration parameter (u/v...),
   * - TEID is time-dependent element index (one per time-dep. parameter)
   * - TID  is time interval id
   */
  class GlobalLabel {

  public:

    typedef unsigned int gidTYPE;              /**< shortcut for main data type (unsigned int) */
    static const gidTYPE maxPID     = 99;      /**< max 99 parameter types  1..99 */
    static const gidTYPE maxEID     = 9999999; /**< max 9.999.999 detector elements 1..9999999 (NOT time-dep-) */
    static const gidTYPE maxTIF     = 1;       /**< time-dep. flag */
    static const gidTYPE maxTEID    = 9999;    /**< max time-dep. parameters  1..9999 */
    static const gidTYPE maxTID     = 999;     /**< max time slices for a parameter 1..999   */
    static const gidTYPE pidOffset  = 1;       /**< parameter number are the last 2 decimal digits */
    static const gidTYPE eidOffest  = pidOffset * (maxPID + 1);  /**< Offset of detector element id = 100 */
    static const gidTYPE tifOffset  = eidOffest * (maxEID + 1);  /**< Offset of time flag = 1.000.000.000 */
    static const gidTYPE teidOffset = eidOffest;                 /**< Offset of time dependent element(detector+parameter) = 100 */
    static const gidTYPE tidOffset  = eidOffest * (maxTEID + 1); /**< Offset of time slice id = 1.000.000 */
    static const gidTYPE maxGID     = pidOffset * maxPID + eidOffest * maxEID + tifOffset *
                                      maxTIF; /**< max internal id = 1.999.999.999 */
    static const gidTYPE maxLabel   =
      maxGID;  /**< Label and internal id ("gid") are the same numbers (label is signed but 0 and <0 values are invalid to give to Pede)*/

    /// Default constuctor. Members initialized in declaration
    GlobalLabel() {}

    /**
     * @brief Constructor from Pede label
     * Depends on registered time dependent parameters
     * if the time flag in the label is non-zero.
     *
     * @param globalLabel The encoded label
     */
    explicit GlobalLabel(gidTYPE globalLabel);

    /**
     * @brief Construct label for given DB object (template argument) and its element and parameter
     *
     * @param element Element id in DB object (wire, sensor etc.)
     * @param param Paremetr id of the element (shift, angle, etc.)
     * @return GlobalLabel
     */
    template<class DBObjType>
    static GlobalLabel construct(gidTYPE element, gidTYPE param)
    {
      GlobalLabel theLabel;
      theLabel.construct(DBObjType::getGlobalUniqueID(), element, param);
      return theLabel;
    }

    /**
     * @brief Construct label for given DB object id and its element and parameter
     *
     * @param dbObjId id of the DB object in global calibration
     * @param element Element id in DB object (wire, sensor etc.)
     * @param param Paremetr id of the element (shift, angle, etc.)
     * @return GlobalLabel
     */
    void construct(gidTYPE dbObjId, gidTYPE element, gidTYPE param)
    {
      if (m_components.empty() or m_components.find(dbObjId) != m_components.end())
        construct(100000 * dbObjId + element, param);
      else
        construct(0, 0);
    }

    /**
     * @brief Set which DB objects have non-zero labels (by their id)
     * @param components set of global ids of DB objects
     */
    static void setComponents(const std::set<unsigned short>& components)
    {
      m_components = components;
    }

    /**
     * @brief Register this Detector element and parameter
     *        as time dependent with instance starting at
     *        "start" time index and ending at "end" index.
     *        Call this for each time interval, in which
     *        the parameter is allowed to have different
     *        value (from that at interval 0-0)
     *
     * @param start Start time of parameter time-dep. instance
     *              (the instance has number "start" from start to end).
     *              The instance number before is 0 if not set.
     * @param end End time of parameter instance.
     *            The number after "end" is 0 if not set.
     */
    void registerTimeDependent(gidTYPE start, gidTYPE end = maxTID);

    /**
     * @brief Usefull setter to quickly change only the parameter id
     *        and return back the encoded label (for use in RecoHits)
     * @param paramId Id of calibration/alignment parameter
     * @return Encoded Pede label with new parameter id
     */
    gidTYPE setParameterId(gidTYPE paramId);

    /**
     * @brief Returns encoded Pede label
     *
     * @return int
     */
    int label() {return gid;}

    /**
     * @brief Cast to encoded Pede label
     */
    operator int() {return (int)label();}

    /**
     * @brief Cast to encoded Pede label
     */
    operator unsigned int() {return (unsigned int)label();}

    /**
     * @brief Returns the global id identifing DB object for constantwith this label
     */
    gidTYPE getUniqueId() const {return eid / 100000;}

    /**
     * @brief Returns the element id (like VxdID for silicon sensors) to identify sets of parameters in DB objects
     */
    gidTYPE getElementId() const {return eid % 100000;}

    //! Get id of alignment/calibration parameter
    gidTYPE getParameterId() const {return pid;}

    //! Get time id
    gidTYPE getTimeId()      const {return tid;}

    //! Is label time-dependent?
    bool    getTimeFlag()    const {return tif;}

    //! Is label valid? (non-zero)
    bool    isValid() {return 0 != gid;}

    //! Dumps the label to std::cout
    void    dump(int level = 0) const;

    //! Get the last time id, where this label is valid
    int     getEndOfValidity()
    {
      if (!tif)
        return maxTID;
      auto it = GlobalLabel::getTimeIntervals().find(makeEIDPID(eid, pid));
      if (it == GlobalLabel::getTimeIntervals().end())
        return tid;
      for (unsigned int i = tid; i < maxTID; i++) {
        if (it->second.get(i) != tid)
          return i - 1;
      }
      return tid;
    }

    /**
     * @brief Forget all previously registered time dependent parameters
     */
    static void clearTimeDependentParamaters();

    //! Returns reference to current time id
    static unsigned int& getCurrentTimeIntervalRef()
    {
      static unsigned int subrun = 0;
      return subrun;
    }

    //! Sets current time id
    //! @param time time id
    static void setCurrentTimeInterval(gidTYPE time)
    {
      gidTYPE& timeref = GlobalLabel::getCurrentTimeIntervalRef();
      timeref = time;
    }

    //! Get current time id
    static gidTYPE getCurrentTimeInterval()
    {
      return GlobalLabel::getCurrentTimeIntervalRef();
    }

  private:
    //! Set of global ids of components for which to return non-zero labels
    static std::set<unsigned short> m_components;

    //! Constructor for any detector
    //! @param elementId Unique id of Belle2 detector element (sensor, layer, wire...)
    //! @param paramId id of the parameter of the element
    void construct(gidTYPE elementId, gidTYPE paramId);

  public:
    //! Struct to hold intervals of validity
    struct TimeInterval {

    private:

      //! Time element id & parameter id
      gidTYPE teidpid_;

      //! Array of time ids
      gidTYPE arr_[maxTID + 1];

    public:

      //! Constructor for given validity interval
      //! @param TEIDPID Time-dependent parameter id
      //! @param start start time id of validity
      //! @param end end time id of validity
      TimeInterval(gidTYPE TEIDPID, gidTYPE start, gidTYPE end)
      {
        teidpid_ = TEIDPID;
        for (gidTYPE i = 0; i <= maxTID; i++) {
          if (i >= start && i <= end)
            arr_[i] = start;
          else
            arr_[i] = 0;
        }
      }

      //! Get composed time element id & param id
      gidTYPE teidpid() {return teidpid_;}

      //! Get the start time id for validity interval valid at given timeid
      gidTYPE get(gidTYPE timeid) {return arr_[timeid];}

      //! Set new parameter validity interval from start to end
      void set(gidTYPE start, gidTYPE end)
      {
        for (gidTYPE i = 0; i <= maxTID; i++) {
          if (i >= start && i <= end)
            arr_[i] = start;
        }
      }
    };
  public:
    //! Reference to map EIDPID -> (TEIDPID, time intervals)
    static std::map<gidTYPE, TimeInterval >& getTimeIntervals()
    {
      // Map EIDPID -> (TEIDPID, time intervals)
      static std::map<gidTYPE, TimeInterval > intervals;
      return intervals;
    }

    //! Reference to dictionary/map TEIDPID -> EIDPID
    static std::map<gidTYPE, gidTYPE>& getDictionary()
    {
      // Map TEIDPID -> EIDPID
      static std::map<gidTYPE, gidTYPE> dictionary;
      return dictionary;
    }

    //! Helper to compose elemnt id & param id
    gidTYPE makeEIDPID(gidTYPE eid_, gidTYPE pid_) {return pid_ * pidOffset + eid_ * eidOffest;}

    //! Helper to compose time elemnt id & param id
    gidTYPE makeTEIDPID(gidTYPE teid_, gidTYPE pid_) {return pid_ * pidOffset + teid_ * teidOffset;}
  private:
    //! global id
    gidTYPE gid {0};

    //! element id
    gidTYPE eid {0};

    //! parameter id
    gidTYPE pid {0};

    //! time id
    gidTYPE tid {0};

    //! time identification flag
    gidTYPE tif {0};

  };

}
