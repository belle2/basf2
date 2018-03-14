/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GLOBALLABEL_H
#define GLOBALLABEL_H

#include <map>
#include <set>
#include <vxd/dataobjects/VxdID.h>
#include <cdc/dataobjects/WireID.h>
#include <bklm/dataobjects/BKLMElementID.h>
#include <eklm/dataobjects/EKLMElementID.h>

#include <framework/gearbox/Const.h>

namespace Belle2 {

  /// Class to identify beam parameters
  class BeamID {

  public:

    /// Primary vertex x-position
    static const int vertexX = 1;

    /// Primary vertex y-position
    static const int vertexY = 2;

    /// Primary vertex z-position
    static const int vertexZ = 3;

    /// Constructor
    explicit BeamID() {}

    /// convert to int, returns 0 as there are no elements of beam, only parameters
    operator int() {return 0;}

    /// convert to unsigned int, returns 0 as there are no elements of beam, only parameters
    operator unsigned int() {return 0;}

  };

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
    static const gidTYPE vxdOffset  = 1000000;  /**< Offset of 100000 for VXD (VxdID(0) is dummy) */
    static const gidTYPE cdcOffset  = 2000000;  /**< Offset of 200000 in element ids for CDC. WireID(0) is a real wire */
    static const gidTYPE bklmOffset = 3000000;  /**< Offset of 300000 in element ids for BKLM */
    static const gidTYPE eklmOffset = 4000000;  /**< Offset of 400000 in element ids for EKLM */

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
     * @brief Constructor from VxdID (depends on time internally)
     * @param vxdid VxdId of detector element (sensor, layer, ladder)
     * @param paramId Numeric identificator of calibration/alignment parameter
     *                type (U-shift, V-shift, alpha-Rotation, Lorentz-angle etc.).
     */
    GlobalLabel(VxdID vxdid, gidTYPE paramId);

    /**
     * @brief Constructor from WireID (depends on time internally)
     * @param vxdid WireID of detector element (wire, layer?, superlayer?, endplate1?, endplate2?)
     * @param paramId Numeric identificator of calibration/alignment parameter
     *                type (x-wire-shift, y-layer-shift, endplate-Rotation, XT-parameter1 etc.).
     */
    GlobalLabel(WireID cdcid, gidTYPE paramId);

    /**
     * @brief Constructor from BKLMid (depends on time internally)
     * @param bklmid Unique of detector element (wire, layer?, superlayer?, endplate1?, endplate2?)
     * @param paramId Numeric identificator of calibration/alignment parameter
     *                type (x-wire-shift, y-layer-shift, endplate-Rotation, XT-parameter1 etc.).
     */
    GlobalLabel(BKLMElementID bklmid, gidTYPE paramId);

    /**
     * Constructor from EKLMElementID.
     * @param[in] eklmElement EKLM element identifier.
     * @param[in] paramId     Numeric identifier of alignment parameter.
     */
    GlobalLabel(EKLMElementID eklmElement, gidTYPE paramId);

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
      GlobalLabel label;
      label.construct(DBObjType::getGlobalUniqueID(), element, param);
      return label;
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
     * @brief Assignment operator
     */
    GlobalLabel& operator=(const GlobalLabel& rhs) {gid = rhs.gid, eid = rhs.eid, pid = rhs.pid, tid = rhs.tid, tif = rhs.tif; return *this; };

    //! Get the VxdID (returns default if not VXD label)
    VxdID   getVxdID()       const;

    //! Get the WireID (returns default if not CDC label)
    WireID  getWireID()      const;

    //! Get the BklmID (returns 0 if not BKLM label)
    BKLMElementID getBklmID() const;

    /**
     * Get EKLM element identifier. It should be checked that this label is
     * a EKLM label. If this is not the case, then the function fails with
     * fatal error.
     */
    EKLMElementID getEklmID() const;

    //! Is this VXD label?
    bool    isVXD()          const {return (eid >= vxdOffset && eid < cdcOffset);}

    //! Is this CDC label?
    bool    isCDC()          const {return (eid >= cdcOffset && eid < bklmOffset);}

    //! Is this BKLM label?
    bool    isBKLM()         const {return (eid >= bklmOffset && eid < eklmOffset);}

    //! Is this EKLM label?
    bool    isEKLM()         const {return (eid >= eklmOffset && eid < maxEID);}

    //! Is this Beam label?
    bool    isBeam()         const {return (eid > 0 && eid < vxdOffset);}


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
        return 0;
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
    void construct(gidTYPE elementId, gidTYPE paramId);

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

#endif
