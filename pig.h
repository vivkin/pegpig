#pragma once

#include <memory>

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
				virtual bool parse(Scanner &scn, Context &ctx) = 0;
			};

			template<typename T> struct rule_subject : rule_base
			{
				T subject;
				rule_subject(const T &subject): subject(subject) { }
				virtual bool parse(Scanner &scn, Context &ctx) { return subject.parse(scn, ctx); }
			};

			std::unique_ptr<rule_base> subject;
		};

		std::shared_ptr<rule_def> def = std::make_shared<rule_def>();

		rule() = default;
		rule(const rule &) = default;
		rule &operator=(const rule &) = default;

		template<typename T> rule(const T &subject) { def->subject.reset(new rule_def::rule_subject<T>(subject)); }
		template<typename T> rule &operator=(const T &subject) { def->subject.reset(new rule_def::rule_subject<T>(subject)); return *this; }
		bool parse(Scanner &scn, Context &ctx) { return def->subject && def->subject->parse(scn, ctx); }
	};

	struct eof
	{
		typedef eof type;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx) { return scn.eof(); }
	};

	struct any
	{
		typedef any type;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			if (!scn.eof())
			{
				scn.next();
				return true;
			}
			return false;
		}
	};

	struct one
	{
		typedef one type;
		char c;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			if (!scn.eof() && *scn == c)
			{
				scn.next();
				return true;
			}
			return false;
		}
	};

	struct range
	{
		typedef range type;
		char min;
		char max;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			if (!scn.eof() && (*scn >= min && *scn <= max))
			{
				scn.next();
				return true;
			}
			return false;
		}
	};

	struct set
	{
		typedef set type;
		const char *cstr;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
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
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
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
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			auto save = scn.save();
			if (!subject.parse(scn, ctx))
			{
				scn.restore(save);
			}
			return true;
		}
	};

	template<typename T> struct kleene_star
	{
		typedef kleene_star<T> type;
		T subject;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			auto save = scn.save();
			if (!subject.parse(scn, ctx))
			{
				scn.restore(save);
				return true;
			}
			save = scn.save();
			while (subject.parse(scn, ctx))
			{
				save = scn.save();
			}
			scn.restore(save);
			return true;
		}
	};

	template<typename T> struct kleene_plus
	{
		typedef kleene_plus<T> type;
		T subject;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			if (!subject.parse(scn, ctx))
			{
				return false;
			}
			auto save = scn.save();
			while (subject.parse(scn, ctx))
			{
				save = scn.save();
			}
			scn.restore(save);
			return true;
		}
	};

	template<typename T> struct and_predicate
	{
		typedef and_predicate<T> type;
		T subject;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
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
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
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
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			if (!left.parse(scn, ctx))
			{
				return false;
			}
			return right.parse(scn, ctx);
		}
	};

	template<typename T, typename Y> struct alternative
	{
		typedef alternative<T, Y> type;
		T left;
		Y right;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			auto save = scn.save();
			if (left.parse(scn, ctx))
			{
				return true;
			}
			scn.restore(save);
			return right.parse(scn, ctx);
		}
	};

	template<typename T, typename Y> struct action
	{
		typedef action<T, Y> type;
		T subject;
		Y action;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			auto save = scn.save();
			if (!subject.parse(scn, ctx))
			{
				return false;
			}
			action(save, scn.save(), ctx);
			return true;
		}
	};

	constexpr one operator"" _ch(char c) { return {c}; }
	constexpr range operator"" _rng(const char *cstr, size_t sz) { return {cstr[1], cstr[3]}; }
	constexpr set operator"" _set(const char *cstr, size_t sz) { return {cstr}; }
	constexpr literal operator"" _lit(const char *cstr, size_t sz) { return {cstr}; }
	template<typename T> constexpr greedy_option<typename T::type> operator-(const T &subject) { return {subject}; }
	template<typename T> constexpr kleene_star<typename T::type> operator*(const T &subject) { return {subject}; }
	template<typename T> constexpr kleene_plus<typename T::type> operator+(const T &subject) { return {subject}; }
	template<typename T> constexpr and_predicate<typename T::type> operator&(const T &subject) { return {subject}; }
	template<typename T> constexpr not_predicate<typename T::type> operator!(const T &subject) { return {subject}; }
	template<typename T, typename Y> constexpr sequence<typename T::type, typename Y::type> operator>(const T &left, const Y &right) { return {left, right}; }
	template<typename T, typename Y> constexpr alternative<typename T::type, typename Y::type> operator/(const T &left, const Y &right) { return {left, right}; }
	template<typename T, typename Y> constexpr action<typename T::type, Y> operator%(const T &left, const Y &right) { return {left, right}; }
}
