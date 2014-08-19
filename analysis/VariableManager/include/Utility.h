#pragma once

#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>

#include <string>
#include <iostream>
#include <stdexcept>
#include <map>

namespace Belle2 {

  class Particle;

  namespace Variable {

    /**
    * Remove () from variable names, because ROOT doesn't like them
    */
    std::string makeROOTCompatible(std::string str);

    /**
     * Returns position of the matched closing parenthesis if the first character in the given
     * string contains an opening parenthesis. Otherweise return 0.
     */
    unsigned long int findMatchedParenthesis(std::string str, char open = '[', char close = ']');

    /**
     * Split into std::vector on delimiter ignoring delimiters between parenthesis
     */
    std::vector<std::string> splitOnDelimiterAndConserveParenthesis(std::string str, char delimiter, char open, char close);


    /**
     * This class implements a common way to implement cut/selection functionality for particle lists.
     * Every module which wants to perform cuts should use this object.
     * As a parameter the module has to require Cut::Parameter.
     * This Cut::Parameter has to be passed as an argument to the init function of the Cut object before a cut is performed.
     * Cuts can be performed via the check method.
     * In detail:
     *
     * private section of the module:
     *   Variable::Cut::Parameter m_cutParameter;
     *   Variable::Cut m_cut;
     *
     * constructor of the module:
     *   Variable::Cut::Parameter emptyCut;
     *   addParam("cut", m_cutParameter, "Selection criteria to be applied", emptyCut);
     *
     * init function of the module:
     *   m_cut.init(m_cutParameter);
     *
     * event function of the module:
     *   if(m_cut.check(particlePointer)) {
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
     * == and != conditions are evaluated not exactly because we deal with flaoting point values
     * instead two floating point number are equal if their distance in their integral ordering is less than 3.
     */
    class Cut {

    public:

      /**
       * Typedef to std::string. If we want to change the cut syntax in the future
       * and require another type we can change this type here without touching every module
       */
      typedef std::string Parameter;

      /**
       * Constructor of the cut. Call init with given string
       * @param str Cut is initaliszed with the specified cuts. Default are no cuts
       */
      Cut(Parameter str = "");

      /**
       * Desstructor
       */
      ~Cut();

      /**
       * Initialises Cut object with the given cuts
       * @param str Cut is initaliszed with the specified cuts.
       */
      void init(Parameter str);

      /**
       * Check if the current cuts are passed by the given particle
       * @param p pointer to the particle object
       */
      bool check(const Particle* p);

      /**
       * Print cut tree
       */
      void print();

    private:
      /**
       * Clean the cut object. Delete all former cuts
       */
      void clean();

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
      float get(const Particle* p);

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

      Cut* left; /**< Left-side cut */
      Cut* right; /**< Right-side cut */
      Operation operation; /**< Operation which connects left and right cut */
      float number; /**< literal number contained in the cut */
      bool isNumeric; /**< if there was a literal number in this cut */
      const Variable::Manager::Var* var; /**< set if there was a valid variable in this cut */
    };
  }
}
