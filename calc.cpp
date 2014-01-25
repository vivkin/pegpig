#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

	auto eol = "\r\n" / set{"\n\r"} / eof;
	auto space = *set{" \t"};
	auto left_brace = '(' > space;
	auto right_brace = ')' > space;
	auto add = '+' > space;
	auto sub = '-' > space;
	auto mul = '*' > space;
	auto div = '/' > space;
	auto number = (-set{"-+"} > +ch("0-9")) % act_n > space;

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
			while (!state.empty())
			{
				double v = state.top();
				state.pop();
				LOG(I, "%.2f", v);
			}
		}
		else
		{
			LOG(E, "Parsing error: '%s'", scanner.position);
		}
	}
}
