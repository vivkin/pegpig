#include <stdio.h>
#include "pig.h"

#define LOG(tag, format, ...) fprintf(stderr, "%s/%s(%s:%d): " format "\n", #tag, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

void calc()
{
	using namespace pig;

	auto act_dbg = [](const state &begin, const state &end)
	{
		int row = begin.number + 1;
		int col = begin.pos - begin.line + 1;
		LOG(D, "act dbg %d:%d:'%.*s'", row, col, int(end.pos - begin.pos), begin.pos);
	};

	auto space = *" \t"_set;
	auto eol = "\r\n"_lit / "\n\r"_set / eof();
	auto left_brace = '('_ch % act_dbg >> space;
	auto right_brace = ')'_ch % act_dbg >> space;
	auto add = '+'_ch % act_dbg >> space;
	auto sub = '-'_ch % act_dbg >> space;
	auto mul = '*'_ch % act_dbg >> space;
	auto div = '/'_ch % act_dbg >> space;
	auto digit = "[0-9]"_rng;
	auto number = +digit % act_dbg >> space;

	rule<> expr;
	auto value = number / (left_brace >> expr >> right_brace);
	auto product = value >> *((mul >> value) / (div >> value));
	auto sum = product >> *((add >> product) / (sub >> product));
	expr = sum;

	parse(expr, "(3) + 5 / ( 7 - 11 ) * 20 + (3+5/(7-11)*20)");
}

int main()
{
	LOG(I, "fuck :(");
	calc();
	exit(EXIT_SUCCESS);
}
