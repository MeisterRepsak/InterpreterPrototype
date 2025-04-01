#pragma once
#include "arena.hpp"
#include <variant>

// CFG for this parser

// Rule 1
// <Prog>
//	1.1
//	<Prog> -> <Stmts>$
// Rule 2
// <Stmts>
//	2.1
//	<Stmts> -> <Stmt>\n<Stmts>
//	2.2
//	<Stmts> -> epsilon
// Rule 3
// <Stmt>
//	3.1
//	<Stmt> -> number identifier = <Expr>
//	3.2
// <Stmt> -> print(<Expr>)
// Rule 4
// <Expr>
//	4.1
//	<Expr> -> <Term>
//	4.2
//	<Expr> -> <Arithmetic Expr>
// Rule 5
// <Arithmetic Expr>
//	5.1
//	<Arithmetic Expr> -> <AddExpr>
// Rule 6
// <AddExpr>
//	6.1
//	<AddExpr> -> <Expr>+<Expr>
// Rule 7
// <Term>
//	7.1
//	<Term> -> int_lit
//	7.2
//	<Term> -> identfier



namespace node {

	struct NodeTermIntLit
	{
		Token int_lit;
	};

	struct NodeTermIdentifier
	{
		Token identifier;
	};
	// Forward cast probably bad
	struct NodeExpr;
	struct  NodeExprAdd {
		NodeExpr* lhs;
		NodeExpr* rhs;
	};

	struct NodeArithmeticExpr
	{
		std::variant<NodeExprAdd*> var;
	};

	struct NodeTerm {
		std::variant<NodeTermIntLit*, NodeTermIdentifier*> var;
	};

	struct NodeExpr
	{
		std::variant<NodeTerm*,NodeArithmeticExpr*> var;
	};

	struct NodeStmtNumber
	{
		Token ident;
		NodeExpr* expr{};
	};

	struct NodeStmtPrint {
		NodeExpr* expr;
	};

	struct NodeStmt
	{
		std::variant<NodeStmtNumber*, NodeStmtPrint*> var;
	};

	struct NodeProg
	{
		std::vector<NodeStmt*> stmts;
	};
}

class Parser 
{
public:
	Parser(std::vector<Token> tokens) :
		m_tokens(std::move(tokens)),
		m_allocater(1024 * 1024 * 8) // 8mb
	{

	}
	// Rule 7
	// Term
	std::optional<node::NodeTerm*> parse_term() {
		// Rule 7.1
		// <Term> -> int_lit
		
		if (std::optional<Token> int_lit = try_consume(TokenType::int_lit)) {
			auto* term_int_lit = m_allocater.alloc<node::NodeTermIntLit>();
			term_int_lit->int_lit = int_lit.value();
			auto term = m_allocater.alloc<node::NodeTerm>();
			term->var = term_int_lit;
			term;
			return term;
		}
		// Rule 7.2
		// <Term> -> identifier
		if (std::optional<Token> identifier = try_consume(TokenType::identifier)) {
			auto* term_identifier = m_allocater.alloc<node::NodeTermIdentifier>();
			term_identifier->identifier = identifier.value();
			auto term = m_allocater.alloc<node::NodeTerm>();
			term->var = term_identifier;
			term;
			return term;
		}

		return {};
	}

