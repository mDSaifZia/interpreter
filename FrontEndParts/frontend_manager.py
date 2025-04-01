import argparse
from custom_semantic_checker import SemanticChecker
from custom_lexer import Lexer
from custom_parser import Parser
from custom_bytecode_generator import BytecodeGenerator

#To run this file from proj dir: python FrontEndParts/frontend_manager.py -i sample_source_codes/sourceCode.rtsk -o byteCode.bytecode

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
    tokens = lexer.tokenize()   # produces a list of Token objects
    print("Tokens:")
    for token in tokens:
        print(token)

    # Syntax Analysis
    parser_obj = Parser(tokens) 
    ast = parser_obj.parse()    # produces an Abstract Syntax Tree (AST) from the list of Token objects
    print("\nAST:")
    print(ast)

    # Semantic Analysis
    checker = SemanticChecker()
    try:
        checker.check(ast)
        print("Semantic Analysis: PASS")
    except Exception as e:
        print(f"Semantic Analysis: FAIL ({e})")

    # ----------------- Added Bytecode Generation -----------------
    generator = BytecodeGenerator()
    generator.write_bytecode_files(ast)


if __name__ == '__main__':
    main()
