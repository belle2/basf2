/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/utilities/GeneralCut.h>
#include <skim/softwaretrigger/core/SoftwareTriggerVariableManager.h>

#include <TRandom.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Software Trigger Cut to be used in the Software Trigger Modules.
     * This cut can be down- and uploaded from the database or compiled from a string.
     * It is basically a GeneralCut using the SoftwareTriggerVariableManager with a prescale factor.
     *
     * For database interactions, use the SoftwareTriggerDBHandler.
     */
    class SoftwareTriggerCut {

    public:
      /**
       * Compile a new SoftwareTriggerCut from a cut string (by using the GeneralCut::compile function) and
       * an optional prescale factor. This together with the downloadFromDatabase function is the only possibility to
       * create a new SoftwareTriggerCut.
       * @param cut_string: The string from which the cut should be compiled. Must be in a format, the GeneralCut::compile function understands.
       * @param prescaleFactor: An optional prescale factor which will be used whenever the cut is checked. If the prescale is e.g. 10, the
       *        cut will only result in a true result (although the cut condition itself is true) in 1 of 10 cases on average.
       *        Defaults to 1, which means that the prescale has no impact on the cut check.
       * @return a unique_ptr on a SoftwareTriggerCut with the given cut condition and prescale.
       */
      static std::unique_ptr<SoftwareTriggerCut> compile(const std::string& cut_string, const unsigned int prescaleFactor = 1)
      {
        auto compiledGeneralCut = GeneralCut<SoftwareTriggerVariableManager>::compile(cut_string);
        std::unique_ptr<SoftwareTriggerCut> compiledSoftwareTriggerCut(new SoftwareTriggerCut(std::move(compiledGeneralCut),
            prescaleFactor));

        return compiledSoftwareTriggerCut;
      }

      /**
       * Decompile the internal General Cut back into a string.
       * This function is needed for streaming the SoftwareTriggerCut into the database.
       */
      std::string decompile() const
      {
        return m_cut->decompile();
      }

      /**
       * Main function of the SoftwareTriggerCut: check the cut condition. If it returns true, roll a dice to
       * see if the result after prescaling is still true. In all other cases, return false.
       */
      bool checkPreScaled(const SoftwareTriggerVariableManager::Object& prefilledObject) const
      {
        if (not m_cut) {
          B2FATAL("Software Trigger is not initialized!");
        }
        // First check if the cut gives a positive result. If not, we can definitely return false.
        if (m_cut->check(&prefilledObject)) {
          // if yes, we have to use the prescale factor to see, if the result is really yes.
          return makePreScale();
        }

        return false;
      }

      /**
       * Function to get the prescale factor. See the constructor for a description on what the prescale is.
       */
      unsigned int getPreScaleFactor() const
      {
        return m_preScaleFactor;
      }

    private:
      /// Internal representation of the cut condition as a general cut.
      std::unique_ptr<GeneralCut<SoftwareTriggerVariableManager>> m_cut = nullptr;
      /// Internal variable for the prescale factor.
      unsigned int m_preScaleFactor = 1;

      /**
      * Make constructor private. You should only download a SoftwareCut from the database or compile a new one from a string.
      */
      SoftwareTriggerCut(std::unique_ptr<GeneralCut<SoftwareTriggerVariableManager>>&& cut, const unsigned int prescaleFactor = 1) :
        m_cut(std::move(cut)), m_preScaleFactor(prescaleFactor)
      {
      }

      /**
      * Delete the copy constructor.
      */
      SoftwareTriggerCut(const SoftwareTriggerCut&) = delete;

      /**
      * Delete the assign operator.
      */
      SoftwareTriggerCut& operator=(const SoftwareTriggerCut&) = delete;


      /// Helper function to do a prescaling using a random integer number and the prescaling factor from the object.
      bool makePreScale() const
      {
        // A prescale factor of one is always true...
        if (m_preScaleFactor == 1) {
          return true;
          // ... and a prescale factor of 0 is always false...
        } else if (m_preScaleFactor == 0) {
          return false;
        } else {
          // All other cases are a bit more interesting
          // We do this by drawing a random number between 0 and m_preScaleFactor - 1 and comparing it to 0.
          // The probability to get back a true result is then given by 1/m_preScaleFactor.
          const unsigned int randomNumber = gRandom->Integer(m_preScaleFactor);
          return randomNumber == 0;
        }
      }
    };
  }
}
