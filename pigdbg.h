#pragma once

namespace pig
{
	struct state
	{
		const char *pos;
		const char *line;
		int number;

		char operator*()
		{
			return *pos;
		}

		state &operator++()
		{
			char ch = *pos;
			++pos;
			if (ch == '\n')
			{
				line = pos;
				++number;
			}
			return *this;
		}

		operator const char *()
		{
			return pos;
		}
	};

	struct debug_context
	{
		state cur;

		debug_context(const char *src)
		{
			cur.pos = cur.line = src;
			cur.number = 0;
		}

		char operator*()
		{
			return *cur;
		}

		debug_context &operator++()
		{
			++cur;
			return *this;
		}

		state save()
		{
			return cur;
		}

		void restore(const state &saved)
		{
			cur = saved;
		}

		bool eof()
		{
			return *cur == 0;
		}
	};
}
