/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/utilities/AbstractNodes.h>

namespace Belle2 {

  void injectBooleanOperatorToStream(std::ostream& stream, const BooleanOperator& boperator)
  {
    switch (boperator) {
      case BooleanOperator::AND:
        stream << " and ";
        break;
      case BooleanOperator::OR:
        stream << " or ";
        break;
      case BooleanOperator::NAND:
        stream << " nand ";
        break;
      case BooleanOperator::XOR:
        stream << " xor ";
        break;
      default:
        throw std::runtime_error("Invalid BooleanOperator provided to injectBooleanOperatorToStream.");
    }
  }

  void injectComparisonOperatorToStream(std::ostream& stream, const ComparisonOperator& coperator)
  {
    switch (coperator) {
      case ComparisonOperator::EQUALEQUAL:
        stream << " == ";
        break;
      case ComparisonOperator::GREATEREQUAL:
        stream << " >= ";
        break;
      case ComparisonOperator::LESSEQUAL:
        stream << " <= ";
        break;
      case ComparisonOperator::GREATER:
        stream << " > ";
        break;
      case ComparisonOperator::LESS:
        stream << " < ";
        break;
      case ComparisonOperator::NOTEQUAL:
        stream << " != ";
        break;
      default:
        throw std::runtime_error("Invalid ComparisonOperator provided to injectComparisonOperatorToStream.");
    }
  }

  void injectArithmeticOperatorToStream(std::ostream& stream, const ArithmeticOperation& aoperation)
  {
    switch (aoperation) {
      case ArithmeticOperation::PLUS:
        stream << " + ";
        break;
      case ArithmeticOperation::MINUS:
        stream << " - ";
        break;
      case ArithmeticOperation::PRODUCT:
        stream << " * ";
        break;
      case ArithmeticOperation::DIVISION:
        stream << " / ";
        break;
      case ArithmeticOperation::POWER:
        stream << " ** ";
        break;
      default:
        throw std::runtime_error("Invalid ArithmeticOperator provided to injectArithmeticOperatorToStream.");
    }
  }
}
