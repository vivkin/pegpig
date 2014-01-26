#pragma once

namespace pig
{
	template<typename Iterator> struct scanner
	{
		typedef Iterator iterator_type;

		iterator_type position;
		iterator_type end;

		scanner(const iterator_type &begin, const iterator_type &end): position(begin), end(end) { }
		char operator*() { return *position; }
		void next() { ++position; }
		iterator_type save() { return position; }
		void restore(const iterator_type &saved) { position = saved; }
		bool eof() { return position == end; }
	};

	template<typename Scanner, typename Context> struct rule
	{
		typedef rule<Scanner, Context> type;

		struct rule_def
		{
			struct rule_base
			{
				virtual ~rule_base() { }
				virtual bool parse(Scanner &scn, Context &ctx) const = 0;
			};

			template<typename T> struct rule_subject : rule_base
			{
				T subject;
				rule_subject(const T &subject): subject(subject) { }
				virtual bool parse(Scanner &scn, Context &ctx) const { return subject.parse(scn, ctx); }
			};

			rule_base *subject;
			int refcount;
		};

		rule_def *def = nullptr;

		rule(): def(new rule_def{nullptr, 1}) { }
		rule(const rule &x): def(x.def) { addref(); }
		template<typename T> rule(const T &x): def(new rule_def{new rule_def::rule_subject<T>(x), 1}) { }
		~rule() { release(); }

		rule &operator=(const rule &x)
		{
			release();
			def = x.def;
			addref();
			return *this;
		}

		template<typename T> rule &operator=(const T &x)
		{
			delete def->subject;
			def->subject = new rule_def::rule_subject<T>(x);
			return *this;
		}

		void addref() { ++def->refcount; }
		void release() { if (--def->refcount == 0) { delete def->subject; delete def; } }

		bool parse(Scanner &scn, Context &ctx) const { return def->subject && def->subject->parse(scn, ctx); }
	};

	struct eof_parser
	{
		typedef eof_parser type;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const { return scn.eof(); }
	};

	struct any_parser
	{
		typedef any_parser type;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
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
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
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
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
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
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
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
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
		{
			auto save = scn.save();
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
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
		{
			auto save = scn.save();
			if (!subject.parse(scn, ctx)) scn.restore(save);
			return true;
		}
	};

	template<typename T> struct kleene_star
	{
		typedef kleene_star<T> type;
		T subject;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
		{
			auto save = scn.save();
			if (!subject.parse(scn, ctx))
			{
				scn.restore(save);
				return true;
			}
			save = scn.save();
			while (subject.parse(scn, ctx)) save = scn.save();
			scn.restore(save);
			return true;
		}
	};

	template<typename T> struct kleene_plus
	{
		typedef kleene_plus<T> type;
		T subject;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
		{
			if (!subject.parse(scn, ctx)) return false;
			auto save = scn.save();
			while (subject.parse(scn, ctx)) save = scn.save();
			scn.restore(save);
			return true;
		}
	};

	template<typename T> struct and_predicate
	{
		typedef and_predicate<T> type;
		T subject;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
		{
			auto save = scn.save();
			bool result = subject.parse(scn, ctx);
			scn.restore(save);
			return result;
		}
	};

	template<typename T> struct not_predicate
	{
		typedef not_predicate<T> type;
		T subject;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
		{
			auto save = scn.save();
			bool result = !subject.parse(scn, ctx);
			scn.restore(save);
			return result;
		}
	};

	template<typename T, typename Y> struct sequence
	{
		typedef sequence<T, Y> type;
		T left;
		Y right;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
		{
			if (!left.parse(scn, ctx)) return false;
			return right.parse(scn, ctx);
		}
	};

	template<typename T, typename Y> struct alternative
	{
		typedef alternative<T, Y> type;
		T left;
		Y right;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
		{
			auto save = scn.save();
			if (left.parse(scn, ctx)) return true;
			scn.restore(save);
			return right.parse(scn, ctx);
		}
	};

	template<typename T, typename Y> struct action
	{
		typedef action<T, Y> type;
		T subject;
		Y action;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) const
		{
			auto save = scn.save();
			if (!subject.parse(scn, ctx)) return false;
			action(save, scn.save(), ctx);
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

	constexpr auto eof = eof_parser();
	constexpr auto any = any_parser();
	constexpr auto eol = "\r\n" / set{"\n\r"} / eof;
	constexpr auto space = set{"\t\n\v\f\r\x20"};
	constexpr auto blank = set{"\t\x20"};
	constexpr auto lower = rng["a-z"];
	constexpr auto upper = rng["A-Z"];
	constexpr auto digit = rng["0-9"];
	constexpr auto alpha = lower / upper;
	constexpr auto alnum = alpha / digit;
	constexpr auto xdigit = digit / rng["a-fA-F"];
}
