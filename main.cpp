#include <stdlib.h>
#include <stdio.h>

#define LOG(tag, format, ...) fprintf(stderr, "%s/%s(%s:%d): " format "\n", tag, __func__, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGI(...) LOG("I", __VA_ARGS__)
#define LOGE(...) LOG("E", __VA_ARGS__)
#define LOGD(...) LOG("D", __VA_ARGS__)

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

bool read_file(const char *path, char **buffer)
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
				return true;
			}
			free(*buffer);
		}
	}
	LOGE("stat() || open() || read() %s fail: %s", path, strerror(errno));
	return false;
}


#include "peg.h"

void peg_grammar_itself()
{
	using namespace peg;
/*
	// Hierarchical syntax
	auto Grammar = Spacing Definition+ EndOfFile
	auto Definition = Identifier LEFTARROW Expression
	auto Expression = Sequence (SLASH Sequence)*
	auto Sequence = Prefix*
	auto Prefix = (AND / NOT)? Suffix
	auto Suffix = Primary (QUESTION / STAR / PLUS)?
	auto Primary = Identifier !LEFTARROW
		/ OPEN Expression CLOSE
		/ Literal / Class / DOT
	// Lexical syntax
	auto Identifier = IdentStart IdentCont* Spacing
	auto IdentStart = [a-zA-Z_]
	auto IdentCont = IdentStart / [0-9]
	auto Literal = [’] (![’] Char)* [’] Spacing
		/ ["] (!["] Char)* ["] Spacing
		Class = ’[’ (!’]’ Range)* ’]’ Spacing
		Range = Char ’-’ Char / Char
		Char = ’\\’ [nrt’"\[\]\\]
	auto / ’\\’ [0-2][0-7][0-7]
	auto / ’\\’ [0-7][0-7]?
	auto / !’\\’ .
	auto LEFTARROW = ’<-’ Spacing
	auto SLASH = ’/’ Spacing
	auto AND = ’&’ Spacing
	auto NOT = ’!’ Spacing
	auto QUESTION = ’?’ Spacing
	auto STAR = ’*’ Spacing
	auto PLUS = ’+’ Spacing
	auto OPEN = ’(’ Spacing
	auto CLOSE = ’)’ Spacing
	auto DOT = ’.’ Spacing
	auto Spacing = *(Space / Comment);
	auto Comment = '#' >> *(!EndOfLine >> any()) >> EndOfLine;
	auto Space = one{' '} / one{'\t'} / EndOfLine;
	auto EndOfLine = (one{'\n'} >> one{'\r'}) / one{'\n'} / one{'\r'};
	auto EndOfFile = !any();
*/
}

namespace peg
{
	template<typename Parser> void dparse(const char *str, Parser p)
	{
		LOGD("Parsing: '%s'", str);
		LOGD("########################################");
		context ctx{str, str + strlen(str)};
		bool result = p.parse(ctx);
		LOGD("########################################");
		LOGD("PEG yoyo %s, reach end %s", result ? "WIN" : "FAIL", ctx.eof() ? "YES" : "NO");
	}
}

void act_comment(const char *begin, const char *end)
{
	char buffer[1024] {};
	size_t len = end - begin;
	memcpy(buffer, begin, len);
	LOGD("act lambda comment len %zd, data '%s'", len, buffer);
}

void act_decimal(const char *begin, const char *end)
{
	act_comment(begin, end);
	LOGD("act decimal %ld", strtol(begin, 0, 10));
}

void peg_foo()
{
	using namespace peg;

	auto EndOfFile = !any();
	auto EndOfLine = (one{'\n'} >> one{'\r'}) / one{'\n'} / one{'\r'};
	auto Space = one{' '} / one{'\t'} / EndOfLine;
	auto Comment = one{'#'} >> *(!EndOfLine >> any()) >> EndOfLine;
	auto Commenta = Comment % &act_comment;
	auto Spacing = *(Space / Commenta);

	auto ws1 = set{" \t\n\r"};
	auto ws = " \t\n\r"_set;
	auto letter = range{'a', 'z'};
	auto digit = "[0-9]"_rng;//range{'0', '9'};
	LOGD("%c, %c", digit.min, digit.max);
	auto decimal = +digit % &act_decimal >> *ws;

	dparse(" #foo\n  #213213\ndsad", Spacing);
	dparse("#dsfsdafgfds foo \t\n7 \n13 042\n", Spacing >> *decimal >> EndOfFile);
	dparse("foo \n\t    ba$r", literal{"foo"} %&act_comment >> Spacing >> (literal{"bar"} %&act_comment) / literal{"ba$r"} %&act_comment);
	dparse("foo \n\t    ba$r", "foo"_lit %&act_comment >> Spacing >> ("bar"_lit %&act_comment) / "a$r"_lit %&act_comment);
	//dparse("777\n13\n042", Spacing >> *decimal >> EndOfFile);
	//dparse("7\n13\n042", *(+digit >> -Spacing) >> EndOfFile);

	//dparse("#foo bar\nnomore", Comment);
}

int main(void)
{
	peg_foo();
	exit(EXIT_SUCCESS);
}
