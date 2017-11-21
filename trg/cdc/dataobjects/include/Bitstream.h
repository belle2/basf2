#pragma once
#include <TObject.h>

namespace Belle2 {

  /**
   * Class to hold one clock cycle of raw bit content
   *
   * It is intended to be put into a StoreArray, where each element contains the
   * data in one clock cycle.
   *
   * The recommended type of the signal bus is nested std::array<char, N>
   * or std::array<std::bitset, N> (although it can be virtually any container).
   *
   * To add a signal bus with a new type (or dimension), declare the type in linkdef.h, e.g.
   *
   * #pragma link C++ class Belle2::Bitstream<array<array<array<char, 429>, 4>, 5> >+
   *
   * and then it can be used in a module.
   *
   * using namespace Belle2;
   * using std::array;
   * using signalBus = array<array<array<char, 429>, 4>, 5>;
   * StoreArray<Bitstream<signalBus> > bitsFromTSFTo2D;
   *
   */
  template <typename SignalBus>
  class Bitstream : public TObject {
  public:
    /** default constructor */
    Bitstream() {};

    /** constructor from Signal Bus */
    Bitstream(SignalBus bus)
    {
      m_signal = bus;
    };

    /** destructor, empty because we don't allocate memory explicitly. */
    ~Bitstream() { };

    // accessors
    SignalBus signal()
    {
      return m_signal;
    }

  protected:
    SignalBus m_signal;

    //! Needed to make the ROOT object storable
    ClassDef(Bitstream, 1);
  };

}

