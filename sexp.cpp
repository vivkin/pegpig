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

	auto ws = *" \t\r\n"_set;

	auto string = '"'_ch > *(!'"'_ch > ('\\'_ch > '"'_ch) / any()) % act_p > '"'_ch > ws;

	auto sign = "-+"_set;
	auto digit = "[0-9]"_rng;
	auto bin = "0b"_lit > +"01"_set;
	auto hex = "0x"_lit > +(digit / "[a-f]"_rng / "[A-F]"_rng);
	auto oct = '0'_ch > *"[0-7]"_rng;
	auto dec = "[1-9]"_rng > *digit;
	auto fraction = (*digit > '.'_ch > +digit) / (+digit > '.'_ch);
	auto exponent = "eE"_set > -sign > +digit;
	auto number = (-sign > (fraction > -exponent) / (+digit > exponent) / bin / hex / oct / dec) % act_p > ws;

	auto delim = "();\" \t\r\n"_set;
	auto identifier = (!delim > any() > *(!delim > any())) % act_p > ws;

	rule_type value;
	auto list = ('('_ch > ws > *value > ')'_ch) % act_p > ws;
	value = string / number / identifier / list > ws;

	return ws > *value > eof();
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