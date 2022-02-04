##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import argparse
import re
from sly import Lexer, Parser


class B2Lexer(Lexer):
    """
    Class responsible for scanning the cut and generating a stream of tokens.
    The token stream can be passed to `B2Parser` to generate a syntax tree.
    """

    def __init__(self):
        """Initialize Lexer"""
        #: control_token_stack (list): stack for keeping track of seen brackets
        #: and parenthesis. Allows finding parenthesis and bracket syntax
        #: errors on scanner level.
        self.control_token_stack = list()

    #: Set of all tokens
    tokens = {
        # structure tokens
        LBRACK, RBRACK, LPAREN, RPAREN,  # noqa: F821
        # data types
        DOUBLE, INTEGER, IDENTIFIER, BOOLEAN,  # noqa: F821
        LABEL, PARTICLENAME,  # noqa: F821
        # boolean operators
        AND, OR, NOT,  # noqa: F821
        # comparison operatots
        EQUALEQUAL, GREATEREQUAL, LESSEQUAL, GREATER, LESS,  # noqa: F821
        NOTEQUAL,  # noqa: F821
        # arithmetic operators
        POWER, TIMES, DIVIDE, PLUS, MINUS  # noqa: F821
    }

    # Scanning Rules
    #: ignore spaces tabs and newlines
    ignore = " \t\n"

    # Token definitions
    #: comma token definition as literal
    literals = {r","}

    # Comparison operator token definitions
    #: token regular expression for '=='
    EQUALEQUAL = r"=="
    #: token regular expression for '>='
    GREATEREQUAL = r">="
    #: token regular expression for '<='
    LESSEQUAL = r"<="
    #: token regular expression for '>'
    GREATER = r">"
    #: token regular expression for '<'
    LESS = r"<"
    #: token regular expression for '!='
    NOTEQUAL = r"!="

    # Arithmetic operator token definitions
    #: token regular expression for power, both '**' and '^' allowed
    POWER = r"\*\*|\^"
    #: token regular expression for '*'
    TIMES = r"\*"
    #: token regular expression for '/'
    DIVIDE = r"/"
    #: token regular expression for '+'
    PLUS = r"\+"
    #: token regular expression for '-'
    MINUS = r"-"
    #: token regular expression for matching wrapped particle names
    PARTICLENAME = r'".*?"'
    #: token regular expression for particle list labels
    LABEL = r":[a-zA-Z0-9_]*"
    # Scanning Functions for tokens which
    # require additional operations
    # regular expressions are supplied via @_ decorator

    @_(r"\[")  # noqa: F821
    def LBRACK(self, t):
        """
        Scan opening bracket.

        Parameters:
            t (sly.lex.token): token of type LBRACK

        Raises:
            SyntaxError: if no following closing bracket is found
                in the input.

        Side Effect:
            Pushes 'BRACK' onto control_token_stack

        Returns:
            sly.lex.Token
        """
        if "]" not in self.text[self.index:]:
            raise SyntaxError("Unmatched '[' in cut.")
        self.control_token_stack.append("BRACK")
        return t

    @_(r"\]")  # noqa: F821
    def RBRACK(self, t):
        """
        Scan closing bracket.

        Parameters:
            t (sly.lex.token): token of type RBRACK

        Raises:
            SyntaxError: 1. If control_token_stack is empty, which means
                no bracket was opened previously.
                         2. If state of control_token_stack is 'PAREN', which
                means a closing parenthesis is expected.

        Side Effect:
            Pops object from control_token_stack

        Returns:
            sly.lex.Token
        """
        try:
            state = self.control_token_stack.pop()
        except IndexError:  # pop from empty list
            raise SyntaxError("Unmatched ']' in cut.")
        if state == "BRACK":
            return t
        elif state == "PAREN":
            raise SyntaxError("Illegal ']', expected ').")

    @_(r"\(")  # noqa: F821
    def LPAREN(self, t):
        """
        Scan opening parenthesis.

        Parameters:
            t (sly.lex.token): token of type LPAREN

        Raises:
            SyntaxError: if no following closing parenthesis is found
                in the input.

        Side Effect:
            Pushes 'PAREN' onto control_token_stack

        Returns:
            sly.lex.Token
        """
        if ")" not in self.text[self.index:]:
            raise SyntaxError("Unmatched '(")
        self.control_token_stack.append("PAREN")
        return t

    @_(r"\)")  # noqa: F821
    def RPAREN(self, t):
        """
        Scan closing parenthesis.

        Parameters:
            t (sly.lex.token): token of type RPAREN

        Raises:
            SyntaxError: 1. If control_token_stack is empty, which means
                no parenthesis was opened previously.
                         2. If state of control_token_stack is 'BRACK', which
                means a closing bracket is expected.

        Side Effect:
            Pops state from control_token_stack

        Returns:
            sly.lex.Token
        """
        try:
            state = self.control_token_stack.pop()
        except IndexError:  # pop from empty list
            raise SyntaxError("Unmatched ')' in cut.")
        if state == "BRACK":
            raise SyntaxError("Illegal ')', expected ']'.")
        elif state == "PAREN":
            return t

    @_(r"((\d+\.\d*|\d*\.\d+)(e(-|\+)?\d+|E(-|\+)?\d+)?|\d+(e(-|\+)?\d+|E(-|\+)?\d+))")  # noqa: E501, F821
    def DOUBLE(self, t):
        """
        Scanning function for double values

        Parameters:
            t (sly.lex.Token): initial token generated by the scanner library.
                The value attribute is of type str initially, equals
                the matched sequence and is casted to float.

        Possible notations covered by this regular expression:
            Normal decimal notation e.g 0.1
            Hanging decimal seperator notation e.g 1.
            Preceding decimal seperator notation e.g .1
            Scientific notation with (signed) exponents e.g 1.0E4, 1.e-4, .1E+3
            Exponents are case insensitive e.g 1.e4, 1.E4
            Integer with exponent e.g 1E4

        Returns:
            sly.lex.Token
        """
        t.value = float(t.value)
        return t

    @_(r"(0(x|X)[0-9A-Fa-f]+)|\d+")  # noqa: F821
    def INTEGER(self, t):
        """
        Scanning function for integer values
        Allows normal and hex notation (case insensitive)

        Parameters:
            t (sly.lex.Token): initial token generated by the scanner library.
                The value attribute is of type str initially, equals
                the matched sequence and is casted to int.

        Warning:
            python int-objects are converted
            to the standard c++ int datatype (32bit).
            Overflows can happen because numerical limits
            of python int and c++ int datatypes differ.
            If you need to input large values write it as double.

        Returns:
            sly.lex.Token
        """
        try:
            t.value = int(t.value)
        except ValueError:
            # casting hex notation
            t.value = int(t.value, base=16)
        return t

    @_(r"[a-zA-Z][a-zA-Z_0-9&]*")  # noqa: F821
    def IDENTIFIER(self, t):
        """
        Scaning function for identifiers

        If a matched sequence equals reserved keywords of other tokens
        the token type and value is remapped via the reserved dictionary.

        Parameters:
            t (sly.lex.Token): initial token generated by the scanner library.
                value attribute equals the matched sequence.

        Returns:
            sly.lex.Token
        """
        reserved = {
            "and": "AND",
            "or": "OR",
            "not": "NOT",
            "True": "BOOLEAN",
            "true": "BOOLEAN",
            "False": "BOOLEAN",
            "false": "BOOLEAN",
            "nan": "DOUBLE",
            "infinity": "DOUBLE",
            "inf": "DOUBLE",
        }
        # Check for reserved words
        t.type = reserved.get(t.value, "IDENTIFIER")

        # Set value to bool if BOOLEAN type was returned from reserved dict.
        if t.type == "BOOLEAN":
            t.value = t.value == "True" or t.value == "true"
        # Take care of special infinity and nan values.
        if t.type == "DOUBLE":
            t.value = float(t.value)
        return t


