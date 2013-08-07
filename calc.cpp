#include <stdlib.h>
#include <stdio.h>
#include <stack>
#include "pig.h"

#define LOG(tag, format, ...) fprintf(stderr, "%s/%s(%s:%d): " format "\n", #tag, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

struct calc_context : pig::context
{
	std::stack<double> stack;
	calc_context(const char *src): pig::context(src)
	{
	}
};

pig::rule<calc_context> calc_grammar()
{
	using namespace pig;

	auto act_dbg = [](calc_context &ctx, const state &begin, const state &end)
	{
		int row = begin.number + 1;
		int col = begin.pos - begin.line + 1;
		LOG(D, "act_dbg %d:%d:'%.*s'", row, col, int(end.pos - begin.pos), begin.pos);
	};

	auto act_n = [](calc_context &ctx, const state &begin, const state &end)
	{
		ctx.stack.push(strtod(begin.pos, 0));
	};

	auto act_op = [](calc_context &ctx, const state &begin, const state &end)
	{
		auto r = ctx.stack.top();
		ctx.stack.pop();
		auto l = ctx.stack.top();
		ctx.stack.pop();
		LOG(D, "%.2f %.1s %.2f", l, begin.pos, r);
		switch (begin.pos[0])
		{
			case '+':
				ctx.stack.push(l + r);
				break;
			case '-':
				ctx.stack.push(l - r);
				break;
			case '*':
				ctx.stack.push(l * r);
				break;
			case '/':
				ctx.stack.push(l / r);
				break;
		}
	};

	auto space = *" \t"_set;
	auto eol = "\r\n"_lit / "\n\r"_set / eof();
	auto left_brace = '('_ch >> space;
	auto right_brace = ')'_ch >> space;
	auto add = '+'_ch >> space;
	auto sub = '-'_ch >> space;
	auto mul = '*'_ch >> space;
	auto div = '/'_ch >> space;
	auto number = (-"-+"_set >> +"[0-9]"_rng) % act_dbg % act_n >> space;

	rule<calc_context> sum;
	auto value = number / (left_brace >> sum >> right_brace);
	auto product = value >> *(((mul >> value) % act_op) / ((div >> value) % act_op));
	sum = product >> *(((add >> product) % act_op) / ((sub >> product) % act_op));

	return sum >> eol;
}

int main()
{
	auto g = calc_grammar();
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), stdin))
	{
		calc_context ctx(buffer);
		if (g.parse(ctx) && ctx.eof())
		{
			while (!ctx.stack.empty())
			{
				double v = ctx.stack.top();
				ctx.stack.pop();
				LOG(D, "%f", v);
			}
		}
		else
		{
			LOG(E, "Parsing error: '%s'", ctx.cur.pos);
		}
	}
}
