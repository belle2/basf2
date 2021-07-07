/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <functional>
#include <ios>
#include <TSystem.h>
#include <TROOT.h>
#include <string>

namespace Belle2 {
  /** Simple ScopeGuard to execute a function at the end of the object lifetime.
   *
   * This should be used when you want to make sure some cleanup is performed
   * at the end of the current scope even if you might have multiple possible
   * ways to exit the scope (return statements, exceptions).
   *
   * Optionally you can call `release()` to indicate that cleanup will not be
   * needed after all but the state should remain as it is.
   *
   * For many common use cases there are factory functions to create the
   * correct ScopeGuard object automatically
   *
   * @see guardValue(), guardGetterSetter(), guardFunctor(),
   *      guardDeletion(), guardStreamState(), guardWorkingDirectory()
   */
  class ScopeGuard {
  private:
    /** Function to be called on exit */
    std::function<void()> m_exitfunc;
    /** Indicate whether function should be called */
    bool m_engaged{true};
  public:
    /** Construct a object with a callable function to be called on destruction */
    template<class Callable> explicit ScopeGuard(Callable&& f): m_exitfunc{std::forward<Callable>(f)} {}
    /** No copies */
    ScopeGuard(const ScopeGuard&) = delete;
    /** No move construction */
    ScopeGuard(ScopeGuard&& sg) = delete;
    /** No assignment */
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    /** No move assignment */
    ScopeGuard& operator=(ScopeGuard&&) = delete;
    /** Call function on destruct unless released has been called */
    ~ScopeGuard() { if (m_engaged) m_exitfunc(); }
    /** Release the guard without calling the cleanup function */
    void release() { m_engaged = false; }

    /** Create a ScopeGuard for a value: The content of reference will be copied
     * and reset when the returned object goes out of scope
       \code
       int value{6};
       {
         auto guard = ScopeGuard::guardValue(value);
         value = 45;
         // now value=45
       }
       // now reverted to value=6
       \endcode
     *
     * @warning Do not just call this function and discard the return value.
     *    You have to assign the return value to an object in the current scope
     *    to the scope guard to work as intended.
     *
     * @param reference a reference to a value to be guarded
     */
    template<class T> [[nodiscard]] static ScopeGuard guardValue(T& reference)
    {
      // the [[nodiscard]] will issue warnings when users just call this
      // function and don't assign the ScopeGuard
      const T old = reference;
      // This works even without any assignment or move constructors since
      // C++17 guarantees that this temporary object will be constructed in
      // place and no movement is required (tags: RVO, copy elision)
      return ScopeGuard([&reference, old]() { reference = old; });
    }

    /** Create a ScopeGuard for a value: The content of reference will be copied
     * and reset when the returned object goes out of scope.
     *
     * Will also immediately set a new value.
     *
       \code
       int value{6};
       {
         auto guard = ScopeGuard::guardValue(value, 45);
         // now value=45
       }
       // now reverted to value=6
       \endcode
     *
     * @warning Do not just call this function and discard the return value.
     *    You have to assign the return value to an object in the current scope
     *    to the scope guard to work as intended.
     *
     * @param reference a reference to a value to be guarded
     * @param newValue the new value to set the reference to. Has to be assignable to reference
     */
    template<class T, class V> [[nodiscard]] static ScopeGuard guardValue(T& reference, const V& newValue)
    {
      const T old = reference;
      reference = newValue;
      return ScopeGuard([&reference, old]() { reference = old; });
    }

    /** Create a ScopeGuard from a getter and setter: On construction the getter
     * object is called to obtain a value which is handed to the setter when the
     * guard object goes out of scope
       \code
       {
         auto numberOfEventsGuard = ScopeGuard::guardGetterSetter(
           [](){return Environment::Instance().getNumberEventsOverride();},
           [](unsigned int v){ Environment::Instance().setNumberEventsOverride(v);}
         );
         // when this block is finished the number of events will be reset to its
         // original value
       }
       \endcode
     *
     * @warning Do not just call this function and discard the return value.
     *    You have to assign the return value to an object in the current scope
     *    to the scope guard to work as intended.
     *
     * @param getter a callable object to return a value
     * @param setter a callable object to set a value of the same type
     */
    template<class Getter, class Setter> [[nodiscard]]
    static ScopeGuard guardGetterSetter(Getter getter, Setter setter)
    {
      const auto old = getter();
      return ScopeGuard([setter, old]() { setter(old); });
    }

