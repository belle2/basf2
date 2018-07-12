/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rok Pestotnik, Manca Mrvar                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TTimeStamp.h>
#include <string>
#include <arich/dbobjects/ARICHAerogelInfo.h>

namespace Belle2 {
  /**
  *   Map of the Aerogel placement
  */
  class ARICHAerogelMap: public TObject {
  public:
    /**
     * Various constants
     */
    enum {c_layers = 2, /**< number of layers */
         };

    /**
     * Default constructor
     */
    ARICHAerogelMap(): m_id(""), m_ring(0), m_column(0), m_timeStamp(0, 0, 0, kTRUE, 0), m_comment("")
    {
      for (unsigned ii = 0; ii < c_layers; ii++) m_layer[ii] = 0;
    };

    /**
     * Constructor
     */
    ARICHAerogelMap(const std::string& id, int ring, int column, unsigned int* layer, TTimeStamp timeStamp,
                    const std::string& comment): m_id(id),
      m_ring(ring), m_column(column), m_timeStamp(timeStamp), m_comment(comment)
    {
      for (unsigned ii = 0; ii < c_layers; ii++) m_layer[ii] = layer[ii];
    }


    /**
     * Destructor
     */
    ~ARICHAerogelMap() {};

    /** Return Aerogel serial number
     * @return Aerogel serial number
     */
    std::string getAerogelSN() const {return m_id; }

    /** Set Aerogel serial number
     * @param Aerogel serial number
     */
    void setAerogelSN(const std::string& agelid) {m_id = agelid; }

    /**
      * Return Aerogel Ring Identifier
      * @return Aerogel Ring
      */
    int getAerogelRingID() const {return m_ring;}

    /**
     * Set Aerogel Ring Identifier
     * @param Ring Identifier
     */
    void setAerogelRingID(int ring) {m_ring = ring;}

    /**
     * Return Aerogel Column Identifier
     * @return Aerogel Column
     */
    int getAerogelColumnID() const {return m_column;}

    /**
     * Set Aerogel Column Identifier
     * @param Column Identifier
     */
    void setAerogelColumnID(int column) {m_column = column;}

    /**
     * Return Aerogel Layer Identifier
     * @return Aerogel Layer
     */
    unsigned int getAerogelLayer(unsigned i) const { if (i < c_layers) { return m_layer[i]; } return 0; }

    /**
     * Set aerogel info of layer i
     * @param i aerogel layer
     */
    void setAerogelLayer(unsigned int i, unsigned int layer);


    /** Return Aerogel installation date
     * @return Aerogel installation date
     */
    TTimeStamp getAerogelTimeStamp() const {return m_timeStamp; }

    /** Set Aerogel installation date
     * @param Aerogel installation date
     */
    void setTimeStamp(TTimeStamp timeStamp) {m_timeStamp = timeStamp; }

    /** Return comment
     * @return comment
     */
    std::string getAerogelMapComment() const {return m_comment; }

    /** Set comment
     * @param comment
     */
    void setAerogelMapComment(const std::string& comment) {m_comment = comment; }


  private:
    std::string m_id;                     /**< Aerogel Identifier */
    int m_ring;                           /**< Aerogel Ring Identifier */
    int m_column;                         /**< Aerogel Column identifier */
    unsigned int m_layer[c_layers];       /**< Aerogel Layers */
    TTimeStamp m_timeStamp;               /**< Installation Date */
    std::string m_comment;                /**< optional comment */

    ClassDef(ARICHAerogelMap, 2);           /**< ClassDef */
  };
} // end namespace Belle2
