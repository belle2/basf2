/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <TObjArray.h>


namespace Belle2 {
  class EventMetaData;

  /**
   * Base class for handling changing conditions during a run.
   *
   * Note that the IntraRunDependency object takes ownership of the individual payload objects by default.
   */
  class IntraRunDependency: public TObject {
  public:

    /**
     * Constructor for intra run depenent conditions.
     * @param object   the first valid object in the run.
     * @param owner    flag that indicates whether the IntraRunDependency takes ownership of the payload objects or not.
     */
    explicit IntraRunDependency(TObject* object = 0, bool owner = true) {m_objects.SetOwner(owner); if (object) m_objects.Add(object);};

    /**
     * Destructor.
     * Virtual because this is a base class.
     */
    virtual ~IntraRunDependency() {};

    /**
     * Get the conditions object that is valid for the given event.
     * @param event   meta data of the event for which we want to have the conditions.
     * @return        object valid for the given event.
     */
    TObject* getObject(const EventMetaData& event) const {return m_objects.At(getIndex(event));};

    /**
     * Get any of the objects. To be used only by the DBStore for type checking.
     * @return        any of the intra-run dependent objects.
     */
    TObject* getAnyObject() const {return m_objects.At(0);};

    /**
     * Flag for ownership.
     * @return        true if the IntraRunDependency owns the individual objects.
     */
    bool isOwner() const {return m_objects.IsOwner();}

  protected:
    TObjArray m_objects;   /**< Array of intra-run dependent objects **/

    /**
     * Get the index of the object that is valid for the given event.
     * @param event   meta data of the event for which we want to have the conditions.
     * @return        array index of the object valid for the given event.
     */
    virtual int getIndex(const EventMetaData& event) const = 0;

    ClassDef(IntraRunDependency, 1);  /**< base class for intra run dependent conditions. */
  };
}
