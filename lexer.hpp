#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

enum class TokenType {
	number = 0,
	equal = 1,
	int_lit = 2,
	identifier = 3,
	new_line = 4,
	print = 5,
	open_paranthesis = 6,
	closed_paranthesis = 7,
	plus = 8,
	end_of_file = 0xFFFFFFFF,

};

struct Token
{
	TokenType type;
	std::optional<std::string> value;
};

class Lexer 
{
public:

	Lexer(std::string src):
		m_src(std::move(src)){}

	std::vector<Token> Tokenize() 
	{
		std::vector<Token> result;
		std::string buf;


		while (peek().has_value()) {
			
			if (m_src.at(m_currentIndex) == '\n') {
				result.push_back({ .type = TokenType::new_line });
				consume();
			}

			if (std::isalpha(peek().value())) 
			{
				buf.push_back(consume());
				while (peek().has_value() && std::isalnum(peek().value()))
				{
					buf.push_back(consume());
				}

				if (m_reservedKeywords.contains(buf)) {
					result.push_back(m_reservedKeywords.at(buf));
					buf.clear();
				}
				else
				{
					result.push_back({ .type = TokenType::identifier, .value = buf });
					buf.clear();
				}

			}
			else if (std::isdigit(peek().value())) {
				buf.push_back(consume());
				while (peek().has_value() && std::isdigit(peek().value())) {
					buf.push_back(consume());
				}
				result.push_back({ .type = TokenType::int_lit, .value = buf });
				buf.clear();
			}
			else if (std::isspace(peek().value())) 
			{
				consume();
			}
			else if (m_specialChars.contains(peek().value())) {
				result.push_back(m_specialChars.at(peek().value()));
				consume();
			}
			
		}
		m_currentIndex = 0;
		result.push_back({ .type = TokenType::end_of_file });
		return result;
	}

private:
	std::optional<char> peek(int offset = 0) const 
	{
		if (m_currentIndex + offset >= m_src.length())
			return {};
		
		return m_src.at(m_currentIndex + offset);
	}

	char consume() 
	{
		return m_src.at(m_currentIndex++);
	}

	const std::string m_src;
	int m_currentIndex{};

	std::unordered_map<std::string, Token> m_reservedKeywords =
	{
		{"number", {.type = TokenType::number}},
		{"print", {.type = TokenType::print}},
	};
	std::unordered_map<char, Token> m_specialChars =
	{
		{'=', {.type = TokenType::equal}},
		{'+', {.type = TokenType::plus}},
		{'(', {.type = TokenType::open_paranthesis}},
		{')', {.type = TokenType::closed_paranthesis}},
	};
};