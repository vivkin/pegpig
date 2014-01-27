#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pig.h"

int main(int argc, char **argv)
{
	using namespace pig;

	struct context
	{
		int line_number = 1;
		char include_filename[256];
	};

	auto set_filename = [](const char *first, const char *last, context &ctx)
	{
		size_t n = last - first;
		if (n > sizeof(ctx.include_filename) - 1) n = sizeof(ctx.include_filename) - 1;
		memcpy(ctx.include_filename, first, n);
		ctx.include_filename[n] = '\0';
	};
	auto include_file = [](const char *first, const char *last, context &ctx)
	{
		fprintf(stderr, "%d %s\n", ctx.line_number, ctx.include_filename);
	};
	auto inc_line_number = [](const char *first, const char *last, context &ctx) { ++ctx.line_number; };

	auto spacing = *blank;
	auto skip = *(!eol > any) > eol;
	auto filename = '"' > *(!ch('"') > any) % set_filename > '"';
	auto include = "include" > spacing > filename;
	auto pragma = '#' > spacing > "pragma";
	auto directive = pragma > spacing > include > skip >= include_file;
	auto line = directive / skip >= inc_line_number;
	auto pp = *(!eof > spacing > line) > eof;

	if (argc > 0)
	{
		FILE *f = fopen(argv[1], "rb");
		if (f)
		{
			fseek(f, 0, SEEK_END);
			auto size = ftell(f);
			fseek(f, 0, SEEK_SET);
			char *source = (char *)malloc(size);
			fread(source, 1, size, f);
			fclose(f);

			scanner<const char *> scn{source, source + size};
			context ctx;
			pp(scn, ctx);
		}
	}

	return 0;
}
