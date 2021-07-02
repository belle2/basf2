/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/utilities/FormulaParser.h>
#include <stdexcept>
#include <cmath>

namespace Belle2 {

  char FormulaParserBase::operatorToChar(EOperator op) noexcept
  {
    switch (op) {
      case EOperator::c_noop: return ' ';
      case EOperator::c_plus: return '+';
      case EOperator::c_minus: return '-';
      case EOperator::c_multiply: return '*';
      case EOperator::c_divide: return '/';
      case EOperator::c_power: return '^';
      case EOperator::c_roundBracketOpen: return '(';
      case EOperator::c_roundBracketClose: return ')';
      case EOperator::c_squareBracketOpen: return '[';
      case EOperator::c_squareBracketClose: return ']';
    }
    return 0;
  }

  double FormulaParserBase::applyOperator(EOperator op, double a, double b)
  {
    switch (op) {
      case EOperator::c_plus: return a + b;
      case EOperator::c_minus: return a - b;
      case EOperator::c_multiply: return a * b;
      case EOperator::c_divide: return a / b;
      case EOperator::c_power: return std::pow(a, b);
      default: throw std::runtime_error(std::string("Cannot apply operator ") + operatorToChar(op));
    }
    return 0;
  }

  auto FormulaParserBase::checkNumber(ENumberStatus current, char next) -> ENumberStatus {
    switch (current)
    {
      case ENumberStatus::c_Invalid:
        // invalid stays invalid
        return ENumberStatus::c_Invalid;
      case ENumberStatus::c_Empty:
        // numbers are allowed to start with digits, a dot or a sign
        if (std::isdigit(next)) return ENumberStatus::c_Int;
        if (next == '.') return ENumberStatus::c_LeadingDot;
        if (next == '+' or next == '-') return ENumberStatus::c_Sign;
        // everything else we don't like
        return ENumberStatus::c_Invalid;
      case ENumberStatus::c_Sign:
        // if we started with a sign we can only go to digit and dots, no exponent
        if (std::isdigit(next)) return ENumberStatus::c_Int;
        if (next == '.') return ENumberStatus::c_Dot;
        return ENumberStatus::c_Invalid;
      case ENumberStatus::c_Int:
        // So far it's a valid int consisting only of [sign +] digits, next
        // stage is more digits, a . or an exponent
        if (std::isdigit(next)) return ENumberStatus::c_Int;
        if (next == '.') return ENumberStatus::c_Dot;
        if (next == 'E' or next == 'e') return ENumberStatus::c_Exponent;
        return ENumberStatus::c_Invalid;
      case ENumberStatus::c_Dot:
        // After the dot there can be more digits ... or a exponent
        if (std::isdigit(next)) return ENumberStatus::c_Float;
        if (next == 'E' or next == 'e') return ENumberStatus::c_Exponent;
        return ENumberStatus::c_Invalid;
      case ENumberStatus::c_LeadingDot:
        // But if the dot was in the beginning then no exponent
        if (std::isdigit(next)) return ENumberStatus::c_Float;
        return ENumberStatus::c_Invalid;
      // so, we saw some digits after the dot ... more digits or exponent it is
      case ENumberStatus::c_Float:
        if (std::isdigit(next)) return ENumberStatus::c_Float;
        if (next == 'E' or next == 'e') return ENumberStatus::c_Exponent;
        return ENumberStatus::c_Invalid;
      case ENumberStatus::c_Exponent:
        // and for the exponent we need either additional digits or a sign
        if (std::isdigit(next)) return ENumberStatus::c_Scientific;
        if (next == '+' or next == '-') return ENumberStatus::c_ExponentSign;
        return ENumberStatus::c_Invalid;
      case ENumberStatus::c_ExponentSign:
      case ENumberStatus::c_Scientific:
        // and after the exponent sign and any digit thereafter only digits are possible
        if (std::isdigit(next)) return ENumberStatus::c_Scientific;
        return ENumberStatus::c_Invalid;
    }
    return ENumberStatus::c_Invalid;
  }

  void FormulaParserBase::assertOperatorUsable(size_t stacksize)
  {
    // we only have binary operators so we need two operands
    if (stacksize < 1)
      throw std::runtime_error("could not parse, stack of operands empty. Please report, this is most likely a bug");
    if (stacksize < 2)
      throw std::runtime_error("Missing operand");
  }

