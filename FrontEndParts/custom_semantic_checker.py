from custom_ast_nodes import *

class SymbolTable:
    def __init__(self):
        self.scopes = [{}]
        self.in_function = False    # Track if we're inside a function
        self.function_params = {}  # Initialize the function_params dictionary
        self.control_vars = set()  # Track control variables (e.g., loop variables)

    def define(self, name, type, is_control_var=False):
        current_scope = self.scopes[-1]
        if name in current_scope:
            existing_type = current_scope[name]
            if existing_type != type:
                raise Exception(f"Type Error: Can't redefine '{name}' from '{existing_type}' to '{type}'")
        current_scope[name] = type
        if is_control_var:
            self.control_vars.add(name)

    def define_function(self, name, params):
        self.function_params[name] = [param[1] for param in params]  # Store parameter types
        self.define(name, "function")

    def lookup(self, name):
        for scope in reversed(self.scopes):
            if name in scope:
                return scope[name]
        raise Exception(f"Semantic Error: '{name}' is not defined.")

    def is_control_var(self, name):
        return name in self.control_vars
    
    def enter_scope(self):
        self.scopes.append({})

    def exit_scope(self):
        if len(self.scopes) > 1:
            self.scopes.pop()
        else:
            raise Exception("Semantic Error: Can't exit global scope.")
        
    def enter_function_scope(self):
        self.in_function = True
        self.enter_scope()

    def exit_function_scope(self):
        self.in_function = False
        self.exit_scope()

