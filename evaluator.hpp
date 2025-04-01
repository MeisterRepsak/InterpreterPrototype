#pragma once
#include <unordered_map>
#include <string>
#include <stack>

#include "parser.hpp"

class Evaluator {
public:
	Evaluator(node::NodeProg prog) :
		m_prog(std::move(prog))
	{

	}


	void evaluate_term(const node::NodeTerm* term) {
		struct TermVisitor {
			Evaluator* evaluator;
			void operator()(const node::NodeTermIntLit* term_int_lit) const {
				evaluator->m_varStack.push(std::stod(term_int_lit->int_lit.value.value()));
			}
			void operator()(const node::NodeTermIdentifier* term_ident) const {
				if (!evaluator->m_vars.contains(term_ident->identifier.value.value())) {
					std::cerr << "Undeclared identifier: " << term_ident->identifier.value.value() << std::endl;
					exit(EXIT_FAILURE);
				}
				evaluator->m_varStack.push(term_ident->identifier.value.value());
				
			}
		};
		TermVisitor visitor(this);
		std::visit(visitor, term->var);
	}

	void evaluate_airthmetic_expr(const node::NodeArithmeticExpr* expr) {
		struct ArithmeticExprVisitor {
			Evaluator* evaluator;

			void operator()(const node::NodeExprAdd* add) {
				evaluator->evaluate_expr(add->lhs);
				evaluator->evaluate_expr(add->rhs);
				Var rhs;
				Var lhs;
				if (evaluator->m_vars.contains(evaluator->m_varStack.top())) {
					rhs = evaluator->m_vars.at(evaluator->m_varStack.top());
					evaluator->m_varStack.pop();
				}
				else {
					rhs.val = evaluator->m_varStack.top();
					evaluator->m_varStack.pop();
				}

				if (evaluator->m_vars.contains(evaluator->m_varStack.top())) {
					lhs = evaluator->m_vars.at(evaluator->m_varStack.top());
					evaluator->m_varStack.pop();
				}
				else {
					lhs.val = evaluator->m_varStack.top();
					evaluator->m_varStack.pop();
				}
				if (std::holds_alternative<double>(lhs.val) && std::holds_alternative<double>(rhs.val)) {
					evaluator->m_varStack.push(get<double>(lhs.val) + get<double>(rhs.val));
				}
				else {
					std::cerr << "Terms are invalid" << std::endl;
					exit(EXIT_FAILURE);
				}
			}
		};
		ArithmeticExprVisitor visitor(this);
		std::visit(visitor, expr->var);
	}

	void evaluate_expr(const node::NodeExpr* expr) {
		
		struct ExprVisitor {
			Evaluator* evaluator;

			void operator()(const node::NodeTerm* term) const {
				evaluator->evaluate_term(term);
			}
			void operator()(const node::NodeArithmeticExpr* expr) const {
				evaluator->evaluate_airthmetic_expr(expr);
			}
		};

		ExprVisitor expr_visitor(this);
		std::visit(expr_visitor, expr->var);
	}

	void evaluate_stmt(const node::NodeStmt* stmt)
	{
		struct StmtVisitor {
			Evaluator* evaluator;
			void operator()(const node::NodeStmtPrint* stmt_print) const {
				evaluator->evaluate_expr(stmt_print->expr);
				Var var = evaluator->m_vars.at(evaluator->m_varStack.top());
				if (std::holds_alternative<std::string>(var.val) && evaluator->m_vars.contains(get<std::string>(var.val))) {
					auto value = evaluator->m_vars.at(var.val).val;
					if (std::holds_alternative<double>(value)) {
						std::cout << get<double>(value) << std::endl;
					}
					else if (std::holds_alternative<std::string>(value)) {
						std::cout << get<std::string>(value) << std::endl;
					}
				}
				else if (std::holds_alternative<double>(var.val)) {
					std::cout << get<double>(var.val) << std::endl;
				}
				else if (std::holds_alternative<std::string>(var.val)) {
					std::cout << get<std::string>(var.val) << std::endl;
				}
			}
			void operator()(const node::NodeStmtNumber* stmt_number) const {
				// declares numbers
				if (!evaluator->m_vars.contains(stmt_number->ident.value.value())) {
					evaluator->evaluate_expr(stmt_number->expr);
					evaluator->m_vars.insert({ stmt_number->ident.value.value(), Var{.val = evaluator->m_varStack.top()}});
					evaluator->m_varStack.pop();
				}
				else {
					std::cerr << "Identifier already used: " << stmt_number->ident.value.value() << std::endl;
					exit(EXIT_FAILURE);
				}
			}
		};
		StmtVisitor visitor(this);
		std::visit(visitor, stmt->var);
	};

	void evaluate_prog() {
		for (const node::NodeStmt* stmt : m_prog.stmts)
		{
			evaluate_stmt(stmt);
		}
	}

private:


	struct Var
	{
		std::variant<double, std::string> val = 0.0;
	};
	
	node::NodeProg m_prog;
	std::unordered_map<std::variant<double, std::string>, Var> m_vars{};
	std::stack<std::variant<double,std::string>> m_varStack;
};