  void FormulaParserBase::addOperator(EOperator op)
  {
    if (op == EOperator::c_squareBracketOpen or op == EOperator::c_roundBracketOpen) {
      //the last thing we added was a variable so a bracket doesn't make sense
      if (!m_lastTokenWasOperator) throw std::runtime_error("missing operator");
      // otherwise, ont the stack it goes
      m_operatorStack.push(op);
      return;
    }
    if (op == EOperator::c_squareBracketClose or op == EOperator::c_roundBracketClose) {
      // closing bracket. Look for a matching opening bracket and execute all
      // operators until then
      EOperator correct = EOperator::c_roundBracketOpen;
      EOperator wrong = EOperator::c_squareBracketOpen;
      if (op == EOperator::c_squareBracketClose) std::swap(correct, wrong);
      while (!m_operatorStack.empty()) {
        EOperator tok = m_operatorStack.top();
        m_operatorStack.pop();
        if (tok == wrong) throw std::runtime_error("wrong type of closing bracket");
        if (tok == correct) return;
        executeOperator(tok);
      }
      // stack is empty, still no bracket
      throw std::runtime_error("unmatched bracket");
    }

    // Ok, now normal operators: there shouldn't be two in a row
    if (m_lastTokenWasOperator) throw std::runtime_error("missing operand before operator");
    m_lastTokenWasOperator = true;

    // The operator precedence is in the upper 4 bits ... hrhr
    // TODO: make a function for this?
    int op_precedence = (int)op >> 4;
    while (!m_operatorStack.empty()) {
      EOperator tok = m_operatorStack.top();
      // Stop at brackets
      if (tok == EOperator::c_roundBracketOpen or tok == EOperator::c_squareBracketOpen) break;
      int tok_precedence = (int)tok >> 4;
      // Pow operator has right assiocativity, all others are left associative
      // TODO: make nicer?
      bool tok_right = op == EOperator::c_power;
      // If the token has lower precedence or equal precedence but is right associative stop taking tokens
      if (tok_precedence < op_precedence or (tok_precedence == op_precedence and tok_right)) break;
      // otherwise pop and execute
      executeOperator(tok);
      m_operatorStack.pop();
    }
    m_operatorStack.push(op);
  }

  void FormulaParserBase::flushPendingOperators()
  {
    while (!m_operatorStack.empty()) {
      EOperator op = m_operatorStack.top();
      m_operatorStack.pop();
      // found a bracket but no more closing brackets to come ... so error
      if (op == EOperator::c_squareBracketOpen or op == EOperator::c_roundBracketOpen)
        throw std::runtime_error("missing closing bracket");
      executeOperator(op);
    }
  }

  void FormulaParserBase::flushCurrentVariable()
  {
    if (!m_currentVariableName.empty()) {
      if (!m_lastTokenWasOperator) throw std::runtime_error("Missing operator before variable");
      m_lastTokenWasOperator = false;
      // looks like a number, so add a number
      if (m_currentVariableNameNumberStatus != ENumberStatus::c_Invalid) {
        char* ptr;
        double value;
        value = std::strtod(m_currentVariableName.c_str(), &ptr);
        addVariable(InputToken(value));
      } else {
        addVariable(InputToken(m_currentVariableName));
      }
    }
    m_currentVariableName.clear();
    m_currentVariableNameNumberStatus = ENumberStatus::c_Empty;
  }

  auto FormulaParserBase::checkForOperator(char next) -> EOperator {
    if (next == '+' or next == '-')
    {
      // plus and minus are also part of literals so only treat it as operator
      // if, together with the next character, this is not a valid float literal
      auto isvalid = checkNumber(m_currentVariableNameNumberStatus, next);
      if (isvalid != ENumberStatus::c_Invalid and checkNumber(isvalid, m_buffer.peek()) != ENumberStatus::c_Invalid) {
        // this looks like a number don't interpret as operator
        return EOperator::c_noop;
      }
      if (next == '+') return EOperator::c_plus;
      if (next == '-') return EOperator::c_minus;
    }
    if (next == '/') return EOperator::c_divide;
    if (next == '^') return EOperator::c_power;
    if (next == '*')
    {
      // is it python style '**'? if yes, remove one char from stream and
      // assume pow
      if (m_buffer.peek() == '*') {
        m_buffer.get();
        return EOperator::c_power;
      }
      // otherwise multiply
      return EOperator::c_multiply;
    }
    if (next == '(') return EOperator::c_roundBracketOpen;
    if (next == ')') return EOperator::c_roundBracketClose;
    if (next == '[') return EOperator::c_squareBracketOpen;
    if (next == ']') return EOperator::c_squareBracketClose;
    // no operator, so let's return just that
    return EOperator::c_noop;
  }

