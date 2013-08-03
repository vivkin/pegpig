#pragma once

namespace pig
{
	struct debug_context : context
	{
		int char_count[256] = {0};
		int total_char_count = 0;

		debug_context(const char *src)
		{
			cur = src;
		}

		context &operator++()
		{
			LOGD("dbg_ctx: %c", *cur);
			return context::operator++();
		}
	};

	struct debug_marker
	{
		typedef debug_marker peg_type;
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
