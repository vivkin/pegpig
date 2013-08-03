#pragma once

namespace peg
{
	template<typename Context> struct debug_context : context
	{
		int char_count[256] = {0};
		int total_char_count = 0;

		debug_context(Context ctx)
		{
			cur = ctx.cur;
			LOGD("###");
		}

		~debug_context()
		{
			LOGD("###");
		}

		typedef const char *state_t;

		const char *cur;

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

	template<typename Grammar, typename Context> bool dbg_parse(Grammar g, Context ctx)
	{
		debug_context<Context> dbg_ctx(ctx);
		bool result = g.parse(dbg_ctx);
		return result;
	}
}