    /** Create a ScopeGuard from a getter and setter: On construction first the
     * getter is called to get the original value, then the setter is called to
     * set the new value and when the guard object is destructed the setter is
     * called with the original value again.
     *
       \code
       {
         auto numberOfEventsGuard = ScopeGuard::guardGetterSetter(
           [](){return Environment::Instance().getNumberEventsOverride();},
           [](unsigned int v){ Environment::Instance().setNumberEventsOverride(v);}
           56);
         // now the number of events is set to 56 but when this block is finished
         // the number of events will be reset to its original value
       }
       \endcode
     *
     * @warning Do not just call this function and discard the return value.
     *    You have to assign the return value to an object in the current scope
     *    to the scope guard to work as intended.
     *
     * @param getter a callable object to return a value
     * @param setter a callable object to set a value of the same type
     * @param newValue a new value to pass to the setter when this function is called
     */
    template<class Getter, class Setter> [[nodiscard]]
    static ScopeGuard guardGetterSetter(const Getter& getter, Setter setter,
                                        const typename std::result_of<Getter()>::type newValue)
    {
      const auto old = getter();
      setter(newValue);
      return ScopeGuard([setter, old]() { setter(old); });
    }

    /** Create a ScopeGuard from a functor object with appropriately overloaded
     * `operator()` calls to get and set the values. Otherwise it behaves exactly
     * like guardGetterSetter()
     *
     * @warning Do not just call this function and discard the return value.
     *    You have to assign the return value to an object in the current scope
     *    to the scope guard to work as intended.
     *
     * @param functor An object which has a suitable overloaded operator() to be able to
     *                  * get a value ( const T& operator()(); )
     *                  * set a value ( void operator()(const T&); )
     */
    template<class Functor> [[nodiscard]] static ScopeGuard guardFunctor(Functor functor)
    {
      typedef typename std::result_of<Functor()>::type value_type;
      std::function<void(const value_type&)> setter(functor);
      const auto old = functor();
      return ScopeGuard([setter, old] {setter(old);});
    }

    /** Create a ScopeGuard from a functor object with appropriately overloaded
     * `operator()` calls to get and set the values. Otherwise it behaves exactly
     * like guardGetterSetter()
     *
     * @warning Do not just call this function and discard the return value.
     *    You have to assign the return value to an object in the current scope
     *    to the scope guard to work as intended.
     *
     * @param functor An object which has a suitable overloaded operator() to be able to
     *                  * get a value (`const T& operator()();`)
     *                  * set a value (`void operator()(const T&);`)
     * @param newValue a new value to pass to the functor when this function is called
     */
    template<class Functor> [[nodiscard]]
    static ScopeGuard guardFunctor(Functor functor, const typename std::result_of<Functor()>::type& newValue)
    {
      typedef typename std::result_of<Functor()>::type value_type;
      std::function<void(const value_type&)> setter(functor);
      const auto old = functor();
      setter(newValue);
      return ScopeGuard([setter, old] {setter(old);});
    }

    /** Create a ScopeGuard to delete a raw pointer at the end of the scope.
     *
     * Sometimes with ROOT we cannot use `std::unique_ptr` as ROOT wants the
     * address of the pointer to modify it and allocate objects but ROOT still
     * gives us ownership :/.
     *
     * This class is thus a very simplified scope guard which will just delete
     * the pointer on destruction but still allows using a raw pointer so that
     * ROOT can do whatever it does while this guard is alive.
     *
     * @warning Most of the time you don't want this but `std::unique_ptr`
     *    instead. Only use this if you know what you are doing.
     *
     * @warning Do not just call this function and discard the return value.
     *    You have to assign the return value to an object in the current scope
     *    to the scope guard to work as intended.
     *
     * @param pointer reference to a raw pointer to be deleted when the returned
     *                guard object goes out of scope
     */
    template <class T> [[nodiscard]] static ScopeGuard guardDeletion(T*& pointer)
    {
      return ScopeGuard([&pointer]() { delete pointer; pointer = nullptr; });
    }

