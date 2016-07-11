// Utility functions used for parsing and evaluating equations
// William Immendorf - 2016
// Compile with --std=c++11

#include <stdexcept>
#include <unordered_map>

#include "utility.hpp"

struct operator_detail 
{
	int precendence;
	bool right_associative;
};

const std::unordered_map<char, operator_detail> operator_map 
{
	{'+', {2, false}},
	{'-', {2, false}},
	{'*', {3, false}},
	{'/', {3, false}},
	{'^', {4, true}}
};

namespace EquParser
{
	bool precendence_less_than(char op1, char op2)
	{
		auto op_iterator1 = operator_map.find(op1);
		auto op_iterator2 = operator_map.find(op2);

		if (op_iterator1 != operator_map.end() && op_iterator2 != operator_map.end())
		{
			if (op_iterator1->second.right_associative)
			{
				if (op_iterator1->second.precendence < op_iterator2->second.precendence)
					return true;
			}
			else 
			{
				if (op_iterator1->second.precendence <= op_iterator2->second.precendence)
					return true;
			}
		}

		return false;
	}

  std::stack<int> process_operator(std::stack<int> num_stack, std::function<int (int val1, int val2)> expression)
  {
    if (num_stack.size() >= 2)
    {
      int val1 = num_stack.top();
      num_stack.pop();
      int val2 = num_stack.top();
      num_stack.pop();
      num_stack.push(expression(val1, val2));
    }
    else
      throw std::runtime_error("Not enough arguments for an operator");
    return num_stack;
	}
}
