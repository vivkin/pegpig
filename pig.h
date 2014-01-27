#pragma once

#include <functional>

namespace pig
{
	struct cstr_scanner
	{
		typedef const char *iterator_type;

		iterator_type first;

		char operator*() const { return *first; }
		void next() { ++first; }
		bool eof() const { return *first == 0; }
		iterator_type pos() const { return first; }
		void restore(const iterator_type &saved) { first = saved; }
	};

	template<typename Iterator> struct scanner
	{
		using iterator_type = Iterator;

		iterator_type first;
		iterator_type last;

		char operator*() const { return *first; }
		void next() { ++first; }
		bool eof() const { return first == last; }
		iterator_type pos() const { return first; }
		void restore(const iterator_type &saved) { first = saved; }
	};

	template<typename ID, typename Scanner, typename Context> struct rule
	{
		typedef rule<ID, Scanner, Context> type;
		static std::function<bool (Scanner &, Context &)> rule_def;
		rule() = default;
		template<typename T> rule(const T &x) { rule_def = x; }
		template<typename T> rule &operator=(const T &x) { rule_def = x; return *this; }
		bool operator()(Scanner &scn, Context &ctx) const
		{
			return rule_def(scn, ctx);
		}
	};

	template<typename ID, typename Scanner, typename Context> std::function<bool (Scanner &, Context &)> rule<ID, Scanner, Context>::rule_def;

