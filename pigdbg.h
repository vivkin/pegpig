#pragma once

namespace pig
{
	struct debug_context : context
	{
		int char_count[256] = {0};
		int total_char_count = 0;

		char operator*()
		{
			return *cur;
		}

		context &operator++()
		{
			++cur;
			LOGD("%c", *cur);
			return *this;
		}

		state_t save()
		{
			return cur;
		}

		void restore(state_t state)
		{
			cur = state;
		}

		bool eof()
		{
			return *cur != 0;
		}
	};

	struct debug_marker
	{
		const char *cstr;
		template<typename Context> bool parse(Context &ctx)
		{
			LOGD("marker: %s", cstr);
			return true;
		}
	};

	inline constexpr debug_marker operator"" _dbg(const char *cstr, size_t sz)
	{
		return {cstr};
	}
}
