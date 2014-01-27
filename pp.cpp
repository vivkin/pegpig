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
		memcpy(ctx.include_filename, first, last - first);
		ctx.include_filename[last - first] = '\0';
	};
	auto include_file = [](const char *first, const char *last, context &ctx)
	{
		fprintf(stderr, "%d %s\n", ctx.line_number, ctx.include_filename);
	};
	auto inc_line_number = [](const char *first, const char *last, context &ctx) { ++ctx.line_number; };

	auto spacing = *blank;
	auto pragma = '#' > spacing > "pragma" > spacing;
	auto include = "include" > spacing > '"' > *(!ch('"') > any) % set_filename > '"' > spacing;
	auto skip = *(!eol > any);
	auto line = (pragma > include) % include_file / skip > eol >= inc_line_number;
	auto pp = *(!eof > line) > eof;

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
