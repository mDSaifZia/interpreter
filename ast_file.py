import sys
import re
import argparse
import copy
#To run this file: python ast_file.py -i sample_source_codes/sourceCode.rtsk -o byteCode.bytecode

# ================================================ Token and Lexer Implementation ================================================
class Token:
    def __init__(self, type, value, line, position):
        self.type = type
        self.value = value
        self.line = line
        self.position = position

    def __repr__(self):
        return f"Token({self.type}, {self.value}, {self.line}, {self.position})"

class Lexer:
    def __init__(self, code):
        self.code = code
        self.line = 1
        self.keywords = {"if", "else", "while", "return", "fn", "var", "loop", "from", "import"}
        self.token_specification = [
            ('MANY_LINE_COMMENT', r'///(.*)///'),   # Greedy match for multi-line comments
            ('ONE_LINE_COMMENT',  r'//[^\n]*'),      # Updated: match until newline only
            ('FLOAT',             r'\d+\.\d+'),
            ('INTEGER',           r'\d+'),
            ('STRING',            r'"(.*?)"'),
            ('RELATIONAL',        r'==|!=|>=|<=|[><]'), # must check multi-char first. If not will match "==" as 2 consecutive "=" instead
            ('LOGICAL',           r'&&|\|\||[!^]'),
            ('ARITHMETIC',        r'[+\-*/]'),
            ('ASSIGN',            r'='),
            ('DELIMITER',         r'[;,\(\)\{\}\[\]\.]'),
            ('IDEN',              r'[A-Za-z_][A-Za-z0-9_]*'),
            ('NEWLINE',           r'\n'),
            ('WHITESPACE',        r'[ \t]+'),
            ('MISMATCH',          r'.'),    # Catches any other character that is not recognised
        ]

    def tokenize(self):
        tok_regex = '|'.join(f'(?P<{name}>{pattern})' for name, pattern in self.token_specification)
        print(tok_regex)
        tokens = []
        for obj in re.finditer(tok_regex, self.code, re.DOTALL):
            type = obj.lastgroup
            value = obj.group(type)
            match type:
                case 'NEWLINE':
                    self.line += 1
                    continue
                case 'WHITESPACE' | 'ONE_LINE_COMMENT' | 'MANY_LINE_COMMENT':
                    continue
                case 'MISMATCH':
                    raise RuntimeError(f"Unexpected character {value!r} on line {self.line}")
                case _:
                    token = Token(type, value, self.line, obj.start())
                    # Re-classify identifiers that are keywords
                    if type == 'IDEN' and value in self.keywords:
                        token.type = "KEYWORD"
                    tokens.append(token)
        return tokens

# ================================================ AST Node Definitions ================================================
class ASTNode:
    pass

class Program(ASTNode):
    def __init__(self, statements):
        self.statements = statements
    def __repr__(self):
        return f"Program({self.statements})"

class VarDecl(ASTNode):
    def __init__(self, identifier, expr):
        self.identifier = identifier
        self.expr = expr
    def __repr__(self):
        return f"VarDecl({self.identifier}, {self.expr})"

class LoopStmt(ASTNode):
    def __init__(self, var, start_expr, end_expr, body):
        self.var = var
        self.start_expr = start_expr
        self.end_expr = end_expr
        self.body = body
    def __repr__(self):
        return f"LoopStmt({self.var}, {self.start_expr}, {self.end_expr}, {self.body})"

class IfStmt(ASTNode):
    def __init__(self, condition, then_branch, else_branch=None):
        self.condition = condition
        self.then_branch = then_branch
        self.else_branch = else_branch
    def __repr__(self):
        return f"IfStmt({self.condition}, {self.then_branch}, {self.else_branch})"

class WhileStmt(ASTNode):
    def __init__(self, condition, body):
        self.condition = condition
        self.body = body
    def __repr__(self):
        return f"WhileStmt({self.condition}, {self.body})"

class ExpressionStmt(ASTNode):
    def __init__(self, expr):
        self.expr = expr
    def __repr__(self):
        return f"ExpressionStmt({self.expr})"

class Block(ASTNode):
    def __init__(self, statements):
        self.statements = statements
    def __repr__(self):
        return f"Block({self.statements})"

class BinaryOp(ASTNode):
    def __init__(self, left, op, right):
        self.left = left
        self.op = op
        self.right = right
    def __repr__(self):
        return f"BinaryOp({self.left}, {self.op}, {self.right})"

class UnaryOp(ASTNode):
    def __init__(self, op, operand):
        self.op = op
        self.operand = operand
    def __repr__(self):
        return f"UnaryOp({self.op}, {self.operand})"