	struct eof_parser
	{
		typedef eof_parser type;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			return scn.eof();
		}
	};

	struct any_parser
	{
		typedef any_parser type;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			if (!scn.eof())
			{
				scn.next();
				return true;
			}
			return false;
		}
	};

	struct char_parser
	{
		typedef char_parser type;
		char x;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			if (!scn.eof() && *scn == x)
			{
				scn.next();
				return true;
			}
			return false;
		}
	};

	struct char_range
	{
		typedef char_range type;
		char x, y;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			if (!scn.eof() && (*scn >= x && *scn <= y))
			{
				scn.next();
				return true;
			}
			return false;
		}
	};

	struct char_set
	{
		typedef char_set type;
		const char *cstr;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			if (!scn.eof())
			{
				char ref = *scn;
				for (const char *str = cstr; *str; ++str)
				{
					if (*str == ref)
					{
						scn.next();
						return true;
					}
				}
			}
			return false;
		}
	};

	struct literal
	{
		typedef literal type;
		const char *cstr;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			auto save = scn.pos();
			for (const char *str = cstr; *str && !scn.eof(); ++str, scn.next())
			{
				if (*str != *scn)
				{
					scn.restore(save);
					return false;
				}
			}
			return true;
		}
	};

	template<typename T> struct greedy_option
	{
		typedef greedy_option<T> type;
		T subject;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			auto save = scn.pos();
			if (!subject(scn, ctx)) scn.restore(save);
			return true;
		}
	};

	template<typename T> struct kleene_star
	{
		typedef kleene_star<T> type;
		T subject;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			auto save = scn.pos();
			if (!subject(scn, ctx))
			{
				scn.restore(save);
				return true;
			}
			save = scn.pos();
			while (subject(scn, ctx)) save = scn.pos();
			scn.restore(save);
			return true;
		}
	};

	template<typename T> struct kleene_plus
	{
		typedef kleene_plus<T> type;
		T subject;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			if (!subject(scn, ctx)) return false;
			auto save = scn.pos();
			while (subject(scn, ctx)) save = scn.pos();
			scn.restore(save);
			return true;
		}
	};

	template<typename T> struct and_predicate
	{
		typedef and_predicate<T> type;
		T subject;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			auto save = scn.pos();
			bool result = subject(scn, ctx);
			scn.restore(save);
			return result;
		}
	};

	template<typename T> struct not_predicate
	{
		typedef not_predicate<T> type;
		T subject;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			auto save = scn.pos();
			bool result = !subject(scn, ctx);
			scn.restore(save);
			return result;
		}
	};

	template<typename T, typename Y> struct sequence
	{
		typedef sequence<T, Y> type;
		T left;
		Y right;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			if (!left(scn, ctx)) return false;
			return right(scn, ctx);
		}
	};

	template<typename T, typename Y> struct alternative
	{
		typedef alternative<T, Y> type;
		T left;
		Y right;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			auto save = scn.pos();
			if (left(scn, ctx)) return true;
			scn.restore(save);
			return right(scn, ctx);
		}
	};

	template<typename T, typename Y> struct action
	{
		typedef action<T, Y> type;
		T subject;
		Y action;
		template<typename Scanner, typename Context> bool operator()(Scanner &scn, Context &ctx) const
		{
			auto save = scn.pos();
			if (!subject(scn, ctx)) return false;
			action(save, scn.pos(), ctx);
			return true;
		}
	};

	template<typename T> constexpr greedy_option<typename T::type> operator-(T x) { return {x}; }
	template<typename T> constexpr kleene_star<typename T::type> operator*(T x) { return {x}; }
	template<typename T> constexpr kleene_plus<typename T::type> operator+(T x) { return {x}; }
	template<typename T> constexpr and_predicate<typename T::type> operator&(T x) { return {x}; }
	template<typename T> constexpr not_predicate<typename T::type> operator!(T x) { return {x}; }
	template<typename T, typename Y> constexpr sequence<typename T::type, typename Y::type> operator>(T x, Y y) { return {x, y}; }
	template<typename T> constexpr sequence<typename T::type, char_parser> operator>(T x, char y) { return {x, char_parser{y}}; }
	template<typename T> constexpr sequence<char_parser, typename T::type> operator>(char x, T y) { return {char_parser{x}, y}; }
	template<typename T> constexpr sequence<typename T::type, literal> operator>(T x, const char *y) { return {x, literal{y}}; }
	template<typename T> constexpr sequence<literal, typename T::type> operator>(const char *x, T y) { return {literal{x}, y}; }
	template<typename T, typename Y> constexpr alternative<typename T::type, typename Y::type> operator/(T x, Y y) { return {x, y}; }
	template<typename T> constexpr alternative<typename T::type, char_parser> operator/(T x, char y) { return {x, char_parser{y}}; }
	template<typename T> constexpr alternative<char_parser, typename T::type> operator/(char x, T y) { return {char_parser{x}, y}; }
	template<typename T> constexpr alternative<typename T::type, literal> operator/(T x, const char *y) { return {x, literal{y}}; }
	template<typename T> constexpr alternative<literal, typename T::type> operator/(const char *x, T y) { return {literal{x}, y}; }
	template<typename T, typename Y> constexpr action<typename T::type, Y> operator>=(T x, Y y) { return {x, y}; }
	template<typename T, typename Y> constexpr action<typename T::type, Y> operator%(T x, Y y) { return {x, y}; }

	constexpr char_parser ch(char x) { return {x}; }
	struct {
		constexpr char_range operator[](const char (&x)[4]) { return {x[0], x[2]}; }
		constexpr alternative<char_range, char_range> operator[](const char (&x)[7]) { return char_range{x[0], x[2]} / char_range{x[3], x[5]}; }
		constexpr alternative<alternative<char_range, char_range>, char_range> operator[](const char (&x)[10]) { return char_range{x[0], x[2]} / char_range{x[3], x[5]} / char_range{x[6], x[8]}; }
	} rng;
	typedef char_set set;
	typedef literal lit;

	const auto eof = eof_parser();
	const auto any = any_parser();
	const auto eol = "\r\n" / set{"\n\r"} / eof;
	const auto space = set{"\t\n\v\f\r\x20"};
	const auto blank = set{"\t\x20"};
	const auto lower = rng["a-z"];
	const auto upper = rng["A-Z"];
	const auto digit = rng["0-9"];
	const auto alpha = lower / upper;
	const auto alnum = alpha / digit;
	const auto xdigit = digit / rng["a-fA-F"];
}
