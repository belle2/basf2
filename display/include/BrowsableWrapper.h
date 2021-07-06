/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TQObject.h>
#include <TObject.h>

#include <string>
#include <map>

class TBrowser;
class TVirtualPad;

namespace Belle2 {
  /** A wrapper for browsable objects to enable automatic redrawing. */
  class BrowsableWrapper : public TQObject, public TObject {
  public:
    /** Wrap the given object, does NOT take ownership. */
    explicit BrowsableWrapper(TObject* wrapped): m_wrapped(wrapped) { }
    virtual ~BrowsableWrapper() { }

    /** Reimplementation to store additional info on current state. */
    virtual void Browse(TBrowser* b);

    /** Wrap */
    virtual const char* GetIconName() const { return m_wrapped ? m_wrapped->GetIconName() : ""; }
    /** Wrap */
    virtual const char* GetName() const { return m_wrapped ? m_wrapped->GetName() : ""; }

    //not a good idea without making all other functions of wrapped object available :/
    //virtual bool InheritsFrom(const char* classname) const { return m_wrapped?m_wrapped->InheritsFrom(classname):false; }
    //virtual TClass* IsA() const { return m_wrapped?m_wrapped->IsA():TObject::Class(); }

    /** Slot to remove associated pad from central map. */
    void padClosed();

    /** Wrap the given object, does NOT take ownership. */
    void setWrapped(TObject* wrapped) { m_wrapped = wrapped; }

    /** Get list of pads (static). */
    static const std::map<TVirtualPad*, std::string>& getPads() { return s_pads; }

  private:
    TObject* m_wrapped; /**< wrapped object. */
    static std::map<TVirtualPad*, std::string> s_pads; /**< TPad -> name of drawn object. */

    ClassDef(BrowsableWrapper, 0); /**< A wrapper for browsable objects to enable automatic redrawing. */

  };
}
