#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LOG(tag, format, ...) fprintf(stderr, "%s/%s(%s:%d): " format "\n", tag, __func__, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGI(...) LOG("I", __VA_ARGS__)
#define LOGE(...) LOG("E", __VA_ARGS__)
#define LOGD(...) LOG("D", __VA_ARGS__)

bool read_file(const char *path, char **buffer, size_t *buffer_size)
{
	struct stat sb;
	if (stat(path, &sb) != -1)
	{
		LOGI("%s size %d", path, (int)sb.st_size);
		int fd = open(path, O_RDONLY | O_NONBLOCK);
		if (fd != -1)
		{
			*buffer = (char *)malloc(sb.st_size + 1);
			ssize_t n = read(fd, *buffer, sb.st_size);
			close(fd);
			if (n == sb.st_size)
			{
				(*buffer)[sb.st_size] = 0;
				*buffer_size = sb.st_size;
				return true;
			}
			free(*buffer);
		}
	}
	LOGE("stat() || open() || read() %s fail: %s", path, strerror(errno));
	return false;
}

#include "pig.h"
#include "pigdbg.h"

void act_dbg(pig::state begin, pig::state end)
{
	int row = begin.number + 1;
	int col = begin.pos - begin.line + 1;
	LOGD("act dbg %d:%d:'%.*s'", row, col, int(end.pos - begin.pos), begin.pos);
}

void foobar()
{
	using namespace pig;

	auto act_print = [](const char *begin, const char *end)
	{
		LOGD("act print '%.*s'", int(end - begin), begin);
	};

	auto act_decimal = [](const char *begin, const char *end)
	{
		LOGD("act decimal %ld", strtol(begin, 0, 10));
	};

	auto EndOfFile = !any();
	auto EndOfLine = (one{'\n'} >> one{'\r'}) / one{'\n'} / one{'\r'};
	auto Space = one{' '} / one{'\t'} / EndOfLine;
	auto Comment = one{'#'} >> *(!EndOfLine >> any()) >> EndOfLine;
	auto Spacing = *(Space / Comment) % act_print;

	auto space = " \t\n\r"_set;
	auto digit = "[0-9]"_rng;
	auto alpha = range{'a', 'z'};

	auto word = +alpha % act_print >> *space;
	parse(+word, "foo bar\r\nbaz");
	auto decimal = +digit % act_decimal >> *space;
	parse(+decimal, "7 13 42");

	auto decimal_list = (Spacing >> *decimal >> EndOfFile) % &act_dbg;
	debug_context dbg_ctx("#dsfsdafgfds foo \t\n7 \n13 042\n");
	parse(decimal_list, dbg_ctx);
}

int main()
{
	foobar();
	exit(EXIT_SUCCESS);
}