def parser_class_decorator(cls, parser_type):
    """
    Class decorator which allows creating a Parser class object
    for the B2Parser and B2ExpressionParser without repeating the class body.

    Args:
        parser_type (str): choice of parser type, 'cut' or 'expression'

    Returns:
        (type): returns a parser class object
    """
    assert parser_type in (
        "cut",
        "expression",
    ), "Invalid parser type, valid choices are 'cut' or 'expression'"

    class B2ParserMixin(cls):
        """
        Parser class implementing the grammar specified below.

        Full Grammar Specification:
        <cut> ::= EMPTY
            | <boolean_expression>

        <boolean_expression> ::= <disjunction>

        <disjunction> ::= <conjunction>
            | <disjunction> OR <conjunction>

        <conjunction> ::= <negation>
            | <conjunction> AND <negation>

        <negation> ::= <bracket_expression>
            | NOT <negation>

        <bracket_expression> ::= <relational_expression>
            | LBRACK <boolean_expression> RBRACK

        <relational_expression> ::= <expression>
            | <expression> <comparison_operator> <expression>
            | <expression> <comparison_operator> <expression>
              <comparison_operator> <expression>

        <comparison_operator> ::= EQUALEQUAL
            | GREATER
            | LESS
            | GREATEREQUAL
            | LESSEQUAL
            | NOTEQUAL

        <expression> ::= <sum>

        <sum> ::= <term>
            | <sum> PLUS <term>
            | <sum> MINUS <term>

        <term> ::= <factor>
            | <term> TIMES <factor>
            | <term> DIVIDE <factor>

        <factor> ::= <power>
            | PLUS <factor>
            | MINUS <factor>

        <power> ::= <primary>
            | <primary> POWER <factor>

        <primary> ::= LPAREN <expression> RPAREN
            | <function>
            | <generalidentifier>
            | INTEGER
            | BOOLEAN
            | DOUBLE

        <generalidentifier> ::= IDENTIFIER
            | PARTICLENAME
            | IDENTIFIER LABEL
            | PARTICLENAME LABEL

        <function> ::= IDENTIFIER LPAREN <parameters> RPAREN

        <parameters> ::= EMPTY
            | <parameter_list>

        <parameter_list> ::= <parameter>
            | <parameter_list> COMMA <parameter>

        <parameter> ::= <boolean_expression>
        """

        def __init__(self, verbose=False):
            """
            Initialize Parser
            @param verbose  run parser in verbose mode. The nodetype names in
                            the parsed tuple are written out and not encoded
                            as integers. Useful for debugging parsing errors.
            """
            super().__init__()
            #: verbose setting, creates more human readable tuple output
            #: only for testing, debugging purposes
            #: not used in production, as default of kwarg is False
            self.verbose = verbose
            #: parameter state stack
            #: used for scope detection of variables and metavariables
            self.parameter_stack = list()

        #: token list from B2Lexer, this is required
        tokens = B2Lexer.tokens
        start = parser_type
        #: Define precedence of operators starting with lowest precedence
        #: first element of tuple indicates associativity of operator
        precedence = (  # noqa: F841
            ("left", "OR"),
            ("left", "AND"),
            ("nonassoc", "NOT"),
            ("left", "EQUALEQUAL", "GREATER", "LESS",
             "GREATEREQUAL", "LESSEQUAL", "NOTEQUAL"),
            ("left", "PLUS", "MINUS"),
            ("left", "TIMES", "DIVIDE"),
            ("right", "POWER"),
        )

        #: Dict for encoding nodetypes to integers
        #: Must match enum in framework/utilities/AbstractNodes.h
        node_types = {
            "UnaryBooleanNode": 0,
            "BinaryBooleanNode": 1,
            "UnaryRelationalNode": 2,
            "BinaryRelationalNode": 3,
            "TernaryRelationalNode": 4,
            "UnaryExpressionNode": 5,
            "BinaryExpressionNode": 6,
            "FunctionNode": 7,
            "IdentifierNode": 8,
            "DoubleNode": 9,
            "IntegerNode": 10,
            "BooleanNode": 11,
        }

        #: Dict for encoding boolean operator types to integers
        #: Must match BooleanOperator enum in framework/utilities/AbstractNodes.h  # noqa: E501
        b_operator_types = {
            "and": 0,
            "or": 1,
        }

        #: Dict for encoding comparison operator types to integers
        #: Must match ComparisonOperator enum in framework/utilities/AbstractNodes.h  # noqa: E501
        c_operator_types = {
            "==": 0,
            ">=": 1,
            "<=": 2,
            ">": 3,
            "<": 4,
            "!=": 5,
        }

        #: Dict for encoding arithmetic operator types to integers
        #: Must match ArithmeticOperator enum in framework/utilities/AbstractNodes.h  # noqa: E501
        a_operation_types = {
            "+": 0,
            "-": 1,
            "*": 2,
            "/": 3,
            "**": 4,
            "^": 4
        }

        def get_node_type(self, node_name: str):
            """
            Return the node type integer value
            or node name if verbose setting is chosen.
            """
            return node_name if self.verbose else self.node_types[node_name]

        def get_coper_type(self, coper_name: str):
            """
            Return the comparison operator type integer value
            or comparison operator name if verbose setting is chosen.
            """
            return coper_name if self.verbose else self.c_operator_types[coper_name]  # noqa: E501

        def get_boper_type(self, boper_name: str):
            """
            Return the boolean operator type integer value
            or boolean operator name if verbose setting is chosen.
            """
            return boper_name if self.verbose else self.b_operator_types[boper_name]  # noqa: E501

        def get_a_operation_type(self, operation_name: str):
            """
            Return the arithmetic operator type integer value
            or arithmetic operator token if verbose setting is chosen.
            """
            return operation_name if self.verbose else self.a_operation_types[operation_name]  # noqa: E501

        if parser_type == "cut":
            @_(r"", r"boolean_expression",)  # noqa: F821
            def cut(self, p):
                """
                Parsing function for <cut> nonterminal

                Grammar rules:
                    <cut> ::= EMPTY
                        | <boolean_expression>
                """
                try:
                    return p.boolean_expression
                except AttributeError:
                    return (
                        self.get_node_type("UnaryRelationalNode"),
                        (
                            self.get_node_type("BooleanNode"),
                            True
                        )
                    )

        @_(r"disjunction")  # noqa: F821
        def boolean_expression(self, p):
            """
            Parsing function for <boolean_expression> nonterminal

            Grammar rule:
                <boolean_expression> ::= <disjunction>
            """
            return p.disjunction

        @_(r"disjunction OR conjunction", r"conjunction")  # noqa: F821
        def disjunction(self, p):
            """
            Parsing function for <disjunction> nonterminal

            Grammar rules:
                <disjunction> ::= <conjunction>
                    | <disjunction> OR <conjunction>
            """
            try:
                return (
                    self.get_node_type("BinaryBooleanNode"),
                    p.disjunction,
                    p.conjunction,
                    self.get_boper_type(p.OR),
                )
            except AttributeError:
                return p.conjunction

        @_(r"conjunction AND negation", r"negation")  # noqa: F821
        def conjunction(self, p):
            """
            Parsing function for <conjunction> nonterminal

            Grammar rules:
                <conjunction> ::= <negation>
                    | <conjunction> AND <negation>
            """
            try:
                return (
                    self.get_node_type("BinaryBooleanNode"),
                    p.conjunction,
                    p.negation,
                    self.get_boper_type(p.AND),
                )
            except AttributeError:
                return p.negation

        @_(r"bracket_expression", r"NOT negation")  # noqa: F821
        def negation(self, p):
            """
            Parsing function for <negation> nonterminal

            Grammar rules:
                <negation> ::= <bracket_expression>
                    | NOT <negation>
            """
            try:
                return p.bracket_expression
            except AttributeError:
                return (
                    self.get_node_type("UnaryBooleanNode"),
                    p.negation,
                    True,
                    False,
                )

        @_(  # noqa: F821
            r"relational_expression",
            r"LBRACK boolean_expression RBRACK")
        def bracket_expression(self, p):
            """
            Parsing function for <bracket_expression> nonterminal

            Grammar rules:
                <bracket_expression> ::= <relational_expression>
                    | LBRACK <boolean_expression> RBRACK
            """
            try:
                return p.relational_expression
            except AttributeError:
                return (
                    self.get_node_type("UnaryBooleanNode"),
                    p.boolean_expression,
                    False,
                    True,
                )

        @_(r"expression")  # noqa: F821
        def relational_expression(self, p):  # noqa: F811
            """
            Parsing function for <relational_expression> nonterminal

            Grammar rule:
                <relational_expression> ::= <expression>
            """

            return (self.get_node_type("UnaryRelationalNode"), p.expression)

        @_(r"expression comparison_operator expression")  # noqa: F821
        def relational_expression(self, p):  # noqa: F811
            """
            Parsing function for <relational_expression> nonterminal

            Grammar rule:
                <relational_expression> ::= <expression> <comparison_operator>
                                            <expression>
            """
            return (
                self.get_node_type("BinaryRelationalNode"),
                p.expression0,
                p.expression1,
                self.get_coper_type(p.comparison_operator),
            )

        @_(r"expression comparison_operator expression comparison_operator expression")  # noqa: F821, E501
        def relational_expression(self, p):  # noqa: F811
            """
            Parsing function for <relational_expression> nonterminal

            Grammar rule:
                <relational_expression> ::= expression> <comparison_operator>
                    <expression> <comparison_operator> <expression>
            """
            return (
                self.get_node_type("TernaryRelationalNode"),
                p.expression0,
                p.expression1,
                p.expression2,
                self.get_coper_type(p.comparison_operator0),
                self.get_coper_type(p.comparison_operator1),
            )

        @_(  # noqa: F821
            r"EQUALEQUAL",
            r"GREATER",
            r"LESS",
            r"GREATEREQUAL",
            r"LESSEQUAL",
            r"NOTEQUAL",
        )
        def comparison_operator(self, p):
            """
            Parsing function for <comparison_operator> nonterminal

            Grammar rules:
                <comparison_operator> ::= EQUALEQUAL
                    | GREATER
                    | LESS
                    | GREATEREQUAL
                    | LESSEQUAL
                    | NOTEQUAL
            """
            return p[0]

        @_(r"sum")  # noqa: F821
        def expression(self, p):
            """
            Parsing function for <expression> nonterminal

            Grammar rule:
                <expression> ::= <sum>
            """
            return p.sum

        @_(r"sum PLUS term", r"sum MINUS term", r"term")  # noqa: F821
        def sum(self, p):
            """
            Parsing function for <sum> nonterminal

            Grammar rules:
                <sum> ::= <term>
                    | <sum> PLUS <term>
                    | <sum> MINUS <term>
            """
            try:
                return (
                    self.get_node_type("BinaryExpressionNode"),
                    p.sum,
                    p.term,
                    self.get_a_operation_type(p[1]),
                )
            except AttributeError:
                return p.term

        @_(r"term TIMES factor", r"term DIVIDE factor", r"factor")  # noqa: F821, E501
        def term(self, p):
            """
            Parsing function for <term> nonterminal

            Grammar rules:
                <term> ::= <factor>
                    | <term> TIMES <factor>
                    | <term> DIVIDE <factor>
            """
            try:
                return (
                    self.get_node_type("BinaryExpressionNode"),
                    p.term,
                    p.factor,
                    self.get_a_operation_type(p[1]),
                )
            except AttributeError:
                return p.factor

        @_(r"power")  # noqa: F821
        def factor(self, p):
            """
            Parsing function for <power> nonterminal

            Grammar rule:
                <factor> ::= <power>
            """
            return p.power

        @_(r"PLUS factor")  # noqa: F821
        def factor(self, p):  # noqa: F811
            """
            Parsing function for <factor> nonterminal

            Grammar rules:
                <factor> ::= PLUS <factor>
            """
            return (
                self.get_node_type("UnaryExpressionNode"),
                p.factor,
                False,
                False,
            )

        @_(r"MINUS factor")  # noqa: F821
        def factor(self, p):  # noqa: F811
            """
            Parsing function for <factor> nonterminal

            Grammar rule:
                <factor> ::= MINUS factor
            """
            return (
                self.get_node_type("UnaryExpressionNode"),
                p.factor,
                True,
                False,
            )

        @_(r"primary")  # noqa: F821
        def power(self, p):
            """
            Parsing function for <power> nonterminal

            Grammar rule:
                <power> ::= <primary>
            """
            return p.primary

        @_(r"primary POWER factor")  # noqa: F821
        def power(self, p):  # noqa: F811
            """
            Parsing function for <power> nonterminal

            Grammar rule:
                <power> ::= <primary> POWER <factor>
            """
            return (
                self.get_node_type("BinaryExpressionNode"),
                p.primary,
                p.factor,
                self.get_a_operation_type(p.POWER),
            )

        @_(r"function")  # noqa: F821
        def primary(self, p):
            """
            Parsing function for <primary> nonterminal

            Grammar rule:
                <primary> ::= <function>
            """
            return p.function

        @_(r"LPAREN expression RPAREN")  # noqa: F821
        def primary(self, p):  # noqa: F811
            """
            Parsing function for <primary> nonterminal

            Grammar rule:
                <primary> ::= LPAREN <expression> RPAREN
            """
            return (
                self.get_node_type("UnaryExpressionNode"),
                p.expression,
                False,
                True,
            )

        @_(r"INTEGER")  # noqa: F821
        def primary(self, p):  # noqa: F811
            """
            Parsing function for <primary> nonterminal

            Grammar rule:
                <primary> ::= INTEGER
            """
            return (self.get_node_type("IntegerNode"), p.INTEGER)

        @_(r"DOUBLE")  # noqa: F821
        def primary(self, p):  # noqa: F811
            """
            Parsing function for <primary> nonterminal

            Grammar rule:
                <primary> ::= DOUBLE
            """
            return (self.get_node_type("DoubleNode"), p.DOUBLE)

        @_(r"BOOLEAN")  # noqa: F821
        def primary(self, p):  # noqa: F811
            """
            Parsing function for <primary> nonterminal

            Grammar rule:
                <primary> ::= BOOLEAN
            """
            return (self.get_node_type("BooleanNode"), p.BOOLEAN)

        @_(r"generalidentifier")  # noqa: F821
        def primary(self, p):  # noqa: F811
            """
            Parsing function for <primary> nonterminal

            Grammar rule:
                <primary> ::= IDENTIFIER
            """
            if self.parameter_stack:
                return (
                    self.get_node_type("IdentifierNode"),
                    p.generalidentifier,
                    False,
                )
            else:
                return (
                    self.get_node_type("IdentifierNode"),
                    p.generalidentifier,
                    True,
                )

        @_(r"IDENTIFIER")  # noqa: F821
        def generalidentifier(self, p):  # noqa: F811
            """
            Parsing function for <generalidentifier> nonterminal

            Grammar rule:
                <generalidentifier> ::= IDENTIFIER
            """
            return p.IDENTIFIER

        @_(r"IDENTIFIER LABEL")  # noqa: F821
        def generalidentifier(self, p):  # noqa: F811
            """
            Parsing function for <generalidentifier> nonterminal

            Grammar rule:
                <generalidentifier> ::= IDENTIFIER LABEL
            """
            return p.IDENTIFIER + p.LABEL

        @_(r"PARTICLENAME LABEL")  # noqa: F821
        def generalidentifier(self, p):  # noqa: F811
            """
            Parsing function for <generalidentifier> nonterminal

            Grammar rule:
                <generalidentifier> ::= PARTICLENAME LABEL
            """
            return p.PARTICLENAME.replace('"', '')+p.LABEL

        @_(r"PARTICLENAME")  # noqa: F821
        def generalidentifier(self, p):  # noqa: F811
            """
            Parsing function for <generalidentifier> nonterminal

            Grammar rule:
                <generalidentifier> ::= PARTICLENAME
            """
            return p.PARTICLENAME.replace('"', '')

        @_(r"IDENTIFIER LPAREN parameters RPAREN")  # noqa: F821
        def function(self, p):
            """
            Parsing function for <function> nonterminal

            Grammar rule:
                <function> ::= IDENTIFIER LPAREN <parameters> RPAREN
            """
            if self.parameter_stack:
                return (
                    self.get_node_type("FunctionNode"),
                    p.IDENTIFIER,
                    len(p.parameters),
                    p.parameters,
                    False,
                )
            else:
                return (
                    self.get_node_type("FunctionNode"),
                    p.IDENTIFIER,
                    len(p.parameters),
                    p.parameters,
                    True,
                )

        @_(r"", r"parameter_list")  # noqa: F821
        def parameters(self, p):
            """
            Parsing function for <parameters> nonterminal

            Grammar rules:
                <parameters> ::= EMPTY
                    | <parameter_list>
            """
            try:
                return p.parameter_list
            except AttributeError:
                return tuple()

        @_(r"parameter", r"parameter_list ',' parameter")  # noqa: F821
        def parameter_list(self, p):
            """
            Parsing function for <parameter_list> nonterminal

            Grammar rules:
                <parameter_list> ::= <parameter>
                    | <parameter_list> COMMA <parameter>
            """
            try:
                return p.parameter_list + (p.parameter,)
            except AttributeError:
                return (p.parameter,)

        @_(r"parameter_begin_scope boolean_expression parameter_end_scope")  # noqa: F821, E501
        def parameter(self, p):
            """
            Parsing function for <parameter> nonterminal

            Grammar rules:
                <parameter> ::= <boolean_expression>
            """
            return p.boolean_expression

        @_("")  # noqa: F821
        def parameter_begin_scope(self, p):
            """
            Parsing action to open parameter scope
            """
            self.parameter_stack.append(None)

        @_("")  # noqa: F821
        def parameter_end_scope(self, p):
            """
            Parsing action to close parameter scope
            """
            self.parameter_stack.pop()

        def error(self, p):
            """
            Error function, called immediately if syntax error is detected
            @param p (sly.token)    offending token p
                                    p is None if syntax error occurs at EOF.
            """
            try:
                # Get error position of offending token in cut.
                error_pos = p.index
            except AttributeError:  # syntax error at EOF, p is None
                # Set error position to length of cut minus one.
                error_pos = len(self.cut) - 1
            try:
                # Get error token type
                error_token = p.type
            except AttributeError:
                # syntax error at EOF get last token from stack
                error_token = self.symstack[-1].type

            # Format error message
            error_msg = f"detected at:\n{self.cut}\n{' '*error_pos}^\n"
            error_msg += f"Unexpected token '{error_token}'"
            raise SyntaxError(error_msg)

        def parse(self, cut: str, token_generator) -> tuple:
            """
            Overwrite sly.Parser parse function.
            @param cut              unparsed cut input which is used to
                                    indicate where the error occurred
            @param token_generator  generator object which yields tokens.
                                    Produced by the lexer from the cut input.
            """
            #: Set cut attribute needed in case of an error.
            self.cut = cut
            return super().parse(token_generator)

    return B2ParserMixin


