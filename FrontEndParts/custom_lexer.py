import re

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
        self.keywords = {"if", "else", "while", "return", "fn", "var", "loop", "from", "print", "input"}
        self.token_specification = [
            ('MANY_LINE_COMMENT', r'///(.*)///'),   # Greedy match for multi-line comments
            ('ONE_LINE_COMMENT',  r'//[^\n]*'),      # Match until newline only
            ('FLOAT',             r'\d+\.\d+'),
            ('INTEGER',           r'\d+'),
            ('STRING',            r'"(.*?)"'),
            ('BOOLEAN',           r'\b(?:true|false)\b'),
            ('BITWISE_SHIFT',     r'<<|>>'),             # Bitwise left/right shift
            ('RELATIONAL',        r'==|!=|>=|<=|[><]'), # must check multi-char first. If not will match "==" as 2 consecutive "=" instead
            ('LOGICAL_AND',       r'&&'),                 
            ('LOGICAL_OR',        r'\|\|'),               
            ('LOGICAL_NOT',       r'!'),                  
            ('BITWISE_OR',        r'\|'),              
            ('BITWISE_AND',       r'&'),                
            ('BITWISE_XOR',       r'\^'),                
            ('ARITHMETIC',        r'[+\-*/%]'),
            ('ASSIGN',            r'='),
            ('DELIMITER',         r'[;,\(\)\{\}\[\]\.]'),
            ('IDEN',              r'[A-Za-z_][A-Za-z0-9_]*'),
            ('NEWLINE',           r'\n'),
            ('WHITESPACE',        r'[ \t]+'),
            ('MISMATCH',          r'.'),    # Catches any other character that is not recognised
        ]

    def tokenize(self):
        # Note order of self.token_specification is impt for regex matching
        combined_token_regex = '|'.join(f'(?P<{name}>{pattern})' for name, pattern in self.token_specification)    # Create an overarching regex pattern
        tokens = []
        for obj in re.finditer(combined_token_regex, self.code, re.DOTALL):
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