class SemanticChecker:
    def __init__(self):
        self.symbol_table = SymbolTable()
        self.current_function = None  # Track the current function being analyzed

    def check(self, node):
        method_name = f"visit_{type(node).__name__}"
        method = getattr(self, method_name, self.generic_visit)
        return method(node)

    def generic_visit(self, node):
        if isinstance(node, ASTNode):
            for attr in vars(node).values():
                if isinstance(attr, ASTNode):
                    self.check(attr)
                elif isinstance(attr, list):
                    for item in attr:
                        if isinstance(item, ASTNode):
                            self.check(item)

    def visit_Program(self, node):
        for stmt in node.statements:
            self.check(stmt)
    
    def visit_FunctionDecl(self, node):
        # Define the function in the current scope
        self.symbol_table.define_function(node.name.name, [(param.name, "int") for param in node.params])  # Assume parameters are integers for now

        # Enter function scope
        self.symbol_table.enter_function_scope()

        # Define function parameters
        for param in node.params:
            self.symbol_table.define(param.name, "int")  # Assume parameters are integers for now

        # Set the current function
        self.current_function = node.name.name

        # Check the function body for unreachable code
        self.visit_Block(node.body)

        # Check the function body for infinite recursion
        self.check_for_infinite_recursion(node.body)

        # Reset the current function
        self.current_function = None

        # Exit function scope
        self.symbol_table.exit_function_scope()

    def visit_VarDecl(self, node):
        value_type = self.check(node.expr)
        try:
            existing_type = self.symbol_table.lookup(node.identifier.name)
            if existing_type != value_type:
                if not (existing_type == "float" and value_type == "int"):
                    if not (existing_type == "int" and value_type == "float"):
                        raise Exception(f"Declaration Error: Can't redefine '{node.identifier.name}' from '{existing_type}' to '{value_type}'")
            self.symbol_table.define(node.identifier.name, value_type)
        except Exception:
            self.symbol_table.define(node.identifier.name, value_type)

    def visit_Assignment(self, node):
        try:
            var_type = self.symbol_table.lookup(node.left.name)
        except Exception:
            raise Exception(f"Semantic Error: '{node.left.name}' is not defined.")
    
        # Check if the variable is a control variable
        if self.symbol_table.is_control_var(node.left.name):
            raise Exception(f"Semantic Error: Cannot modify control variable '{node.left.name}'")
    
        value_type = self.check(node.right)

        # Type compatibility checks
        if var_type == "float" and value_type == "int":
            value_type = "float"
        elif var_type == "int" and value_type == "float":
            value_type = "float"
        elif var_type == "boolean" and value_type != "boolean":
            raise Exception(f"Type Error: Cannot assign '{value_type}' to boolean variable '{node.left.name}'")

        if var_type != value_type:
            raise Exception(f"Type Incompatibility Error: Cannot convert '{value_type}' to '{var_type}'")

        self.symbol_table.define(node.left.name, value_type)

    def visit_Literal(self, node):
        if isinstance(node.value, int):
            return "int"
        elif isinstance(node.value, float):
            return "float"
        elif isinstance(node.value, str):
            return "string"
        elif node.value in {"true", "false"}:
            return "boolean"
        raise Exception(f"Unsupported literal type: {type(node.value)}")

    def visit_BinaryOp(self, node):
        left_type = self.check(node.left)
        right_type = self.check(node.right)

        if node.op == '/' and right_type == "int" and isinstance(node.right, Literal) and node.right.value == 0:
            raise Exception("Division by zero error")

        if left_type != right_type:
            if left_type == "int" and right_type == "float":
                return "float"
            if left_type == "float" and right_type == "int":
                return "float"
            raise Exception(f"Arithmetic Error: Cannot apply '{node.op}' between '{left_type}' and '{right_type}'")

        return left_type

    def visit_UnaryOp(self, node):
        operand_type = self.check(node.operand)
        if node.op == '!':
            if operand_type != "boolean":
                raise Exception(f"Unary Error: Cannot apply '!' to type '{operand_type}'. Expected boolean.")
            return "boolean"
        elif node.op == '-':
            if operand_type in {"int", "float"}:
                return operand_type
            else:
                raise Exception(f"Unary Error: Cannot apply '-' to type '{operand_type}'")

    def visit_Identifier(self, node):
        return self.symbol_table.lookup(node.name)

    def visit_IfStmt(self, node):
        condition_type = self.check(node.condition)
        if condition_type != "boolean":
            raise Exception("Condition expression in 'if' must be of type boolean")
        self.check(node.then_branch)
        if node.else_branch:
            self.check(node.else_branch)

    def visit_WhileStmt(self, node):
        condition_type = self.check(node.condition)
        if condition_type != "boolean":
            raise Exception("Condition expression in 'while' must be of type boolean")
        self.check(node.body)

    def visit_LoopStmt(self, node):
        # Enter a new scope for the loop
        self.symbol_table.enter_scope()
        
         # Define the loop control variable and mark it as a control variable
        self.symbol_table.define(node.var.name, "int", is_control_var=True)
        
        # Check the loop body
        self.check(node.body)
        
        # Exit the scope
        self.symbol_table.exit_scope()

    def visit_ReturnStmt(self, node):
        return self.check(node.expr)
    
    def visit_Block(self, node):
        self.symbol_table.enter_scope()
        for i, stmt in enumerate(node.statements):
            self.check(stmt)
            # Check if the current statement is a return statement
            if isinstance(stmt, ReturnStmt):
                # If there are more statements after the return, they are unreachable
                if i < len(node.statements) - 1:
                    raise Exception(f"Control Flow Error: Unreachable code after return statement in block")
        self.symbol_table.exit_scope()

    def visit_CallExpr(self, node):
        # Look up the function in the symbol table
        try:
            func_type = self.symbol_table.lookup(node.callee.name)
        except Exception:
            raise Exception(f"Semantic Error: Function '{node.callee.name}' is not defined.")

        # Ensure the callee is a function
        if func_type != "function":
            raise Exception(f"Semantic Error: '{node.callee.name}' is not a function.")

        # Check the number of arguments
        if len(node.arguments) != len(self.symbol_table.function_params[node.callee.name]):
            raise Exception(f"Function Argument Error: '{node.callee.name}' expects {len(self.symbol_table.function_params[node.callee.name])} arguments, but got {len(node.arguments)}")

        # Check the types of the arguments
        for i, arg in enumerate(node.arguments):
            arg_type = self.check(arg)
            param_type = self.symbol_table.function_params[node.callee.name][i]
            if arg_type != param_type:
                raise Exception(f"Type Error: Argument {i + 1} of '{node.callee.name}' expects type '{param_type}', but got '{arg_type}'")
        
    def check_for_infinite_recursion(self, node):
        if isinstance(node, Block):
            for stmt in node.statements:
                if isinstance(stmt, ReturnStmt):
                    if isinstance(stmt.expr, CallExpr):
                        if stmt.expr.callee.name == self.current_function:
                            raise Exception(f"Infinite Recursion Error: Function '{self.current_function}' calls itself without a base case")
                self.check_for_infinite_recursion(stmt)
        elif isinstance(node, ASTNode):
            for attr in vars(node).values():
                if isinstance(attr, ASTNode):
                    self.check_for_infinite_recursion(attr)
                elif isinstance(attr, list):
                    for item in attr:
                        if isinstance(item, ASTNode):
                            self.check_for_infinite_recursion(item)
 