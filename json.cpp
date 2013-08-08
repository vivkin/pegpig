#include <stdio.h>
#include <vector>
#include "pig.h"

#define LOG(tag, format, ...) fprintf(stderr, "%s/%s(%s:%d): " format "\n", #tag, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

pig::rule<> json_grammar()
{
	using namespace pig;

	auto act_dbg = [](context &ctx, const state &begin, const state &end)
	{
		int row = begin.number + 1;
		int col = begin.pos - begin.line + 1;
		LOG(D, "act_dbg %d:%d:'%.*s'", row, col, int(end.pos - begin.pos), begin.pos);
	};

	auto ws = *" \t\r\n"_set;
	auto sign = "-+"_set;
	auto digit = "[0-9]"_rng;
	auto hexdigit = digit / "[a-f]"_rng / "[A-F]"_rng;

	auto simpleescape = '\\'_ch >> "\"\\/bfnrt"_set;
	auto hexescape = "\\u"_lit >> +hexdigit;
	auto stringchar = simpleescape / hexescape / (!'\\'_ch >> any());
	auto string = '"'_ch >> *(!'"'_ch >> stringchar) % act_dbg >> '"'_ch >> ws;

	auto fraction = (*digit >> '.'_ch >> +digit) / (+digit >> '.'_ch);
	auto exponent = "eE"_set >> -sign >> +digit;
	auto number = (-sign >> (fraction >> -exponent) / (+digit >> exponent) / +digit) % act_dbg >> ws;

	rule<> value;
	auto colon = ':'_ch >> ws;
	auto pair = string >> colon >> value;
	auto comma = ','_ch >> ws;
	auto object = '{'_ch >> ws >> -(pair >> *(comma >> pair)) >> '}'_ch >> ws;
	auto array = '['_ch >> ws >> -(value >> *(comma >> value)) >> ']'_ch >> ws;
	value = string / number / object / array / "true"_lit / "false"_lit / "null"_lit >> ws;

	return ws >> value >> eof();
}

int main()
{
	char buffer[1024];
	std::vector<char> input;
	while (!feof(stdin))
	{
		size_t n = fread(buffer, 1, sizeof(buffer), stdin);
		LOG(D, "%zd", n);
		input.insert(input.end(), buffer, buffer + n);
	}
	auto g = json_grammar();
	pig::context ctx(input.data());
	if (g.parse(ctx) && ctx.eof())
	{
	}
	else
	{
		int row = ctx.cur.number + 1;
		int col = ctx.cur.pos - ctx.cur.line + 1;
		LOG(D, "Error %d:%d:'%s'", row, col, ctx.cur.pos);
	}
}
