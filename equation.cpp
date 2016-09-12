// Implemtation of equation class
// William Immendorf - 2016

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <locale>
#include <stdexcept>

#include "equation.hpp"
#include "utility.hpp"

namespace EquParser
{
	Equation::Equation()
	{
		// This space intentionally left blank
	}
	
	Equation::Equation(const std::string & equation) : infix_equation(equation)
	{
		convert_to_rpn();
	}

	Equation::~Equation()
	{
		// This makes the linker happy.
	}

	bool Equation::handle_input(char c, std::deque<std::string> & output_queue, std::stack<char> & operator_stack)
	{
		return false;
	}

	bool Equation::handle_term(std::string term, std::stack<double> & result_stack)
	{
		return false;
	}

	// Simply returns the raw infix equation string
	std::string Equation::get_infix_equation() const
	{
		return infix_equation;
	}

	// Returns the RPN equation as its native queue format
	std::queue<std::string> Equation::get_rpn_equation() const
	{
		return rpn_equation;
	}

	// Create a string representation of the RPN formatted equation
	std::string Equation::rpn_to_string() const
	{
		std::queue<std::string> clone_queue(rpn_equation);
		std::string result;
		while (!clone_queue.empty())
		{
			result.append(clone_queue.front());
			result.push_back(' ');
			clone_queue.pop();
		}
		return result.substr(0, result.length() - 1);;
	}

	// Evaluates the equation from stored queue and returns the result.
	double Equation::evaluate()
	{
		using std::cout;
		using std::endl;
		using std::string;
		std::stack<double> result_stack;
		std::queue<string> clone_queue(rpn_equation);
		std::locale loc;

		// If the actual RPN equation is empty, toss an exception. Not supposed to happen.
		if (clone_queue.empty())
		{
			throw std::runtime_error("Error: No expression present, are you sure you entered a valid infix expression?");
		}

		// Parse through the queue, evaluting each term within (as per RPN status)
		while (!clone_queue.empty())
		{
			string term = clone_queue.front();
			clone_queue.pop();
			// First process any special rules:
			if (handle_term(term, result_stack))
				continue;
			// Ignore remaining whitespace
			else if (isspace(term[0], loc))
				continue;
			// Numbers are parsed and go on stack
			else if (isdigit(term[0], loc))
				result_stack.push(std::stod(term));
			else
			{
				// It's a operator, attempt to parse it
				try 
				{
					switch (term[0])
					{
						case '+':
							process_operator(result_stack, [] (double val1, double val2) { return val1 + val2; });
							break;
						case '-':
							process_operator(result_stack, [] (double val1, double val2) { return val2 - val1; });
							break;
						case '*':
							process_operator(result_stack, [] (double val1, double val2) { return val1 * val2; });
							break;
						case '/':
							process_operator(result_stack, [] (double val1, double val2) { return val1 / val2; });
							break;
						case '^':
							process_operator(result_stack, [] (double val1, double val2) { return pow(val1, val2); });
							break;
						default:
							string error = "Unrecognized symbol ";
							error.append(term);
							throw std::runtime_error(error);
					}
				}
				catch (std::runtime_error& e)
				{
					cout << "Exception: " << e.what() << endl;
					exit(EXIT_FAILURE);
				}
			}
		}

		// All RPN equations end with one value on the result stack - if not, error out.
		if (result_stack.size() == 1)
		{
			return result_stack.top();
		}
		else
		{
			cout << "Error: Too many values in result stack" << endl;
			exit(EXIT_FAILURE);
		}
	}

	// Internal method - make a RPN representation of the inputted infix equation.
	void Equation::convert_to_rpn()
	{
		using std::string;

		std::deque<string> output_queue;
		std::stack<char> operator_stack;
		std::locale loc;

		bool decimal = false;

		// Loop through equation and use shunting yard algorithm to handle it
		for (char c : infix_equation)
		{
			// Reset decimal flag if not a decimal pointer or digit
			if (c != '.' && !isdigit(c, loc))
			{
				decimal = false;
			}

			// First process special rules:
			if (handle_input(c, output_queue, operator_stack))
				continue;
			// Whitespace or alphabetical characters are ignored.
			else if (isspace(c, loc) || isalpha(c, loc))
				continue;
			// Number characters are addded to output queue
			else if (isdigit(c, loc))
			{
				// If still part of a decimal, excend current number
				if (decimal)
				{
					string prev_string = output_queue.back();
					output_queue.pop_back();
					prev_string.push_back(c);
					output_queue.push_back(prev_string);
				}
				else
					// Make a new number on the queue
					output_queue.push_back(string(1, c));
			}
			// With a decimal point, start a decimal number (if one isn't already started)
			else if (c == '.')
			{
				if (decimal)
					continue;
				else
				{
					string prev_string = output_queue.back();
					if (isdigit(prev_string[0],loc))
					{
						output_queue.pop_back();
						prev_string.push_back('.');
						output_queue.push_back(prev_string);
						decimal = true;
					}
					else
						continue;
				}
			}
			// Parenthesis open, add to operator stack
			else if (c == '(')
				operator_stack.push(c);
			// Partnthesis close, push all operators after parenthesis open to output queue
			else if (c == ')')
			{
				if (!operator_stack.empty())
				{
					char top_operator = operator_stack.top();
					while (top_operator != '(')
					{
						output_queue.push_back(string(1, top_operator));
						operator_stack.pop();
						if (operator_stack.empty())
						{
							std::cout << "Error: Unclosed parenthesis expression. Exiting." << std::endl;
							exit(EXIT_FAILURE);
						}
						top_operator = operator_stack.top();
					}
					operator_stack.pop(); // get rid of used parenthesis
				}
				else 
				{
					std::cout << "Error: Cannot close a parenthesis with no operators. Exiting." << std::endl;
					exit(EXIT_FAILURE);
				}
			}
			// Operator - add to stack
			else
			{
				// If the new operator has greater presiddence over the last operator, push last operator to queue
				if (!operator_stack.empty())
				{
					char top_operator = operator_stack.top();
					if (precendence_less_than(c, top_operator))
					{
						output_queue.push_back(string(1, top_operator));
						operator_stack.pop();
					}
				}
				operator_stack.push(c);
			}

		}

		// Empty output queue into RPN equation
		while (!output_queue.empty())
		{
			rpn_equation.push(output_queue.front());
			output_queue.pop_front();
		}

		// Empty operator stack into RPN equation
		while (!operator_stack.empty())
		{
			rpn_equation.push(string(1, operator_stack.top()));
			operator_stack.pop();
		}
	}

	// Allow direct printing of equation in RPN form
	std::ostream & operator<<(std::ostream & os, const Equation equation)
	{
		os << equation.rpn_to_string();
		return os;
	}
}
