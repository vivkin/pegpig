#include <stdio.h>
#include <vector>
#include "pig.h"

template<typename Scanner, typename Context> pig::rule<class Json, Scanner, Context> json_grammar()
{
	using namespace pig;
	typedef typename Scanner::iterator_type iterator_type;

	auto act_p = [](iterator_type begin, iterator_type end, Context &ctx)
	{
		fprintf(stderr, "%.*s\n", int(end - begin), &*begin);
	};

	auto spacing = *space;

	auto simpleescape = '\\' > set{"\"\\/bfnrt"};
	auto hexescape = "\\u" > xdigit > xdigit > xdigit > xdigit;
	auto stringchar = simpleescape / hexescape / (!ch('\\') > any);
	auto string = '"' > *(!ch('"') > stringchar) % act_p > '"' > spacing;

	auto sign = -set{"-+"};
	auto fraction = (*digit > '.' > +digit) / (+digit > '.');
	auto exponent = set{"eE"} > sign > +digit;
	auto number = (sign > (fraction > -exponent) / (+digit > exponent) / +digit) % act_p > spacing;

	rule<class JsonValue, Scanner, Context> value;
	auto colon = ':' > spacing;
	auto pair = string > colon > value;
	auto comma = ',' > spacing;
	auto object = '{' > spacing > -(pair > *(comma > pair)) > '}' > spacing;
	auto array = '[' > spacing > -(value > *(comma > value)) > ']' > spacing;
	value = string / number / object / array / "true" / "false" / "null" > spacing;

	return spacing > value > eof;
}

int main()
{
	char buffer[1024];
	std::vector<char> input;
	while (!feof(stdin))
	{
		size_t n = fread(buffer, 1, sizeof(buffer), stdin);
		input.insert(input.end(), buffer, buffer + n);
	}

	typedef pig::scanner<decltype(input.begin())> scanner_type;
	auto grammar = json_grammar<scanner_type, int>();
	scanner_type scanner{input.begin(), input.end()};
	int state;
	if (grammar(scanner, state) && scanner.eof())
	{
	}
	else
	{
		fprintf(stderr, "error: %s\n", &*scanner.first);
	}
}