	// Rule 4
	// Expr
	std::optional<node::NodeExpr*> parse_expr(int min_prec = 0) {

		// Rule 4.1
		// Expr -> <Term>
		std::optional<node::NodeTerm*> term = parse_term();

		// Verify term
		if (!term.has_value()) {
			// In this case this will cause an error because at this point in the function a value is expected.
			return {};
		}

		// For now Rule 4.1 is expected
		auto expr = m_allocater.alloc<node::NodeExpr>();
		expr->var = term.value();

		while (true)
		{
			std::optional<Token> current_token = peek();
			std::optional<int> prec;
			if (current_token.has_value()) {
				// Get precedence value if token is an operator
				prec = bin_prec(current_token.value().type);
				// Either not an operator or precedence lower than minimum
				if (!prec.has_value() || prec < min_prec) {
					// Break which means this Expr ends with Rule 4.1
					break;
				}
			}
			else {
				// Break which means this Expr ends with Rule 4.1
				break;
			}
			Token op = consume(); // get operator
			int next_min_prec = prec.value() + 1;
			// Rule 4.2 is now assumed
			// Expr -> <Arithmetic Expr>

			//Veryfying right hand side for <Arithmetic Expr>
			std::optional<node::NodeExpr*> expr_rhs = parse_expr(next_min_prec);
			
			if (!expr_rhs.has_value()) {
				std::cerr << "Unable to parse expression" << std::endl;
				exit(EXIT_FAILURE);
			}

			

			auto arithmetic_expr = m_allocater.alloc<node::NodeArithmeticExpr>();
			auto expr_lhs = m_allocater.alloc<node::NodeExpr>();
			
			//Switch case for rule 5
			// <Arithmetic Expr> -> <AddExpr>
			switch (op.type)
			{
			// Rule 6.1
			//<AddExpr> -> <Expr>+<Expr>
			case TokenType::plus:
				auto add = m_allocater.alloc<node::NodeExprAdd>();
				expr_lhs->var = expr->var;
				add->lhs = expr_lhs;
				add->rhs = expr_rhs.value();
				arithmetic_expr->var = add;
				break;
			}
			expr->var = arithmetic_expr;
		}
		return expr;
	}
	// Rule 3
	// Stmt
	std::optional<node::NodeStmt*> parse_stmt() {
		//Rule 3.1
		// Stmt -> number identifier = <Expr>
		if (peek().has_value() && peek().value().type == TokenType::number &&
			peek(1).has_value() && peek(1).value().type == TokenType::identifier &&
			peek(2).has_value() && peek(2).value().type == TokenType::equal) 
		{
			// consume terminal symbols
			consume();
			auto* node_stmt_number = m_allocater.alloc<node::NodeStmtNumber>();
			node:node_stmt_number->ident = consume();
			consume();
			
			// Parse expression rule
			if (std::optional<node::NodeExpr*> expr = parse_expr()) {
				node_stmt_number->expr = expr.value();
			}
			else {
				std::cerr << "Invalid expression" << std::endl;
				exit(EXIT_FAILURE);
					
			}
			// consume terminal symbols
			try_consume(TokenType::new_line, "Expected 'new_line'");
			auto* node_stmt = m_allocater.alloc<node::NodeStmt>();
			node_stmt->var = node_stmt_number;
			return node_stmt;
		}
		//Rule 3.2
		// Stmt -> print(<Expr>)
		if (peek().has_value() && peek().value().type == TokenType::print &&
			peek(1).has_value() && peek(1).value().type == TokenType::open_paranthesis) {
			// consume terminal symbols
			consume();
			consume();
			auto* node_stmt_print = m_allocater.alloc<node::NodeStmtPrint>();

			// Parse expression rule
			if (const auto node_epxr = parse_expr()) {
				node_stmt_print->expr = node_epxr.value();
			}
			else {
				std::cerr << "Invalid expression" << std::endl;
				exit(EXIT_FAILURE);
			}

			// consume terminal symbols
			try_consume(TokenType::closed_paranthesis, "Exprected ')'");
			try_consume(TokenType::end_of_file, "Expected 'end_of_file'");

			auto* node_stmt = m_allocater.alloc<node::NodeStmt>();
			node_stmt->var = node_stmt_print;
			return node_stmt;
		}

		return {};

	}

	// Rule 1
	// Prog -> <Stmts>$
	std::optional<node::NodeProg> parse_prog() {
		node::NodeProg prog;
		// Rule 2
		// Stmts -> <Stmt><Stmts>
		while (peek().has_value())
		{	
			//Parse a statement
			if (std::optional<node::NodeStmt*> stmt = parse_stmt()) {
				//Push statement to program
				prog.stmts.push_back(stmt.value());
			}
			else {
				std::cerr << "Invalid statement" << std::endl;
			}
		}
		return prog;
	}
private:

	std::optional<int> bin_prec(TokenType type)
	{
		switch (type)
		{
		case TokenType::plus:
			return 0;
		default:
			return {};
		}
	}

	std::optional<Token> peek(int offset = 0) const {
		if (m_currentIndex + offset >= m_tokens.size())
			return {};
		return m_tokens.at(m_currentIndex + offset);
	}

	Token consume() {
		return m_tokens.at(m_currentIndex++);
	}

	Token try_consume(TokenType type, const std::string& err_msh) {
		if (peek().has_value() && peek().value().type == type) {
			return consume();
		}

		std::cerr << err_msh << std::endl;
		exit(EXIT_FAILURE);
	}
	std::optional<Token> try_consume(TokenType type) {
		if (peek().has_value() && peek().value().type == type)
		{
			return consume();
		}
		else
		{
			return {};
		}
	}

	const std::vector<Token> m_tokens;
	size_t m_currentIndex;
	ArenaAllocater m_allocater;
};