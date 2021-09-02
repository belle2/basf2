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
    POWER = r'\*\*|\^'
    #: token regular expression for '*'
    TIMES = r'\*'
    #: token regular expression for '/'
    DIVIDE = r'/'
    #: token regular expression for '+'
    PLUS = r'\+'
    #: token regular expression for '-'
    MINUS = r'-'

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
        self.control_token_stack.append('BRACK')
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
            raise SyntaxError(
                "Unmatched ']' in cut."
            )
        if state == 'BRACK':
            return t
        elif state == 'PAREN':
            raise SyntaxError(
                "Illegal ']', expected ')."
            )

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
        if state == 'BRACK':
            raise SyntaxError("Illegal ')', expected ']'.")
        elif state == 'PAREN':
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
        Scaning function for integer values
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

    @_(r"[a-zA-Z]([a-zA-Z_0-9&:]|\+:|-:|':)*")  # noqa: F821
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
            "infinity": "DOUBLE"
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


class B2Parser(Parser):
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
        | <expression> <comparison_operator> <expression> <comparison_operator>
          <expression>

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
        | IDENTIFIER
        | INTEGER
        | BOOLEAN
        | DOUBLE

    <function> ::= IDENTIFIER LPAREN <parameters> RPAREN

    <parameters> ::= EMPTY
        | <parameter_list>

    <parameter_list> ::= <parameter>
        | <parameter_list> COMMA <parameter>

    <parameter> ::= <boolean_expression>
    """
    def __init__(self, verbose=False):
        """Initialize Parser
        Parameters:
            verbose (bool): run parser in verbose mode. The nodetype names in
                the parsed tuple are written out and not encoded as integer.
                Useful for debugging parsing errors.
        """
        #: verbose setting, creates more human readable tuple output
        #: only for testing, debugging purposes, not to be used in production
        self.verbose = verbose
        #: parameter state stack
        #: used for scope detection of variables and metavariables
        self.parameter_stack = list()

    #: token list from B2Lexer, this is required
    tokens = B2Lexer.tokens

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
        ("right", "POWER")
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
    #: Must match BooleanOperator enum in framework/utilities/AbstractNodes.h
    b_operator_types = {
        "and": 0,
        "or": 1,
    }

    #: Dict for encoding comparison operator types to integers
    #: Must match ComparisonOperator enum framework/utilities/AbstractNodes.h
    c_operator_types = {
        "==": 0,
        ">=": 1,
        "<=": 2,
        ">": 3,
        "<": 4,
        "!=": 5,
    }

    #: Dict for encoding arithmetic operator types to integers
    #: Must match ArithmeticOperator enum framework/utilities/AbstractNodes.h
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
        Return the node type value or node name if verbose setting is chosen.
        """
        return node_name if self.verbose else self.node_types[node_name]

    def get_coper_type(self, coper_name: str):
        """
        Return the comparison operator type value or comparison operator name
        if verbose setting is chosen.
        """
        return coper_name if self.verbose else self.c_operator_types[coper_name]  # noqa: E501

    def get_boper_type(self, boper_name: str):
        """
        Return the boolean operator type value or boolean operator name
        if verbose setting is chosen.
        """
        return boper_name if self.verbose else self.b_operator_types[boper_name]  # noqa: E501

    def get_a_operation_type(self, operation_name: str):
        """
        Return the arithmetic operator type value or arithmetic operator token
        if verbose setting is chosen.
        """
        return operation_name if self.verbose else self.a_operation_types[operation_name]  # noqa: E501

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
                self.get_boper_type(p.AND)
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
                False
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
                True
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
            self.get_coper_type(p.comparison_operator)
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
            self.get_coper_type(p.comparison_operator1)
            )

    @_(  # noqa: F821
        r"EQUALEQUAL",
        r"GREATER",
        r"LESS",
        r"GREATEREQUAL",
        r"LESSEQUAL",
        r"NOTEQUAL")
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
                self.get_a_operation_type(p[1])
            )
        except AttributeError:
            return p.term

    @_(r"term TIMES factor", r"term DIVIDE factor", r"factor")  # noqa: F821
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
                self.get_a_operation_type(p[1])
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
            False
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
            False
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
            self.get_a_operation_type(p.POWER)
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
            True
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

    @_(r"IDENTIFIER")  # noqa: F821
    def primary(self, p):  # noqa: F811
        """
        Parsing function for <primary> nonterminal

        Grammar rule:
            <primary> ::= IDENTIFIER
        """
        if self.parameter_stack:
            return (self.get_node_type("IdentifierNode"), p.IDENTIFIER, False)
        else:
            return (self.get_node_type("IdentifierNode"), p.IDENTIFIER, True)

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
                False
            )
        else:
            return (
                self.get_node_type("FunctionNode"),
                p.IDENTIFIER,
                len(p.parameters),
                p.parameters,
                True
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
            return p.parameter_list+(p.parameter,)
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

    @_('')  # noqa: F821
    def parameter_begin_scope(self, p):
        """
        Parsing action to open parameter scope
        """
        self.parameter_stack.append(None)

    @_('')  # noqa: F821
    def parameter_end_scope(self, p):
        """
        Parsing action to close parameter scope
        """
        self.parameter_stack.pop()

    def error(self, p):
        """
        Error function
        """
        raise SyntaxError('Error in cutstring.')


def parse(cutstring: str) -> tuple:
    """
    Initialize a parser and lexer object and parse cutstring
    """
    lexer = B2Lexer()
    parser = B2Parser()
    return parser.parse(lexer.tokenize(cutstring))


if __name__ == "__main__":
    cutstring = input("Bitte cutstring eingeben:\n")
    print(parse(cutstring))
