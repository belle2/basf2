/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <string>

namespace Belle2 {
  /**
  *   Contains fractions of dead times measured in a magnet
  */
  class ARICHMagnetTest: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHMagnetTest(): m_serial(""), m_deadTime(), m_dead_lowerA(-1.), m_dead_lowerB(-1.), m_dead_lowerC(-1.), m_dead_lowerD(-1.),
      m_getter(false), m_comment("") {}

    /**
     * Destructor
     */
    ~ARICHMagnetTest() {};

    /**
     * Return HAPD Serial Number
     * @return serial number
     */
    std::string getSerialNumber() const {return m_serial;}

    /**
     * Set HAPD Serial Number
     * @param serial serial number
     */
    void setSerialNumber(const std::string& serial) {m_serial = serial; }

    /**
     * Return dead time fraction of i-th measurement
     * @param i index of the element in the list
     * @return fraction of dead time
     */
    float getDeadTime(unsigned int i) const;

    /**
     * Add a dead time fraction
     * @param dead time fraction
     */
    void appendDeadTime(float deadtime) {m_deadTime.push_back(deadtime); }

    /**
     * Set results of all measurements
     * @param fraction of dead time
     */
    void setDeadTime(const std::vector<float>& deadTime) {m_deadTime = deadTime;}

    /**
     * Return size of the list of dead time fractions
     * @return size
     */
    int getDeadTimeSize() const {return m_deadTime.size();}

    /**
     * Return result of 10V lower voltage at chip A
     * @return fraction of dead time
     */
    float getDeadTimeLowerA() const {return m_dead_lowerA;}

    /**
     * Set result of 10V lower voltage at chip A
     * @param fraction of dead time
     */
    void setDeadTimeLowerA(float dead_lowerA) {m_dead_lowerA = dead_lowerA;}

    /**
     * Return result of 10V lower voltage at chip B
     * @return fraction of dead time
     */
    float getDeadTimeLowerB() const {return m_dead_lowerB;}

    /**
     * Set result of 10V lower voltage at chip B
     * @param fraction of dead time
     */
    void setDeadTimeLowerB(float dead_lowerB) {m_dead_lowerB = dead_lowerB;}

    /**
     * Return result of 10V lower voltage at chip C
     * @return fraction of dead time
     */
    float getDeadTimeLowerC() const {return m_dead_lowerC;}

    /**
     * Set result of 10V lower voltage at chip C
     * @param fraction of dead time
     */
    void setDeadTimeLowerC(float dead_lowerC) {m_dead_lowerC = dead_lowerC;}

    /**
     * Return result of 10V lower voltage at chip D
     * @return fraction of dead time
     */
    float getDeadTimeLowerD() const {return m_dead_lowerD;}

    /**
     * Set result of 10V lower voltage at chip D
     * @param fraction of dead time
     */
    void setDeadTimeLowerD(float dead_lowerD) {m_dead_lowerD = dead_lowerD;}

    /** Set getter reactivation
     * @param getter reactivation
     */
    void setGetter(bool getter) {m_getter = getter; }

    /** Return getter reactivation
     * @return getter reactivation
     */
    bool getGetter() const {return m_getter; }

    /** Set comment
     * @param comment
     */
    void setComment(std::string& comment) {m_comment = comment; }

    /** Return comment
     * @return comment
     */
    std::string getComment() const {return m_comment; }


  private:
    std::string m_serial;           /**< serial number of the sensor */
    std::vector<float> m_deadTime;  /**< fraction of dead time at 1st test */
    float m_dead_lowerA;            /**< fraction of dead time with 10V lower voltage at chip A */
    float m_dead_lowerB;            /**< fraction of dead time with 10V lower voltage at chip B */
    float m_dead_lowerC;            /**< fraction of dead time with 10V lower voltage at chip C */
    float m_dead_lowerD;            /**< fraction of dead time with 10V lower voltage at chip D */
    bool m_getter;                  /**< Getter reactivation */
    std::string m_comment;          /**< Optional comment */


    ClassDef(ARICHMagnetTest, 1); /**< ClassDef */
  };
} // end namespace Belle2

