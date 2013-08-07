#include <stdlib.h>
#include <stdio.h>
#include "pig.h"

#define LOG(tag, format, ...) fprintf(stderr, "%s/%s(%s:%d): " format "\n", #tag, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

void foobar()
{
	using namespace pig;

	auto act_dbg = [](const state &begin, const state &end)
	{
		int row = begin.number + 1;
		int col = begin.pos - begin.line + 1;
		LOG(D, "act dbg %d:%d:'%.*s'", row, col, int(end.pos - begin.pos), begin.pos);
	};

	auto act_print = [](const char *begin, const char *end)
	{
		LOG(D, "act print '%.*s'", int(end - begin), begin);
	};

	auto act_decimal = [](const char *begin, const char *end)
	{
		LOG(D, "act decimal %ld", strtol(begin, 0, 10));
	};

	auto EndOfFile = !any();
	auto EndOfLine = (one{'\n'} >> one{'\r'}) / one{'\n'} / one{'\r'};
	auto Space = one{' '} / one{'\t'} / EndOfLine;
	auto Comment = one{'#'} >> *(!EndOfLine >> any()) >> EndOfLine;
	auto Spacing = *(Space / Comment) % act_print;

	auto space = " \t\n\r"_set;
	auto digit = "[0-9]"_rng;
	rule<> alpha = range{'a', 'z'};

	rule<> word = +alpha % act_print >> *space;
	parse(+word, "foo bar\r\nbaz");
	auto decimal = +digit % act_decimal >> *space;
	parse(+decimal, "7 13 42");

	auto decimal_list = (Spacing >> *decimal >> EndOfFile) % act_dbg;
	parse(decimal_list, "#dsfsdafgfds foo \t\n7 \n13 042\n");

	auto x = u8R"prefix(This is a Unicode Character: \u2018.)prefix";
	LOG(D, "%s", x);
}

int main()
{
	foobar();
}
