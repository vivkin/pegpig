#include <stdio.h>
#include <vector>
#include "pig.h"

#define LOG(tag, format, ...) fprintf(stderr, "%s/%s(%s:%d): " format "\n", #tag, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

template<typename Scanner, typename Context> pig::rule<Scanner, Context> json_grammar()
{
	using namespace pig;
	typedef typename Scanner::iterator_type iterator_type;
	typedef rule<Scanner, Context> rule_type;

	auto act_p = [](const iterator_type &begin, const iterator_type &end, Context &ctx)
	{
		LOG(I, "%.*s", int(end - begin), &*begin);
	};

	auto ws = *" \t\r\n"_set;
	auto sign = "-+"_set;
	auto digit = "[0-9]"_rng;
	auto hexdigit = digit / "[a-f]"_rng / "[A-F]"_rng;

	auto simpleescape = '\\'_ch > "\"\\/bfnrt"_set;
	auto hexescape = "\\u"_lit > +hexdigit;
	auto stringchar = simpleescape / hexescape / (!'\\'_ch > any());
	auto string = '"'_ch > *(!'"'_ch > stringchar) % act_p > '"'_ch > ws;

	auto fraction = (*digit > '.'_ch > +digit) / (+digit > '.'_ch);
	auto exponent = "eE"_set > -sign > +digit;
	auto number = (-sign > (fraction > -exponent) / (+digit > exponent) / +digit) % act_p > ws;

	rule_type value;
	auto colon = ':'_ch > ws;
	auto pair = string > colon > value;
	auto comma = ','_ch > ws;
	auto object = '{'_ch > ws > -(pair > *(comma > pair)) > '}'_ch > ws;
	auto array = '['_ch > ws > -(value > *(comma > value)) > ']'_ch > ws;
	value = string / number / object / array / "true"_lit / "false"_lit / "null"_lit > ws;

	return ws > value > eof();
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

	typedef pig::scanner<decltype(input.begin())> scanner_type;
	auto grammar = json_grammar<scanner_type, int>();
	scanner_type scanner(input.begin(), input.end());
	int state;
	if (grammar.parse(scanner, state) && scanner.eof())
	{
	}
	else
	{
		LOG(D, "Error %s", &*scanner.position);
	}
}
