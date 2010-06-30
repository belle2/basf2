/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef RELATION_H
#define RELATION_H

#include <TRef.h>
#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /*! Use the constructor to create a connection between two objects, that are stored in the DataStore.
      \author <a href=mailto:"martin.heck@kit.edu?subject=Relation">Martin Heck</a>
  */
  class Relation : public TObject {
  public:

    //! Constructor for I/O.
    Relation()
        : m_weight(1.0) {}

    //! Constuctor to create the actual relation.
    Relation(TObject* from, TObject* to, const float& weight = 1.0)
        : m_weight(weight) {
      m_from = from;
      m_to   = to;
    }

    //! Destructor.
    ~Relation() {}

    /*! Setter for from.
        \param from Object for "from" end of relation.
    */
    void setFrom(TObject* from) {
      m_from = from;
    }

    /*! Setter for to.
        \param to Object for "to" end of the relation.
    */
    void setTo(TObject* to) {
      m_to = to;
    }

    /*! Getter for from.
        \return Object of "from" end of the relation.
    */
    TObject* getFrom() {
      return m_from.GetObject();
    }

    /*! Getter for to.
        \return Object of "to" end of the relation.
    */
    TObject* getTo() {
      return m_to.GetObject();
    }

    /*! Setter for weight.
        \param weight value for the weight of the relation.
    */
    void setWeight(const float& weight) {
      m_weight = weight;
    }

    /*! Getter for weight.
        \return Weight of relation.
    */
    float getWeight() {
      return m_weight;
    }


  private:
    //! First end of Relation.
    TRef m_from;

    //! Second end of Relation.
    TRef m_to;

    //! Weight of the Relation.
    float m_weight;

    ClassDef(Relation, 1);
  }; //class
} // namespace Belle2
#endif // RELATION