  void FormulaParserBase::processString(const std::string& formula)
  {
    // initialize buffer
    m_buffer = std::istringstream(formula);
    // clear stacks
    std::stack<EOperator>().swap(m_operatorStack);
    // and an empty identifier name
    m_currentVariableName.clear();
    m_lastTokenWasOperator = true;
    // reset some other variable state
    flushCurrentVariable();
    // and if the variable has arguments remember the nesting level of the ()
    int nestlevel{0};
    // Now loop over the whole formula character by character
    for (char next; m_buffer.get(next);) {
      // If nestlevel>0 we are in a variable(...) parameters area so ignore
      // everything but keep track of how many open/closing brackets we saw
      // until we are back to nestlevel=0
      if (nestlevel > 0) {
        m_currentVariableName += next;
        if (next == '(') ++nestlevel;
        if (next == ')') --nestlevel;
        // finished variable arguments so variable is definitely done
        if (nestlevel == 0) flushCurrentVariable();
        // done with this character
        continue;
      }

      // check for opening parenthesis: could be variable arguments or operation binding
      if (next == '(' and not m_currentVariableName.empty()) {
        m_currentVariableName += next;
        ++nestlevel;
        // definitely not a number anymore
        m_currentVariableNameNumberStatus = ENumberStatus::c_Invalid;
        // done with this character
        continue;
      }

      // check for operator
      auto opcode = checkForOperator(next);
      if (opcode != EOperator::c_noop) {
        // found operator, flush variable, add operator
        flushCurrentVariable();
        addOperator(opcode);
        // done with this character
        continue;
      }

      // check for whitespace
      if (next == ' ' or next == '\n' or next == '\t' or next == '\r') {
        // variable is finished, just flush here.
        flushCurrentVariable();
        // otherwise nothing to do with whitespace ...
        continue;
      }

      // anything else is a identifier, most likely a variable name or a
      // float literal now lets build up the variable name, first lets check
      // if the variable name will still be a valid number
      m_currentVariableNameNumberStatus = checkNumber(m_currentVariableNameNumberStatus, next);
      // then just add it to the state
      m_currentVariableName += next;
    }
    if (nestlevel > 0) throw std::runtime_error("unterminated variable arguments");
    // done with parsing, lets make sure everything is flushed
    flushCurrentVariable();
    flushPendingOperators();
  }

  void FormulaParserBase::raiseError(const std::runtime_error& e)
  {
    // So lets some fun printing the error message :D
    std::ostringstream message;
    // check where we stopped parsing
    auto pos = m_buffer.tellg();
    // -1 -> after the end
    if (pos == -1) pos = m_buffer.str().size() + 1;
    // basic boring message + reason
    message << "Error parsing formula at character " << pos << ": " << e.what() << std::endl;
    // now lets go through the formula again, line by line. YES, multi line formula are a thing
    std::istringstream errbuff(m_buffer.str());
    long lastpos = 0;
    bool arrowShown{false};
    for (std::string line; std::getline(errbuff, line);) {
      // print each line
      message << "   " << line << std::endl;
      // and get the position after each line
      auto curpos = errbuff.tellg();
      // if it's the last line or if we are now beyond the error position then print an arrow
      if (!arrowShown && (curpos == -1 || curpos >= pos)) { // -1 = last line
        // from the beginning to the line to the position of the error
        for (long i = lastpos - 1; i < pos; ++i) message << "-";
        message << "^" << std::endl;
        // but only show it once
        arrowShown = true;
      }
      lastpos = curpos;
    }
    throw std::runtime_error(message.str());
  }
}