B2Parser = parser_class_decorator(Parser, parser_type="cut")

B2ExpressionParser = parser_class_decorator(Parser, parser_type="expression")


def preprocess(cut: str) -> str:
    """
    Some particle names interfere with the grammar and the scanning.
    They have to be matched in the cut and wrapped in
    double quotes before tokenization.
    """
    def wrap_particle_name(matchobj):
        particle_name = matchobj.group(1)
        matched_string = matchobj.group(0)
        return matched_string.replace(particle_name, f'"{particle_name}"')

    particle_name_pattern = r"[\(,]\s*(?P<ParticleName>anti-Lambda_c\(2593\)-|anti-Lambda_c\(2625\)-|anti-Lambda\(1405\)0|anti-Lambda\(1520\)0|anti-Lambda\(1600\)0|anti-Lambda\(1670\)0|anti-Lambda\(1690\)0|anti-Lambda\(1800\)0|anti-Lambda\(1810\)0|anti-Lambda\(1820\)0|anti-Lambda\(1830\)0|anti-Sigma\(1660\)0|anti-Sigma\(1670\)0|anti-Sigma\(1750\)0|anti-Sigma\(1775\)0|anti-K\(2\)\(1770\)0|anti-K\(2\)\(1820\)0|Lambda_c\(2593\)\+|Lambda_c\(2625\)\+|anti-Sigma_c\*--|anti-Omega_cc\*-|anti-b'-hadron|omega\(3\)\(1670\)|anti-Lambda_c-|anti-Sigma_c--|anti-Sigma_c\*-|anti-Sigma_c\*0|anti-Omega_c\*0|anti-Omega_cc-|anti-Lambda_b0|anti-Sigma_b\*-|anti-Sigma_b\*0|anti-Sigma_b\*\+|anti-Omega_b\*\+|anti-rndmflav|anti-c-hadron|anti-b-hadron|anti-t-hadron|rho\(3\)\(1690\)\+|rho\(3\)\(1690\)-|rho\(3\)\(1690\)0|Upsilon_1\(1D\)|Upsilon_2\(1D\)|Upsilon_3\(1D\)|Upsilon_1\(2D\)|Upsilon_2\(2D\)|Upsilon_3\(2D\)|Lambda\(1405\)0|Lambda\(1520\)0|Lambda\(1600\)0|Lambda\(1670\)0|Lambda\(1690\)0|Lambda\(1800\)0|Lambda\(1810\)0|Lambda\(1820\)0|Lambda\(1830\)0|anti-Sigma_c-|anti-Sigma_c0|anti-Omega_c0|anti-Xi_cc\*--|anti-Sigma_b-|anti-Sigma_b0|anti-Sigma_b\+|anti-Omega_b\+|anti-deuteron|pi\(1\)\(1400\)\+|pi\(1\)\(1400\)-|pi\(1\)\(1400\)0|pi\(1\)\(1600\)\+|pi\(1\)\(1600\)-|pi\(1\)\(1600\)0|eta\(2\)\(1645\)|pi\(2\)\(1670\)\+|pi\(2\)\(1670\)-|pi\(2\)\(1670\)0|phi\(3\)\(1850\)|anti-D\*\(2S\)0|anti-Delta--|anti-Lambda0|anti-Sigma\*-|anti-Sigma\*0|anti-Sigma\*\+|Sigma\(1660\)0|Sigma\(1670\)0|Sigma\(1750\)0|Sigma\(1775\)0|anti-Xi_cc--|anti-Xi_cc\*-|anti-tritium|anti-B0heavy|anti-nu_tau|a\(0\)\(1450\)\+|a\(0\)\(1450\)-|a\(0\)\(1450\)0|omega\(1650\)|K\(2\)\(1770\)\+|K\(2\)\(1770\)-|K\(2\)\(1770\)0|K\(2\)\(1820\)\+|K\(2\)\(1820\)-|K\(2\)\(1820\)0|anti-D\(2S\)0|anti-B_s0\*0|anti-B_s2\*0|anti-B'_s10|Upsilon\(2S\)|Upsilon\(3S\)|Upsilon\(4S\)|Upsilon\(5S\)|Upsilon\(6S\)|anti-Delta-|anti-Delta0|anti-Delta\+|anti-Sigma-|anti-Sigma0|anti-Sigma\+|anti-Omega\+|anti-Xi'_c-|anti-Xi'_c0|anti-Xi_c\*-|anti-Xi_c\*0|anti-Xi_cc-|anti-Xi'_b0|anti-Xi'_b\+|anti-Xi_b\*0|anti-Xi_b\*\+|anti-B0long|anti-nu_mu|f\(0\)\(1710\)|f\(2\)\(1950\)|a_4\(1970\)\+|a_4\(1970\)-|a_4\(1970\)0|f\(2\)\(2010\)|f\(4\)\(2050\)|f\(2\)\(2300\)|f\(2\)\(2340\)|anti-K'_10|anti-K_0\*0|anti-K_2\*0|anti-K''\*0|anti-K_3\*0|anti-K_4\*0|anti-D_0\*0|anti-D'_10|anti-D_2\*0|anti-B_0\*0|anti-B'_10|anti-B_2\*0|anti-B_s\*0|anti-B_s10|eta_b2\(1D\)|chi_b0\(2P\)|chi_b1\(2P\)|chi_b2\(2P\)|eta_b2\(2D\)|chi_b0\(3P\)|chi_b1\(3P\)|chi_b2\(3P\)|Zb\(10610\)-|Zb\(10610\)\+|Zb\(10650\)-|Zb\(10650\)\+|Sigma_c\*\+\+|anti-Xi_c-|anti-Xi_c0|Omega_cc\*\+|anti-Xi_b0|anti-Xi_b\+|anti-alpha|anti-nu_e|anti-nu_L|b'-hadron|anti-dd_1|anti-uu_1|anti-ud_0|anti-ud_1|anti-sd_0|anti-sd_1|anti-su_0|anti-su_1|anti-ss_1|anti-cd_0|anti-cd_1|anti-cu_0|anti-cu_1|anti-cs_0|anti-cs_1|anti-cc_1|anti-bd_0|anti-bd_1|anti-bu_0|anti-bu_1|anti-bs_0|anti-bs_1|anti-bc_0|anti-bc_1|anti-bb_1|anti-Xdu-|eta\(1405\)|omega\(2S\)|eta\(1475\)|f_0\(1500\)|phi\(1680\)|pi\(1800\)\+|pi\(1800\)-|pi\(1800\)0|anti-K_10|anti-K'\*0|anti-D_10|anti-B_10|anti-B_s0|eta_c\(2S\)|psi\(3770\)|X_1\(3872\)|X_2\(3872\)|psi\(4040\)|psi\(4160\)|psi\(4415\)|eta_b\(2S\)|eta_b\(3S\)|anti-Xi\*0|anti-Xi\*\+|Lambda_c\+|Sigma_c\+\+|Sigma_c\*\+|Sigma_c\*0|Omega_c\*0|Omega_cc\+|Sigma_b\*\+|Sigma_b\*0|Sigma_b\*-|Omega_b\*-|B\*\+nospin|B\*-nospin|c-hadron|b-hadron|t-hadron|anti-Xdd|anti-Xsd|anti-Xsu|anti-Xss|rho\(2S\)\+|rho\(2S\)-|rho\(2S\)0|rho\(3S\)\+|rho\(3S\)-|rho\(3S\)0|anti-K\*0|anti-D\*0|anti-B\*0|Z\(4430\)\+|Z\(4430\)-|anti-Xi0|anti-Xi\+|Sigma_c\+|Xi_cc\*\+\+|Sigma_b\+|Sigma_b-|Omega_b-|anti-He3|anti-b'|anti-t'|Higgs'0|anti-R0|eta\(2S\)|pi\(2S\)\+|pi\(2S\)-|pi\(2S\)0|anti-K0|anti-D0|D\*\(2S\)\+|D\*\(2S\)-|D\*\(2S\)0|anti-B0|psi\(2S\)|h_b\(2P\)|h_b\(3P\)|anti-p-|anti-n0|Delta\+\+|Sigma\*\+|Sigma\*0|Sigma\*-|Xi_cc\+\+|Xi_cc\*\+|anti-d|anti-u|anti-s|anti-c|anti-b|anti-t|Higgs\+|Higgs-|D\(2S\)\+|D\(2S\)-|D\(2S\)0|D_s0\*\+|D_s0\*-|D'_s1\+|D'_s1-|D_s2\*\+|D_s2\*-|B_s0\*0|B_s2\*0|B'_s10|B_c0\*\+|B_c0\*-|B_c2\*\+|B_c2\*-|B'_c1\+|B'_c1-|Delta\+|Delta-|Sigma\+|Sigma-|Omega-|Xi'_c\+|Xi'_c0|Xi_c\*\+|Xi_c\*0|Xi_cc\+|Xi'_b0|Xi'_b-|Xi_b\*0|Xi_b\*-|K'_1\+|K'_1-|K'_10|K_0\*\+|K_0\*-|K_0\*0|K_2\*\+|K_2\*-|K_2\*0|K''\*\+|K''\*-|K''\*0|K_3\*\+|K_3\*-|K_3\*0|K_4\*\+|K_4\*-|K_4\*0|D_0\*\+|D_0\*-|D_0\*0|D'_1\+|D'_1-|D'_10|D_2\*\+|D_2\*-|D_2\*0|D_s\*\+|D_s\*-|D_s1\+|D_s1-|B_0\*\+|B_0\*-|B_0\*0|B'_1\+|B'_1-|B'_10|B_2\*\+|B_2\*-|B_2\*0|B_s\*0|B_c\*\+|B_c\*-|B_c1\+|B_c1-|J\/psi|Xi_c\+|Xi_b-|tau-|tau\+|Z''0|Xdu\+|rho\+|rho-|eta'|a_0\+|a_0-|b_1\+|b_1-|a_1\+|a_1-|a_2\+|a_2-|f'_0|h'_1|f'_1|f'_2|K_1\+|K_1-|K'\*\+|K'\*-|K'\*0|D_1\+|D_1-|D_s\+|D_s-|B_1\+|B_1-|B_c\+|B_c-|Xi\*0|Xi\*-|mu-|mu\+|Z'0|W'\+|W'-|Xu\+|Xu-|pi\+|pi-|K\*\+|K\*-|K\*0|D\*\+|D\*-|D\*0|B\*\+|B\*-|B\*0|Xi-|b'|t'|e-|e\+|L\+|L-|W\+|W-|K\+|K-|D\+|D-|B\+|B-|p\+)(?:\s*,|\s*\)|:)"  # noqa: E501

    return re.sub(particle_name_pattern, wrap_particle_name, cut)


