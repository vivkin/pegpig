#pragma once

namespace pig
{
	using namespace peg;

	struct marker
	{
		const char *cstr;
		bool parse(context &ctx)
		{
			LOGD("marker: %s", cstr);
			return true;
		}
	};

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
/*
	struct context
	struct eof
	struct any
	struct one
	struct range
	struct set
	struct literal
	template<typename Subject> struct greedy_option
	template<typename Subject> struct kleene_star
	template<typename Subject> struct kleene_plus
	template<typename Subject> struct and_predicate
	template<typename Subject> struct not_predicate
	template<typename LeftType, typename RightType> struct sequence
	template<typename LeftType, typename RightType> struct alternative
	template<typename Subject, typename Action> struct action
	*/
