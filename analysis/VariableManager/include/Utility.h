#pragma once

#include <analysis/VariableManager/Manager.h>

#include <string>
#include <memory>

namespace Belle2 {

  class Particle;

  namespace Variable {

    /**
     * Remove special characters that ROOT dislikes in branch names, e.g. (, ), :, etc
     */
    std::string makeROOTCompatible(std::string str);

    /**
     * Returns position of the matched closing parenthesis if the first character in the given
     * string contains an opening parenthesis. Otherwise return 0.
     */
    unsigned long int findMatchedParenthesis(std::string str, char open = '[', char close = ']');

    /**
     * Split into std::vector on delimiter ignoring delimiters between parenthesis
     */
    std::vector<std::string> splitOnDelimiterAndConserveParenthesis(std::string str, char delimiter, char open, char close);


    /**
     * This class implements a common way to implement cut/selection functionality for particle lists.
     * Every module which wants to perform cuts should use this object.
     * As a parameter the module has to require std::string.
     * This std::string has to be passed as an argument to the static Compile method of the Cut class, which returns a unique_ptr to the Cut object.
     * Cuts can be performed via the check method.
     * In detail:
     *
     * private section of the module:
     *   std::string m_cutParameter;
     *   std::unique_ptr<Variable::Cut> m_cut;
     *
     * constructor of the module:
     *   addParam("cut", m_cutParameter, "Selection criteria to be applied", std::string(""));
     *
     * initialize method of the module:
     *   m_cut = Variable::Cut::Compile(m_cutParameter);
     *
     * event function of the module:
     *   if(m_cut->check(particlePointer)) {
     *     do something
     *   }
     *
     * Valid cuts can contain:
     * 1. Logic conditions: and, or
     * 2. Numeric conditions: <, <=, >, >=, ==, !=
     * 3. Parenthesis []
     * 4. Floats
     * 5. Variables registered in the VariableManager
     *
     * Therefore valid cuts are:
     * 1.2 < M < 1.5
     * daughter0(M) < daughter1(M)
     * [M > 1.5 or M < 0.5] and 0.2 < getExtraInfo(SignalProbability) < 0.7
     *
     * == and != conditions are evaluated not exactly because we deal with floating point values
     * instead two floating point number are equal if their distance in their integral ordering is less than 3.
     */
    class Cut {

    public:
      /**
       * Creates an instance of a cut and returns a unique_ptr to it, if you need a copy-able oject instead
       * you can cast it to a shared_ptr using std::shared_ptr<Variable::Cut>(Cut::Compile(cutString))
       * @param cut the string defining the cut
       * @return std::unique_ptr<Cut>
       */
#if defined(__CINT__) || defined(R__DICTIONARY_FILENAME)
#else
      static std::unique_ptr<Cut> Compile(std::string cut);
#endif
      /**
       * Check if the current cuts are passed by the given particle
       * @param p pointer to the particle object
       */
      bool check(const Particle* p) const;

      /**
       * Print cut tree
       */
      void print() const;


    private:
      /**
       * Constructor of the cut. Call init with given string
       * @param str Cut is initaliszed with the specified cuts. Default are no cuts
       */
      Cut(std::string str);

#if defined(__CINT__) || defined(R__DICTIONARY_FILENAME)
#else
      /**
       * Delete Copy constructor
       */
      Cut(const Cut&) = delete;

      /**
       * Delete assign operator
       */
      Cut& operator=(const Cut&) = delete;
#endif

      /**
       * Preprocess cut string. Trim string and delete global parenthesis
       */
      std::string preprocess(std::string str);

      /**
       * Look for logical conditions in the given cut string
       */
      bool processLogicConditions(std::string str);

      /**
       * Look for numeric binary conditions (e.g. 1.2 < M ) in the given cut string
       */
      bool processBinaryNumericConditions(std::string str);

      /**
       * Look for numeric ternary conditions (e.g. 1.2 < M < 1.5 ) in the given cut string
       */
      bool processTernaryNumericConditions(std::string str);

      /**
       * Returns stored number or Variable value for the given particle
       */
      float get(const Particle* p) const;

      /**
       * Enum with the allowed operations of the Cut Tree
       */
      enum Operation {
        EMPTY = 0,
        NONE,
        AND,
        OR,
        LT,
        LE,
        GT,
        GE,
        EQ,
        NE,
      };

      Operation operation; /**< Operation which connects left and right cut */
      float number; /**< literal number contained in the cut */
      bool isNumeric; /**< if there was a literal number in this cut */
      const Variable::Manager::Var* var; /**< set if there was a valid variable in this cut */
#if defined(__CINT__) || defined(R__DICTIONARY_FILENAME)
#else
      std::unique_ptr<Cut> left; /**< Left-side cut */
      std::unique_ptr<Cut> right; /**< Right-side cut */
#endif
      //NOTE: do not put any data members between the endif and end of class! this would change the class layout in memory
    };
  }
}
