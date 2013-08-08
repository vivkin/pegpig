#include <stdlib.h>
#include <stdio.h>
#include <stack>
#include "pig.h"

#define LOG(tag, format, ...) fprintf(stderr, "%s/%s(%s:%d): " format "\n", #tag, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

template<typename Scanner, typename Context> pig::rule<Scanner, Context> calc_grammar()
{
	using namespace pig;
	typedef typename Scanner::iterator_type iterator_type;
	typedef rule<Scanner, Context> rule_type;

	auto act_n = [](const iterator_type &begin, const iterator_type &end, Context &ctx)
	{
		ctx.push(strtod(begin, 0));
	};

	auto act_op = [](const iterator_type &begin, const iterator_type &end, Context &ctx)
	{
		auto r = ctx.top();
		ctx.pop();
		auto l = ctx.top();
		ctx.pop();
		switch (begin[0])
		{
			case '+':
				ctx.push(l + r);
				break;
			case '-':
				ctx.push(l - r);
				break;
			case '*':
				ctx.push(l * r);
				break;
			case '/':
				ctx.push(l / r);
				break;
		}
		LOG(D, "%.2f %.1s %.2f = %.2f (%zd)", l, begin, r, ctx.top(), ctx.size());
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

	rule_type sum;
	auto value = number / (left_brace >> sum >> right_brace);
	auto product = value >> *(((mul >> value) % act_op) / ((div >> value) % act_op));
	sum = product >> *(((add >> product) % act_op) / ((sub >> product) % act_op));

	return sum >> eol;
}

int main()
{
	typedef pig::scanner<const char *> calc_scanner;
	typedef std::stack<double> calc_context;
	auto grammar = calc_grammar<calc_scanner, calc_context>();
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), stdin))
	{
		calc_scanner scanner(buffer, buffer + strlen(buffer));
		calc_context state;
		if (grammar.parse(scanner, state) && scanner.eof())
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
			LOG(E, "Parsing error: '%s'", scanner.position);
		}
	}
}
