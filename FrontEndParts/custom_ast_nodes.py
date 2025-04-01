class ASTNode:
    pass

class Program(ASTNode):     # Program is the root node of the AST
    def __init__(self, statements):
        self.statements = statements
    def __repr__(self):
        return f"Program({self.statements})"

class VarDecl(ASTNode):     # Variable declaration 
    def __init__(self, identifier, expr):
        self.identifier = identifier
        self.expr = expr
    def __repr__(self):
        return f"VarDecl({self.identifier}, {self.expr})"

class LoopStmt(ASTNode):    # Loop statement        
    def __init__(self, var, start_expr, end_expr, body):
        self.var = var
        self.start_expr = start_expr
        self.end_expr = end_expr
        self.body = body
    def __repr__(self):
        return f"LoopStmt({self.var}, {self.start_expr}, {self.end_expr}, {self.body})"

class IfStmt(ASTNode):    # If statement
    def __init__(self, condition, then_branch, else_branch=None):
        self.condition = condition
        self.then_branch = then_branch
        self.else_branch = else_branch
    def __repr__(self):
        return f"IfStmt({self.condition}, {self.then_branch}, {self.else_branch})"

class WhileStmt(ASTNode):   # While statement
    def __init__(self, condition, body):
        self.condition = condition
        self.body = body
    def __repr__(self):
        return f"WhileStmt({self.condition}, {self.body})"

class ExpressionStmt(ASTNode):  # any other expression
    def __init__(self, expr):
        self.expr = expr
    def __repr__(self):
        return f"ExpressionStmt({self.expr})"

class Block(ASTNode):   # Block of code enclosed in curly braces
    def __init__(self, statements):
        self.statements = statements
    def __repr__(self):
        return f"Block({self.statements})"

class BinaryOp(ASTNode):    # Binary operation e.g. a + b or a * b
    def __init__(self, left, op, right):
        self.left = left
        self.op = op
        self.right = right
    def __repr__(self):
        return f"BinaryOp({self.left}, {self.op}, {self.right})"

class UnaryOp(ASTNode):     # Unary operation e.g. -a(cannot handle this yet) or !a
    def __init__(self, op, operand):
        self.op = op
        self.operand = operand
    def __repr__(self):
        return f"UnaryOp({self.op}, {self.operand})"

class Literal(ASTNode):     # Literal value e.g. 5, "hello", 3.14
    def __init__(self, value):
        self.value = value
    def __repr__(self):
        return f"Literal({self.value})"

class Identifier(ASTNode):  # Identifier e.g. variable name
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return f"Identifier({self.name})"

class CallExpr(ASTNode):    # Specifically for function call expression
    def __init__(self, callee, arguments):
        self.callee = callee
        self.arguments = arguments
    def __repr__(self):
        return f"CallExpr({self.callee}, {self.arguments})"

class Assignment(ASTNode):  # Assignment operation e.g. a = 5
    def __init__(self, left, right):
        self.left = left
        self.right = right
        self.top_level_assignment = False
    def __repr__(self):
        return f"Assignment({self.left}, {self.right})"
    
class FunctionDecl(ASTNode):
    def __init__(self, name, params, body):
        self.name = name
        self.params = params
        self.body = body
    def __repr__(self):
        return f"FunctionDecl({self.name}, {self.params}, {self.body})"
    
class ReturnStmt(ASTNode):
    def __init__(self, expr):
        self.expr = expr
    def __repr__(self):
        return f"ReturnStmt({self.expr})"
    
class BooleanLiteral(ASTNode):
    def __init__(self, value):
        self.value = value
    def __repr__(self):
        return f"BooleanLiteral({self.value})"
    
class PrintStmt(ASTNode):
    def __init__(self, expr):
        self.expr = expr
    def __repr__(self):
        return f"PrintStmt({self.expr})"
