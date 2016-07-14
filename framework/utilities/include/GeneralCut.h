#include <analysis/VariableManager/Manager.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>

#include <string>
#include <vector>
#include <memory>

#include <iostream>
#include <stdexcept>


namespace Belle2 {

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
   * Returns the position of a pattern in a string ignoring everything that is in parenthesis.
   */
  unsigned long int findIgnoringParenthesis(std::string str, std::string pattern, unsigned int begin = 0);

  /**
   * Helper function to test if two floats are almost equal.
   */
  bool almostEqualFloat(const float& a, const float& b);

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
  template <class AVariableManager>
  class GeneralCut {
    typedef typename AVariableManager::Object Object;
    typedef typename AVariableManager::Var Var;

  public:
    /**
     * Creates an instance of a cut and returns a unique_ptr to it, if you need a copy-able oject instead
     * you can cast it to a shared_ptr using std::shared_ptr<Variable::Cut>(Cut::Compile(cutString))
     * @param cut the string defining the cut
     * @return std::unique_ptr<Cut>
     */
    static std::unique_ptr<GeneralCut> Compile(std::string cut)
    {
      return std::unique_ptr<GeneralCut>(new GeneralCut(cut));
    }
    /**
     * Check if the current cuts are passed by the given particle
     * @param p pointer to the particle object
     */
    bool check(const Object* p) const
    {
      switch (m_operation) {
        case EMPTY:
          return true;
        case NONE:
          return this->get(p);
        case AND:
          return m_left->check(p) and m_right->check(p);
        case OR:
          return m_left->check(p) or m_right->check(p);
        case LT:
          return m_left->get(p) < m_right->get(p);
        case LE:
          return m_left->get(p) <= m_right->get(p);
        case GT:
          return m_left->get(p) > m_right->get(p);
        case GE:
          return m_left->get(p) >= m_right->get(p);
        case EQ:
          return almostEqualFloat(m_left->get(p), m_right->get(p));
        case NE:
          return not almostEqualFloat(m_left->get(p), m_right->get(p));
      }
      throw std::runtime_error("Cut string has an invalid format: Invalid operation");
      return false;
    }

    /**
     * Print cut tree
     */
    void print() const
    {
      switch (m_operation) {
        case EMPTY: std::cout << "EMPTY" << std::endl; break;
        case NONE: std::cout << "NONE" << std::endl; break;
        case AND: std::cout << "AND" << std::endl; break;
        case OR: std::cout << "OR" << std::endl; break;
        case LT: std::cout << "LT" << std::endl; break;
        case LE: std::cout << "LE" << std::endl; break;
        case GT: std::cout << "GT" << std::endl; break;
        case GE: std::cout << "GE" << std::endl; break;
        case EQ: std::cout << "EQ" << std::endl; break;
        case NE: std::cout << "NE" << std::endl; break;
      }
      if (m_left != nullptr) {
        std::cout << "Left " << std::endl;
        m_left->print();
        std::cout << "End Left" << std::endl;
      }
      if (m_right != nullptr) {
        std::cout << "Right " << std::endl;
        m_right->print();
        std::cout << "End Right" << std::endl;
      }
    }


  private:
    /**
     * Constructor of the cut. Call init with given string
     * @param str Cut is initalized with the specified cuts. Default are no cuts
     */
    explicit GeneralCut(std::string str)
    {
      boost::algorithm::trim(str);
      if (str.empty()) {
        m_operation = EMPTY;
        return;
      }

      str = preprocess(str);
      if (not processLogicConditions(str)) {
        if (not processTernaryNumericConditions(str)) {
          if (not processBinaryNumericConditions(str)) {
            m_operation = NONE;
            try {
              m_number = Belle2::convertString<float>(str);
              m_isNumeric = true;
            } catch (std::invalid_argument&) {
              m_isNumeric = false;
              processVariable(str);
            }
          }
        }
      }
    }

    /**
     * Delete Copy constructor
     */
    GeneralCut(const GeneralCut&) = delete;

    /**
     * Delete assign operator
     */
    GeneralCut& operator=(const GeneralCut&) = delete;