    /** Create a ScopeGuard for the state of a stream to reset all the formatting at the
     * end of the object lifetime.
     *
       \code
       {
         auto guard = ScopeGuard::guardStreamState(std::cout);
         std::cout << std::setprecision(56) << std::fixed << "Indiana pi: " << 3.2 << std::endl;
       }
       // now the formatting changes are reset
       \endcode
     *
     * @warning Do not just call this function and discard the return value.
     *    You have to assign the return value to an object in the current scope
     *    to the scope guard to work as intended.
     *
     * @param stream stream object to reset
     */
    template<class CharT, class Traits = typename std::char_traits<CharT>> [[nodiscard]]
    static ScopeGuard guardStreamState(std::basic_ios<CharT, Traits>& stream)
    {
      const std::ios::fmtflags flags{stream.flags()};
      const std::streamsize precision{stream.precision()};
      const std::streamsize width{stream.width()};
      const CharT fill{stream.fill()};

      return ScopeGuard([ =, &stream]() {
        stream.flags(flags);
        stream.precision(precision);
        stream.width(width);
        stream.fill(fill);
      });
    }

    /** Create a ScopeGuard of the current working directory.
     *
     * This makes sure that whatever happens to the current working directory
     * we return to the original working directory when the ScopeGuard is
     * destroyed.
     *
       \code
       {
         auto guard = ScopeGuard::guardWorkingDirectory();
         // possibly change directory
       }
       // now back where we were
       \endcode
     *
     * @warning Do not just call this function and discard the return value.
     *    You have to assign the return value to an object in the current scope
     *    to the scope guard to work as intended.
     *
     */
    [[nodiscard]] static ScopeGuard guardWorkingDirectory()
    {
      const std::string old{gSystem->GetWorkingDirectory()};
      return ScopeGuard([old]() {gSystem->ChangeDirectory(old.c_str());});
    }

    /** Create a ScopeGuard of the current working directory and change into a
     * new directory.
     *
     * This makes sure that whatever happens to the current working directory
     * we return to the original working directory when the ScopeGuard is
     * destroyed.
     *
     *
       \code
       {
         auto guard = ScopeGuard::guardWorkingDirectory("/tmp");
         // now in tmp directory
       }
       // now back where we were.
       \endcode
     *
     * @warning Do not just call this function and discard the return value.
     *    You have to assign the return value to an object in the current scope
     *    to the scope guard to work as intended.
     *
     */
    [[nodiscard]] static ScopeGuard guardWorkingDirectory(const std::string& newDirectory)
    {
      const std::string old{gSystem->GetWorkingDirectory()};
      gSystem->ChangeDirectory(newDirectory.c_str());
      return ScopeGuard([old]() {gSystem->ChangeDirectory(old.c_str());});
    }

    /** Create a ScopeGuard to turn ROOT into batch mode and restore the initial
     * batch mode status after the guard object is destroyed.
     *
     * Restoring the initial status is important if your code e..g should run
     * with and without the display module.
     *
     *
       \code
       {
         // the default is to turn on batch mode. You can also pass "false" to turn off
         // the batch mode for the scope of this guard.
         auto guard = ScopeGuard::guardBatchMode();
         // now batch mode is turned on
       }
       // now back to what it was before.
       \endcode
     *
     * @warning Do not just call this function and discard the return value.
     *    You have to assign the return value to an object in the current scope
     *    to the scope guard to work as intended.
     *
     */
    [[nodiscard]] static ScopeGuard guardBatchMode(bool batchMode = true)
    {
      auto getBatchMode = []() {
        return gROOT->IsBatch();
      };

      auto setBatchMode = [](bool batchMode_) {
        gROOT->SetBatch(batchMode_);
      };

      return guardGetterSetter(getBatchMode, setBatchMode, batchMode);
    }
  };
}
