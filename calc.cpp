#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stack>
#include "pig.h"

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
		fprintf(stderr, "%.2f %.1s %.2f = %.2f\n", l, begin, r, ctx.top());
	};

	auto spacing = *blank;
	auto left_brace = '(' > spacing;
	auto right_brace = ')' > spacing;
	auto add = '+' > spacing;
	auto sub = '-' > spacing;
	auto mul = '*' > spacing;
	auto div = '/' > spacing;
	auto number = (-set{"-+"} > +digit) % act_n > spacing;

	rule_type sum;
	auto value = number / (left_brace > sum > right_brace);
	auto product = value > *(((mul > value) % act_op) / ((div > value) % act_op));
	sum = product > *(((add > product) % act_op) / ((sub > product) % act_op));

	return sum > eol;
}

int main()
{
	typedef pig::scanner<const char *> scanner_type;
	typedef std::stack<double> context_type;
	auto grammar = calc_grammar<scanner_type, context_type>();
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), stdin))
	{
		scanner_type scanner(buffer, buffer + strlen(buffer));
		context_type state;
		if (grammar.parse(scanner, state) && scanner.eof())
		{
			fprintf(stderr, "= %.2f\n", state.top());
		}
		else
		{
			fprintf(stderr, "error: %s\n", scanner.position);
		}
	}
}