    /**
     * Preprocess cut string. Trim string and delete global parenthesis
     */
    std::string preprocess(std::string str)
    {
      boost::algorithm::trim(str);
      if (str.empty())
        throw std::runtime_error("Cut string has an invalid format: Empty" + str);

      while (str.size() > 1 and findMatchedParenthesis(str) == str.size() - 1) {
        str = str.substr(1, str.size() - 2);
        boost::algorithm::trim(str);
        if (str.empty())
          throw std::runtime_error("Cut string has an invalid format: Empty" + str);

      }


      return str;
    }

    /**
     * Look for logical conditions in the given cut string
     */
    bool processLogicConditions(std::string str)
    {
      unsigned long int begin = findMatchedParenthesis(str);
      unsigned long int pos = 0;

      if ((pos =  findIgnoringParenthesis(str, " or ", begin)) != std::string::npos) {
        m_operation = OR;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 4)));
        return true;
      }

      if ((pos =  findIgnoringParenthesis(str, " and ", begin)) != std::string::npos) {
        m_operation = AND;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 5)));
        return true;
      }

      return false;
    }

    /**
     * Look for numeric binary conditions (e.g. 1.2 < M ) in the given cut string
     */
    bool processBinaryNumericConditions(std::string str)
    {
      unsigned long int pos = 0;
      if ((pos =  findIgnoringParenthesis(str, "<=")) != std::string::npos) {
        m_operation = LE;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 2)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, "<")) != std::string::npos) {
        m_operation = LT;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 1)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, ">=")) != std::string::npos) {
        m_operation = GE;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 2)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, ">")) != std::string::npos) {
        m_operation = GT;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 1)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, "==")) != std::string::npos) {
        m_operation = EQ;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 2)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, "!=")) != std::string::npos) {
        m_operation = NE;
        m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos)));
        m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos + 2)));
        return true;
      }

      return false;
    }

    /**
     * Look for numeric ternary conditions (e.g. 1.2 < M < 1.5 ) in the given cut string
     */
    bool processTernaryNumericConditions(std::string str)
    {
      for (auto& c : {"<", ">", "!", "="}) {

        unsigned long int pos1 = 0;
        unsigned long int pos2 = 0;

        if (((pos1 =  findIgnoringParenthesis(str, c)) != std::string::npos) and
            ((pos2 =  findIgnoringParenthesis(str, "<", pos1 + 2)) != std::string::npos
             or (pos2 =  findIgnoringParenthesis(str, ">", pos1 + 2)) != std::string::npos
             or (pos2 =  findIgnoringParenthesis(str, "!", pos1 + 2)) != std::string::npos
             or (pos2 =  findIgnoringParenthesis(str, "=", pos1 + 2)) != std::string::npos)) {
          m_operation = AND;
          m_left = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(0, pos2)));
          if (str[pos1 + 1] == '=')
            pos1++;
          m_right = std::unique_ptr<GeneralCut>(new GeneralCut(str.substr(pos1 + 1)));
          return true;
        }
      }

      return false;
    }

    void processVariable(const std::string& str)
    {
      AVariableManager& manager = AVariableManager::Instance();
      var = manager.getVariable(str);
      if (var == nullptr) {
        throw std::runtime_error(
          "Cut string has an invalid format: Variable not found: " + str);
      }
    }

    /**
     * Returns stored number or Variable value for the given particle
     */
    float get(const Object* p) const
    {
      if (m_isNumeric) {
        return m_number;
      } else if (var != nullptr) {
        return var->function(p);
      } else {
        throw std::runtime_error("Cut string has an invalid format: Neither number nor variable name");
      }
    }

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
    } m_operation; /**< Operation which connects left and right cut */
    const Var* var;
    float m_number; /**< literal number contained in the cut */
    bool m_isNumeric; /**< if there was a literal number in this cut */
    std::unique_ptr<GeneralCut> m_left; /**< Left-side cut */
    std::unique_ptr<GeneralCut> m_right; /**< Right-side cut */
  };
}
