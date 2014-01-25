#include <stdlib.h>
#include <stdio.h>
#include "pig.h"

#define LOG(tag, format, ...) fprintf(stderr, "%s/%s(%s:%d): " format "\n", #tag, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

template<typename Scanner, typename Context> pig::rule<Scanner, Context> lisp_grammar()
{
	using namespace pig;
	typedef typename Scanner::iterator_type iterator_type;
	typedef rule<Scanner, Context> rule_type;

	auto act_p = [](const iterator_type &begin, const iterator_type &end, Context &ctx)
	{
		LOG(I, "... %.*s", int(end - begin), &*begin);
	};

	auto ws = *set{" \t\r\n"};

	auto string = '"' > *(!ch('"') > (ch('\\') > '"') / any) % act_p > '"' > ws;

	auto sign = set{"-+"};
	auto digit = rng["0-9"];
	auto bin = "0b" > +set{"01"};
	auto hex = "0x" > +(digit / rng["a-fA-F"]);
	auto oct = '0' > *rng["0-7"];
	auto dec = rng["1-9"] > *digit;
	auto fraction = (*digit > '.' > +digit) / (+digit > '.');
	auto exponent = set{"eE"} > -sign > +digit;
	auto number = (-sign > (fraction > -exponent) / (+digit > exponent) / bin / hex / oct / dec) % act_p > ws;

	auto delim = set{"();\" \t\r\n"};
	auto identifier = (!delim > any > *(!delim > any)) % act_p > ws;

	rule_type value;
	auto list = ('(' > ws > *value > ')') % act_p > ws;
	value = string / number / identifier / list > ws;

	return ws > *value > eof;
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		LOG(F, "usage: foo filename");
		return 1;
	}

	char *buffer = NULL;
	size_t size = 0;
	FILE *fp = fopen(argv[1], "rb");
	if (!fp)
	{
		LOG(F, "can't open file: %s", argv[1]);
		return 0;
	}
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		buffer = (char *)malloc(size + 1);
		buffer[size] = 0;
		fread(buffer, 1, size, fp);
		fclose(fp);
	}

	typedef pig::scanner<const char *> scanner_type;
	auto grammar = lisp_grammar<scanner_type, int>();
	scanner_type scanner(buffer, buffer + size);
	int state;
	if (grammar.parse(scanner, state) && scanner.eof())
	{
	}
	else
	{
		LOG(D, "Error %s", &*scanner.position);
	}

	free(buffer);

	return 0;
}
