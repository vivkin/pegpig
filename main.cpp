#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LOG(tag, format, ...) fprintf(stderr, "%s/%s(%s:%d): " format "\n", tag, __func__, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGI(...) LOG("I", ##__VA_ARGS__)
#define LOGE(...) LOG("E", ##__VA_ARGS__)
#define LOGD(...) LOG("D", ##__VA_ARGS__)

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
#include <memory>

template<typename Context> struct rule
{
	typedef rule peg_type;

	struct rule_base
	{
		virtual ~rule_base()
		{
		}
		virtual bool parse(Context &ctx) = 0;
	};

	template<typename Subject> struct rule_def : rule_base
	{
		Subject subject;
		rule_def(Subject const &subject): subject(subject)
		{
		}
		virtual bool parse(Context &ctx)
		{
			return subject.parse(ctx);
		}
	};

	std::shared_ptr<rule_base> def;

	rule() = default;
	rule(const rule &) = default;
	rule &operator=(const rule &) = default;

	template<typename Subject> rule(Subject const &subject)
	{
		def.reset(new rule_def<Subject>(subject));
	}

	template<typename Subject> rule &operator=(Subject const &subject)
	{
		def.reset(new rule_def<Subject>(subject));
		return *this;
	}

	bool parse(Context &ctx)
	{
		return def && def->parse(ctx);
		if (def)
		{
			return def->parse(ctx);
		}
		return false;
	}
};


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

	typedef rule<context> rule;
	auto space = " \t\n\r"_set;
	auto digit = "[0-9]"_rng;
	rule alpha = range{'a', 'z'};

	rule word = +alpha % act_print >> *space;
	parse(+word, "foo bar\r\nbaz");
	auto decimal = +digit % act_decimal >> *space;
	parse(+decimal, "7 13 42");

	auto decimal_list = (Spacing >> *decimal >> EndOfFile) % &act_dbg;
	context dbg_ctx("#dsfsdafgfds foo \t\n7 \n13 042\n");
	parse(decimal_list, dbg_ctx);

	auto x = u8R"prefix(This is a Unicode Character: \u2018.)prefix";
	LOGD("%s", x);
}

void calc()
{
	using namespace pig;
	typedef rule<context> rule;

	auto act_print = [](const char *begin, const char *end)
	{
		LOGD("act print '%.*s'", int(end - begin), begin);
	};

	auto space = *" \t"_set;
	auto eol = "\r\n"_lit / "\n\r"_set / eof();
	auto left_brace = '('_ch % act_print >> space;
	auto right_brace = ')'_ch % act_print >> space;
	auto add = '+'_ch % act_print >> space;
	auto sub = '-'_ch % act_print >> space;
	auto mul = '*'_ch % act_print >> space;
	auto div = '/'_ch % act_print >> space;
	auto digit = "[0-9]"_rng;
	auto number = +digit % act_print >> space;

	rule expr;
	auto value = number / (left_brace >> expr >> right_brace);
	auto product = value >> *((mul >> value) / (div >> value));
	auto sum = product >> *((add >> product) / (sub >> product));
	expr = sum >> eol;

	context dbg_ctx("3 + ( 7 - 11 ) * 20");
	parse(expr, dbg_ctx);
}

int main()
{
	LOGI("fuck :(");
	foobar();
	calc();
	exit(EXIT_SUCCESS);
}
