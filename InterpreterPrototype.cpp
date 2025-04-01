#include <iostream>
#include <sstream>
#include <fstream>

#include "lexer.hpp"
#include "parser.hpp"
#include "evaluator.hpp"

int main(int argc, char* argv[])
{
    /*if (argc != 2)
    {
        std::cerr << "No Input File! Do this instead" << std::endl;
        std::cerr << "./trash.exe <input.trash>" << std::endl;
        return  EXIT_FAILURE;
    }
    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }*/
    std::string contents = "number z = 2 + 2\nprint(z)";

    Lexer lexer = Lexer(std::move(contents));
    std::vector<Token> tokens = lexer.Tokenize();

    Parser parser = Parser(std::move(tokens));

    std::optional<node::NodeProg> prog = parser.parse_prog();

    Evaluator evaluator = Evaluator(std::move(prog.value()));
    evaluator.evaluate_prog();

    

    
}

