#include <stdlib.h>
#include <stdio.h>
#include <stack>
#include "pig.h"

#define LOG(tag, format, ...) fprintf(stderr, "%s/%s(%s:%d): " format "\n", #tag, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

typedef pig::scanner<std::stack<double>, const char *> calc_scanner;

template<typename Scanner> pig::rule<Scanner> calc_grammar()
{
	typedef typename Scanner::context_type context_type;
	typedef typename Scanner::iterator_type iterator_type;
	using namespace pig;

	auto act_n = [](context_type &state, const iterator_type &begin, const iterator_type &end)
	{
		state.push(strtod(begin, 0));
	};

	auto act_op = [](context_type &state, const iterator_type &begin, const iterator_type &end)
	{
		auto r = state.top();
		state.pop();
		auto l = state.top();
		state.pop();
		switch (begin[0])
		{
			case '+':
				state.push(l + r);
				break;
			case '-':
				state.push(l - r);
				break;
			case '*':
				state.push(l * r);
				break;
			case '/':
				state.push(l / r);
				break;
		}
		LOG(D, "%.2f %.1s %.2f = %.2f (%zd)", l, begin, r, state.top(), state.size());
	};

	auto eol = "\r\n"_lit / "\n\r"_set / eof();
	auto space = *" \t"_set;
	auto left_brace = '('_ch >> space;
	auto right_brace = ')'_ch >> space;
	auto add = '+'_ch >> space;
	auto sub = '-'_ch >> space;
	auto mul = '*'_ch >> space;
	auto div = '/'_ch >> space;
	auto number = (-"-+"_set >> +"[0-9]"_rng) % act_n >> space;

	rule<Scanner> sum;
	auto value = number / (left_brace >> sum >> right_brace);
	auto product = value >> *(((mul >> value) % act_op) / ((div >> value) % act_op));
	sum = product >> *(((add >> product) % act_op) / ((sub >> product) % act_op));

	return sum >> eol;
}

int main()
{
	auto g = calc_grammar<calc_scanner>();
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), stdin))
	{
		std::stack<double> state;
		calc_scanner scn(state, buffer, buffer + strlen(buffer));
		if (g.parse(scn) && scn.eof())
		{
			while (!state.empty())
			{
				double v = state.top();
				state.pop();
				LOG(D, "%.2f", v);
			}
		}
		else
		{
			LOG(E, "Parsing error: '%s'", scn.position);
		}
	}
}
