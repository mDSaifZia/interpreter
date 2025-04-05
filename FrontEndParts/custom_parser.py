from custom_ast_nodes import *

"""
Logic:
    - Checks the first token of the statement to determine the type of statement
    - Calls the corresponding parsing function based on the type of statement
    - Parsing functions are responsible for parsing the statement and returning the corresponding AST node
    - Returns a specific class for that statement and adds it to the list of statements in the Program node
    - Use of the different classes help to enclose the different layers of the statements in the AST
"""

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0    # Current position in the token list

    def current_token(self):    # Get the current token based on the position(self.pos)
        if self.pos < len(self.tokens):
            return self.tokens[self.pos]
        return None

    def consume(self, expected_type=None, expected_value=None):   # Checks for expected type of the token and consume it and move to the next one (via incrementing self.pos)
        token = self.current_token()    
        if token is None:
            raise Exception("Unexpected end of input")
        if expected_type and token.type != expected_type:
            raise Exception(f"Expected token type {expected_type}, got {token.type}")
        if expected_value and token.value != expected_value:
            raise Exception(f"Expected token value {expected_value}, got {token.value}")
        self.pos += 1
        return token

    def match(self, expected_type, expected_value=None):    # Checks that current token matches the expected type and/or value, and consume it if it does
        token = self.current_token()
        if token and token.type == expected_type and (expected_value is None or token.value == expected_value):
            self.consume(expected_type, expected_value)
            return True
        return False

    def parse(self):            # Parse the entire program via running through each token and parsing it into an AST node. 
        statement_list = []         # Returns a ast represented as a Program object containing multiple layers of the statements
        while self.current_token() is not None:
            statement = self.parse_statement()
            if statement:
                statement_list.append(statement)
        return Program(statement_list)

    def parse_statement(self):                  # Parse a single statement based on the current token using different parsing logic for different types of statements
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
                case "print":
                    return self.parse_print_stmt()
        # Check for assignment statement: identifier followed by ASSIGN
        if token.type == "IDEN":
            next_token = self.peek()
            if next_token and next_token.type == "ASSIGN":
                return self.parse_assignment_stmt()
        # Check if it's a block.
        if token.type == "DELIMITER" and token.value == "{":
            return self.parse_block()
        # Otherwise, treat it as an expression statement. Useful for function calls without assignment of its return value
        return self.parse_expression_stmt()
    
    def parse_print_stmt(self):     # Parse a print statement
        self.consume("KEYWORD", "print")
        self.consume("DELIMITER", "(")
        expr = self.parse_expression()
        self.consume("DELIMITER", ")")
        self.consume("DELIMITER", ";")
        return PrintStmt(expr)

    def parse_block(self):          # Parse a block of statements enclosed in curly braces {}, for functions or loops or if-else statements
        self.consume("DELIMITER", "{")
        statement_list = []
        while self.current_token() and not (self.current_token().type == "DELIMITER" and self.current_token().value == "}"):
            statement = self.parse_statement()
            if statement:
                statement_list.append(statement)
        self.consume("DELIMITER", "}")
        return Block(statement_list)

    def parse_var_decl(self):       # Parse a variable declaration statement, matches grammar for variable declaration
        self.consume("KEYWORD", "var")
        id_token = self.consume("IDEN")
        identifier = Identifier(id_token.value)
        self.consume("ASSIGN", "=")
        expr = self.parse_expression()
        self.consume("DELIMITER", ";")
        return VarDecl(identifier, expr)

    def parse_loop_stmt(self):      # Parse a loop statement
        self.consume("KEYWORD", "loop")
        id_token = self.consume("IDEN")
        identifier = Identifier(id_token.value)
        self.consume("KEYWORD", "from")
        self.consume("DELIMITER", "(")
        start_expr = self.parse_expression()    # start index
        self.consume("DELIMITER", ",")
        end_expr = self.parse_expression()   # end index
        self.consume("DELIMITER", ")")
        body = self.parse_block()           # loop body
        # Optionally consume a semicolon after the loop block.
        if self.current_token() and self.current_token().type == "DELIMITER" and self.current_token().value == ";":
            self.consume("DELIMITER", ";")
        return LoopStmt(identifier, start_expr, end_expr, body)

    def parse_if_stmt(self):        # Parse an if-else statement
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

    def parse_while_stmt(self):     # Parse a while statement
        self.consume("KEYWORD", "while")
        self.consume("DELIMITER", "(")
        condition = self.parse_expression()
        self.consume("DELIMITER", ")")
        body = self.parse_block()
        return WhileStmt(condition, body)

    def parse_expression_stmt(self):    # Parse an expression statement, makes use of precedence climbing to parse expressions
        expr = self.parse_expression()
        self.consume("DELIMITER", ";")
        return ExpressionStmt(expr)

    # Expression parsing (using precedence climbing)
    def parse_expression(self):
        return self.parse_logical_or()

    def parse_logical_or(self):
        node = self.parse_logical_and()
        while self.current_token() and self.current_token().type == "LOGICAL_OR" and self.current_token().value == "||":
            op = self.consume("LOGICAL_OR").value
            right = self.parse_logical_and()
            node = BinaryOp(node, op, right)
        return node

    def parse_logical_and(self):
        node = self.parse_equality()
        while self.current_token() and self.current_token().type == "LOGICAL_AND" and self.current_token().value == "&&":
            op = self.consume("LOGICAL_AND").value
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
        node = self.parse_assignment()
        while self.current_token() and self.current_token().type == "RELATIONAL" and self.current_token().value in (">", "<", ">=", "<="):
            op = self.consume("RELATIONAL").value
            right = self.parse_assignment()
            node = BinaryOp(node, op, right)
        return node
    
    def parse_assignment(self):
        node = self.parse_bitwise_or()
        if self.current_token() and self.current_token().type == "ASSIGN":
            self.consume("ASSIGN")
            # Right-associative: parse the right-hand side as an assignment.
            right = self.parse_assignment()
            node = Assignment(node, right)
        return node

    def parse_bitwise_or(self):
        node = self.parse_bitwise_xor()
        while self.current_token() and self.current_token().type == "BITWISE_OR" and self.current_token().value == "|":
            op = self.consume("BITWISE_OR").value
            right = self.parse_bitwise_xor()
            node = BinaryOp(node, op, right)
        return node

    def parse_bitwise_xor(self):
        node = self.parse_bitwise_and()
        while self.current_token() and self.current_token().type == "BITWISE_XOR" and self.current_token().value == "^":
            op = self.consume("BITWISE_XOR").value
            right = self.parse_bitwise_and()
            node = BinaryOp(node, op, right)
        return node

    def parse_bitwise_and(self):
        node = self.parse_shift()
        while self.current_token() and self.current_token().type == "BITWISE_AND" and self.current_token().value == "&":
            op = self.consume("BITWISE_AND").value
            right = self.parse_shift()
            node = BinaryOp(node, op, right)
        return node
    
    def parse_shift(self):
        node = self.parse_additive()
        while self.current_token() and self.current_token().type == "BITWISE_SHIFT":
            op = self.consume("BITWISE_SHIFT").value
            right = self.parse_additive()
            node = BinaryOp(node, op, right)
        return node

    def parse_additive(self):
        left = self.parse_term()
        # If the next token is '+' or '-' then decide how to group.
        if self.current_token() and self.current_token().type == "ARITHMETIC" and self.current_token().value in ("+", "-"):
            # If it's exclusively '+' operators, use right recursion.
            if self.current_token().value == "+":
                original_pos = self.pos
                pure_plus_equation = True
                # Look through the whole expression, consume all successive '+' and terms.
                while self.current_token() and self.current_token().type == "ARITHMETIC":
                    if self.current_token().value != "+":
                        pure_plus_equation = False
                        break
                    self.consume("ARITHMETIC")
                    self.parse_term()
                self.pos = original_pos  # restore position
                if pure_plus_equation:
                    op = self.consume("ARITHMETIC").value  # must be '+'
                    right = self.parse_additive()
                    return BinaryOp(left, op, right)
            # Otherwise, use left-associative grouping.
            node = left
            while self.current_token() and self.current_token().type == "ARITHMETIC" and self.current_token().value in ("+", "-"):
                op = self.consume("ARITHMETIC").value
                right = self.parse_term()
                node = BinaryOp(node, op, right)
            return node
        return left

    def parse_term(self):
        node = self.parse_factor()
        while self.current_token() and self.current_token().type == "ARITHMETIC" and self.current_token().value in ("*", "/", "%"):
            op = self.consume("ARITHMETIC").value
            right = self.parse_factor()
            node = BinaryOp(node, op, right)
        return node

    def parse_factor(self):         # Parse a factor, which can be a literal, identifier, function call, or a parenthesized expression (Most basic unit of an expression)
        token = self.current_token()
        if token.type in ("INTEGER", "FLOAT", "STRING", "BOOLEAN"):    # Check for literals e.g. 5, 3.14, "hello", true/false
            self.consume(token.type)                                    # convert values to python objects for easier handling            
            match token.type:
                case "INTEGER":
                    value = int(token.value)
                case "FLOAT":
                    value = float(token.value)
                case "BOOLEAN":
                    value = True if token.value == "true" else False
                case "STRING":
                    value = str(token.value[1:-1])  # Remove the surrounding quotes from the string literal
                case _:
                    raise Exception(f"Unexpected token {token}")
            return Literal(value, type(value).__name__)
        elif token.type == "IDEN":      # Check for a function call
            self.consume("IDEN")
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
        elif token.type == "DELIMITER" and token.value == "(":    # Check for a parenthesized expression e.g var x = (a + b) * c;
            self.consume("DELIMITER", "(")
            node = self.parse_expression()
            self.consume("DELIMITER", ")")
            return node
        elif token.type == "LOGICAL_NOT" and token.value == "!":    # Check for unary operators e.g !a, -b 
            op = self.consume("LOGICAL_NOT").value
            operand = self.parse_factor()
            return UnaryOp(op, operand)
        elif token.type == "ARITHMETIC" and token.value == "-": # Check for unary operators e.g -b (for handling negative numbers)
            op = self.consume("ARITHMETIC", "-").value
            operand = self.parse_factor()
            return UnaryOp(op, operand)
        else:
            raise Exception(f"Unexpected token {token}")
        
    def peek(self):     # Peek at the next token in the list, useful for identifiers to check that the next token is ASSIGN operator
        if self.pos + 1 < len(self.tokens):
            return self.tokens[self.pos + 1]
        return None
    
    def parse_assignment_stmt(self):        # Parse an assignment statement
        left_token = self.consume("IDEN")
        left = Identifier(left_token.value)
        self.consume("ASSIGN", "=")
        right = self.parse_expression()
        self.consume("DELIMITER", ";")
        return Assignment(left, right)
    
    def parse_function_decl(self):
        self.consume("KEYWORD", "fn")
        name_token = self.consume("IDEN")
        name = Identifier(name_token.value)
        self.consume("DELIMITER", "(")
        params = []
        if not (self.current_token().type == "DELIMITER" and self.current_token().value == ")"):
            params.append(Identifier(self.consume("IDEN").value))
            while self.current_token() and self.current_token().type == "DELIMITER" and self.current_token().value == ",":
                self.consume("DELIMITER", ",")
                params.append(Identifier(self.consume("IDEN").value))
        self.consume("DELIMITER", ")")
        body = self.parse_block()
        return FunctionDecl(name, params, body)
    
    def parse_return_stmt(self):
        self.consume("KEYWORD", "return")
        expr = self.parse_expression()
        self.consume("DELIMITER", ";")
        return ReturnStmt(expr)