class Literal(ASTNode):
    def __init__(self, value):
        self.value = value
    def __repr__(self):
        return f"Literal({self.value})"

class Identifier(ASTNode):
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return f"Identifier({self.name})"

class CallExpr(ASTNode):
    def __init__(self, callee, arguments):
        self.callee = callee
        self.arguments = arguments
    def __repr__(self):
        return f"CallExpr({self.callee}, {self.arguments})"

class Assignment(ASTNode):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __repr__(self):
        return f"Assignment({self.left}, {self.right})"

# ================================================ Parser (Syntax Analysis) ================================================
class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def current_token(self):
        if self.pos < len(self.tokens):
            return self.tokens[self.pos]
        return None

    def consume(self, expected_type=None, expected_value=None):
        token = self.current_token()
        if token is None:
            raise Exception("Unexpected end of input")
        if expected_type and token.type != expected_type:
            raise Exception(f"Expected token type {expected_type}, got {token.type}")
        if expected_value and token.value != expected_value:
            raise Exception(f"Expected token value {expected_value}, got {token.value}")
        self.pos += 1
        return token

    def match(self, expected_type, expected_value=None):
        token = self.current_token()
        if token and token.type == expected_type and (expected_value is None or token.value == expected_value):
            self.consume(expected_type, expected_value)
            return True
        return False

    def parse(self):
        statements = []
        while self.current_token() is not None:
            stmt = self.parse_statement()
            if stmt:
                statements.append(stmt)
        return Program(statements)

    def parse_statement(self):
        token = self.current_token()
        # Check for keyword-based statements
        if token.type == "KEYWORD":
            match token.value:
                case "var":
                    return self.parse_var_decl()
                case "fn":
                    return self.parse_function_decl()
                case "loop":
                    return self.parse_loop_stmt()
                case "if":
                    return self.parse_if_stmt()
                case "while":
                    return self.parse_while_stmt()
                case "return":
                    return self.parse_return_stmt()
        # Check for assignment statement: identifier followed by ASSIGN.
        if token.type == "IDEN":
            next_token = self.peek()
            if next_token and next_token.type == "ASSIGN":
                return self.parse_assignment_stmt()
        # Check if it's a block.
        if token.type == "DELIMITER" and token.value == "{":
            return self.parse_block()
        # Fallback: treat it as an expression statement.
        return self.parse_expression_stmt()

    def parse_block(self):
        self.consume("DELIMITER", "{")
        statements = []
        while self.current_token() and not (self.current_token().type == "DELIMITER" and self.current_token().value == "}"):
            stmt = self.parse_statement()
            if stmt:
                statements.append(stmt)
        self.consume("DELIMITER", "}")
        return Block(statements)

    def parse_var_decl(self):
        self.consume("KEYWORD", "var")
        id_token = self.consume("IDEN")
        identifier = Identifier(id_token.value)
        self.consume("ASSIGN", "=")
        expr = self.parse_expression()
        self.consume("DELIMITER", ";")
        return VarDecl(identifier, expr)

    def parse_loop_stmt(self):
        self.consume("KEYWORD", "loop")
        id_token = self.consume("IDEN")
        identifier = Identifier(id_token.value)
        self.consume("KEYWORD", "from")
        self.consume("DELIMITER", "(")
        start_expr = self.parse_expression()
        self.consume("DELIMITER", ",")
        end_expr = self.parse_expression()
        self.consume("DELIMITER", ")")
        body = self.parse_block()
        # Optionally consume a semicolon after the loop block.
        if self.current_token() and self.current_token().type == "DELIMITER" and self.current_token().value == ";":
            self.consume("DELIMITER", ";")
        return LoopStmt(identifier, start_expr, end_expr, body)

    def parse_if_stmt(self):
        self.consume("KEYWORD", "if")
        self.consume("DELIMITER", "(")
        condition = self.parse_expression()
        self.consume("DELIMITER", ")")
        then_branch = self.parse_block()
        else_branch = None
        if self.current_token() and self.current_token().type == "KEYWORD" and self.current_token().value == "else":
            self.consume("KEYWORD", "else")
            if self.current_token() and self.current_token().type == "KEYWORD" and self.current_token().value == "if":
                else_branch = self.parse_if_stmt()
            else:
                else_branch = self.parse_block()
        return IfStmt(condition, then_branch, else_branch)

    def parse_while_stmt(self):
        self.consume("KEYWORD", "while")
        self.consume("DELIMITER", "(")
        condition = self.parse_expression()
        self.consume("DELIMITER", ")")
        body = self.parse_block()
        return WhileStmt(condition, body)

    def parse_expression_stmt(self):
        expr = self.parse_expression()
        self.consume("DELIMITER", ";")
        return ExpressionStmt(expr)

    # Expression parsing (using precedence climbing)
    def parse_expression(self):
        return self.parse_logical_or()

    def parse_logical_or(self):
        node = self.parse_logical_and()
        while self.current_token() and self.current_token().type == "LOGICAL" and self.current_token().value == "||":
            op = self.consume("LOGICAL").value
            right = self.parse_logical_and()
            node = BinaryOp(node, op, right)
        return node

    def parse_logical_and(self):
        node = self.parse_equality()
        while self.current_token() and self.current_token().type == "LOGICAL" and self.current_token().value == "&&":
            op = self.consume("LOGICAL").value
            right = self.parse_equality()
            node = BinaryOp(node, op, right)
        return node

    def parse_equality(self):
        node = self.parse_relational()
        while self.current_token() and self.current_token().type == "RELATIONAL" and self.current_token().value in ("==", "!="):
            op = self.consume("RELATIONAL").value
            right = self.parse_relational()
            node = BinaryOp(node, op, right)
        return node

    def parse_relational(self):
        node = self.parse_additive()
        while self.current_token() and self.current_token().type == "RELATIONAL" and self.current_token().value in (">", "<", ">=", "<="):
            op = self.consume("RELATIONAL").value
            right = self.parse_additive()
            node = BinaryOp(node, op, right)
        return node

    def parse_additive(self):
        node = self.parse_term()
        while self.current_token() and self.current_token().type == "ARITHMETIC" and self.current_token().value in ("+", "-"):
            op = self.consume("ARITHMETIC").value
            right = self.parse_term()
            node = BinaryOp(node, op, right)
        return node

    def parse_term(self):
        node = self.parse_factor()
        while self.current_token() and self.current_token().type == "ARITHMETIC" and self.current_token().value in ("*", "/"):
            op = self.consume("ARITHMETIC").value
            right = self.parse_factor()
            node = BinaryOp(node, op, right)
        return node

    def parse_factor(self):
        token = self.current_token()
        if token.type in ("INTEGER", "FLOAT", "STRING"):
            self.consume(token.type)
            if token.type == "INTEGER":
                value = int(token.value)
            elif token.type == "FLOAT":
                value = float(token.value)
            else:
                value = token.value
            return Literal(value)
        elif token.type == "IDEN":
            self.consume("IDEN")
            # Check for a function call
            if self.current_token() and self.current_token().type == "DELIMITER" and self.current_token().value == "(":
                self.consume("DELIMITER", "(")
                args = []
                if self.current_token() and not (self.current_token().type == "DELIMITER" and self.current_token().value == ")"):
                    args.append(self.parse_expression())
                    while self.current_token() and self.current_token().type == "DELIMITER" and self.current_token().value == ",":
                        self.consume("DELIMITER", ",")
                        args.append(self.parse_expression())
                self.consume("DELIMITER", ")")
                return CallExpr(Identifier(token.value), args)
            return Identifier(token.value)
        elif token.type == "DELIMITER" and token.value == "(":
            self.consume("DELIMITER", "(")
            node = self.parse_expression()
            self.consume("DELIMITER", ")")
            return node
        elif token.type == "LOGICAL" and token.value in ("!", "-"):
            op = self.consume("LOGICAL").value
            operand = self.parse_factor()
            return UnaryOp(op, operand)
        else:
            raise Exception(f"Unexpected token {token}")
        
    def peek(self):
        if self.pos + 1 < len(self.tokens):
            return self.tokens[self.pos + 1]
        return None
    
    def parse_assignment_stmt(self):
        # Consume the identifier on the left-hand side
        left_token = self.consume("IDEN")
        left = Identifier(left_token.value)
        # Consume the assignment operator
        self.consume("ASSIGN", "=")
        # Parse the right-hand side expression
        right = self.parse_expression()
        # Consume the semicolon at the end of the statement
        self.consume("DELIMITER", ";")
        return Assignment(left, right)

# ================================================ Semantic Analysis ================================================

# ================================================ Bytecode Generator ================================================

# ================================================ Main ================================================
def main():
    parser_arg = argparse.ArgumentParser(description="Custom Language Compiler")
    parser_arg.add_argument("-i", "--input", required=True, help="Input source code file (.rtsk)")
    parser_arg.add_argument("-o", "--output", required=True, help="Output bytecode file")
    args = parser_arg.parse_args()

    # Read the source code from the input file.
    with open(args.input, 'r') as f:
        code = f.read()

    # Lexical Analysis
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    print("Tokens:")
    for token in tokens:
        print(token)

    # Syntax Analysis
    parser_obj = Parser(tokens)
    ast = parser_obj.parse()
    print("\nAST:")
    print(ast)

if __name__ == '__main__':
    main()
