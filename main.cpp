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

#include "peg.h"
#include "pig.h"

void act_string(const char *begin, const char *end)
{
	char buffer[1024] {};
	size_t n = end - begin;
	memcpy(buffer, begin, n);
	LOGD("act string '%s'", buffer);
}

void act_decimal(const char *begin, const char *end)
{
	LOGD("act decimal %ld", strtol(begin, 0, 10));
}

void peg_foo()
{
	using namespace peg;
	using namespace pig;

	auto EndOfFile = !any();
	auto EndOfLine = (one{'\n'} >> one{'\r'}) / one{'\n'} / one{'\r'};
	auto Space = one{' '} / one{'\t'} / EndOfLine;
	auto Comment = one{'#'} >> *(!EndOfLine >> any()) >> EndOfLine;
	auto Commenta = marker{"MARKER"} >> Comment % &act_string;
	auto Spacing = *(Space / Commenta);

	auto ws1 = set{" \t\n\r"};
	auto ws = " \t\n\r"_set;
	auto letter = range{'a', 'z'};
	auto digit = "[0-9]"_rng;//range{'0', '9'};
	LOGD("%c, %c", digit.min, digit.max);
	auto decimal = +digit % &act_decimal >> *ws;

	dparse(" #foo\n  #213213\ndsad", Spacing);
	dparse("#dsfsdafgfds foo \t\n7 \n13 042\n", Spacing >> *decimal >> EndOfFile);
	dparse("foo \n\t    ba$r", literal{"foo"} %&act_string >> Spacing >> (literal{"bar"} %&act_string) / literal{"ba$r"} %&act_string);
	dparse("foo \n\t    ba$r", "foo"_lit %&act_string >> Spacing >> ("bar"_lit %&act_string) / "ba$r"_lit %&act_string);
	//dparse("777\n13\n042", Spacing >> *decimal >> EndOfFile);
	//dparse("7\n13\n042", *(+digit >> -Spacing) >> EndOfFile);

	//dparse("#foo bar\nnomore", Comment);
}

int main(void)
{
	peg_foo();
	exit(EXIT_SUCCESS);
}
