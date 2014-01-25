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

	auto ws = *set{" \t\r\n"};
	auto sign = set{"-+"};
	auto digit = rng["0-9"];
	auto hexdigit = rng["0-9a-fA-F"];

	auto simpleescape = '\\' > set{"\"\\/bfnrt"};
	auto hexescape = "\\u" > +hexdigit;
	auto stringchar = simpleescape / hexescape / (!ch('\\') > any);
	auto string = '"' > *(!ch('"') > stringchar) % act_p > '"' > ws;

	auto fraction = (*digit > '.' > +digit) / (+digit > '.');
	auto exponent = set{"eE"} > -sign > +digit;
	auto number = (-sign > (fraction > -exponent) / (+digit > exponent) / +digit) % act_p > ws;

	rule_type value;
	auto colon = ':' > ws;
	auto pair = string > colon > value;
	auto comma = ',' > ws;
	auto object = '{' > ws > -(pair > *(comma > pair)) > '}' > ws;
	auto array = '[' > ws > -(value > *(comma > value)) > ']' > ws;
	value = string / number / object / array / "true" / "false" / "null" > ws;

	return ws > value > eof;
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