def parse(cut: str, verbose=False) -> tuple:
    """
    Initialize a parser and lexer object and parse cut
    @param cut  cut string which should be parsed
    @param verbose  provide verbose parsing output for
                    parser debugging purposes, not to be set true in production
    """
    lexer = B2Lexer()

    parser = B2Parser(verbose)
    return parser.parse(cut, lexer.tokenize(preprocess(cut)))


def parse_expression(cut: str, verbose=False) -> tuple:
    """
    Initialize a parser and lexer object and parse cut
    @param cut  cut string which should be parsed
    @param verbose  provide verbose parsing output for
                    parser debugging purposes, not to be set true in production
    """
    lexer = B2Lexer()
    parser = B2ExpressionParser(verbose)
    return parser.parse(cut, lexer.tokenize(preprocess(cut)))


if __name__ == "__main__":
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        "-e", "--expression", action="store_const", default=0, const=1
    )
    argparser.add_argument(
        "-l", "--scanning", action="store_const", default=0, const=1
    )
    args = argparser.parse_args()
    cut = input("Please input cut\n")
    cut = preprocess(cut)
    if args.scanning:
        lexer = B2Lexer()
        for token in lexer.tokenize(cut):
            print(token)
    elif args.expression:
        print(parse_expression(cut))
    else:
        print(parse(cut))
