import argparse
from custom_lexer import Lexer
from custom_parser import Parser

#To run this file: python FrontEndParts/frontend_manager.py -i sample_source_codes/sourceCode.rtsk -o byteCode.bytecode

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
