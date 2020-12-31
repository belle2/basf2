#pragma once
#include <TObject.h>

#define MERGER_WIDTH 256
#define NUM_MERGER 146
//#define TSF_TO_2D_HALF_WIDTH 219  //210+9
//#define TSF_TO_2D_WIDTH 429     //210*2+9
#define TSF_TO_2D_HALF_WIDTH 324  //315+9
#define TSF_TO_2D_WIDTH 639       //315*2+9
#define NUM_2D 4
#define NUM_TSF 5
#define T2D_TO_3D_WIDTH 747
#define NN_IN_WIDTH 982
#define NN_WIDTH 2048
#define NN_OUT_WIDTH 570
//#define NN_OUT_WIDTH 709


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
    friend class CDCTriggerUnpackerModule;
    friend struct Merger;
    friend struct Tracker2D;
    friend struct Neuro;
  public:
    /** default constructor */
    Bitstream() {};

    /** constructor from Signal Bus */
    explicit Bitstream(SignalBus bus) : m_signal(bus) {}

    /** destructor, empty because we don't allocate memory explicitly. */
    ~Bitstream() { };

    //! accessors
    const SignalBus& signal()
    {
      return m_signal;
    }

  protected:
    //! SignalBus of the Bitstream
    SignalBus m_signal;

    //! Needed to make the ROOT object storable
    ClassDef(Bitstream, 4);
  };

}

