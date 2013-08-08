#pragma once

#include <memory>

namespace pig
{
	template<typename Iterator> struct scanner
	{
		typedef Iterator iterator_type;

		iterator_type position;
		iterator_type end;

		scanner(iterator_type const &begin, iterator_type const &end): position(begin), end(end)
		{
		}

		char operator*()
		{
			return *position;
		}

		void next()
		{
			++position;
		}

		iterator_type save()
		{
			return position;
		}

		void restore(iterator_type const &saved)
		{
			position = saved;
		}

		bool eof()
		{
			return position == end;
		}
	};

	template<typename Scanner, typename Context> struct rule
	{
		typedef rule peg_type;

		struct rule_def
		{
			struct rule_base
			{
				virtual ~rule_base()
				{
				}
				virtual bool parse(Scanner &scn, Context &ctx) = 0;
			};

			template<typename Subject> struct rule_subject : rule_base
			{
				Subject subject;
				rule_subject(Subject const &subject): subject(subject)
				{
				}
				virtual bool parse(Scanner &scn, Context &ctx)
				{
					return subject.parse(scn, ctx);
				}
			};

			std::unique_ptr<rule_base> subject;
		};

		std::shared_ptr<rule_def> def = std::make_shared<rule_def>();

		rule() = default;
		rule(rule const &) = default;
		rule &operator=(rule const &) = default;

		template<typename Subject> rule(Subject const &subject)
		{
			def->subject.reset(new rule_def::rule_subject<Subject>(subject));
		}

		template<typename Subject> rule &operator=(Subject const &subject)
		{
			def->subject.reset(new rule_def::rule_subject<Subject>(subject));
			return *this;
		}

		bool parse(Scanner &scn, Context &ctx)
		{
			return def->subject && def->subject->parse(scn, ctx);
		}
	};

	struct eof
	{
		typedef eof peg_type;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			return scn.eof();
		}
	};

	struct any
	{
		typedef any peg_type;
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
		typedef one peg_type;
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
		typedef range peg_type;
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
		typedef set peg_type;
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
		typedef literal peg_type;
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

	template<typename Subject> struct greedy_option
	{
		typedef greedy_option<Subject> peg_type;
		Subject subject;
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

	template<typename Subject> struct kleene_star
	{
		typedef kleene_star<Subject> peg_type;
		Subject subject;
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

	template<typename Subject> struct kleene_plus
	{
		typedef kleene_plus<Subject> peg_type;
		Subject subject;
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

	template<typename Subject> struct and_predicate
	{
		typedef and_predicate<Subject> peg_type;
		Subject subject;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			auto save = scn.save();
			bool result = subject.parse(scn, ctx);
			scn.restore(save);
			return result;
		}
	};

	template<typename Subject> struct not_predicate
	{
		typedef not_predicate<Subject> peg_type;
		Subject subject;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			auto save = scn.save();
			bool result = !subject.parse(scn, ctx);
			scn.restore(save);
			return result;
		}
	};

	template<typename Left, typename Right> struct sequence
	{
		typedef sequence<Left, Right> peg_type;
		Left left;
		Right right;
		template<typename Scanner, typename Context> bool parse(Scanner &scn, Context &ctx)
		{
			if (!left.parse(scn, ctx))
			{
				return false;
			}
			return right.parse(scn, ctx);
		}
	};

	template<typename Left, typename Right> struct alternative
	{
		typedef alternative<Left, Right> peg_type;
		Left left;
		Right right;
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

	template<typename Subject, typename Action> struct action
	{
		typedef action<Subject, Action> peg_type;
		Subject subject;
		Action action;
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

	inline constexpr one operator"" _ch(char c)
	{
		return one{c};
	}

	inline constexpr range operator"" _rng(const char *cstr, size_t sz)
	{
		return {cstr[1], cstr[3]};
	}

	inline constexpr set operator"" _set(const char *cstr, size_t sz)
	{
		return {cstr};
	}

	inline constexpr literal operator"" _lit(const char *cstr, size_t sz)
	{
		return {cstr};
	}

	template<typename Subject> greedy_option<typename Subject::peg_type> operator-(Subject const &subject)
	{
		return {subject};
	}

	template<typename Subject> kleene_star<typename Subject::peg_type> operator*(Subject const &subject)
	{
		return {subject};
	}

	template<typename Subject> kleene_plus<typename Subject::peg_type> operator+(Subject const &subject)
	{
		return {subject};
	}

	template<typename Subject> and_predicate<typename Subject::peg_type> operator&(Subject const &subject)
	{
		return {subject};
	}

	template<typename Subject> not_predicate<typename Subject::peg_type> operator!(Subject const &subject)
	{
		return {subject};
	}

	template<typename Left, typename Right> sequence<typename Left::peg_type, typename Right::peg_type> operator>(Left const &left, Right const &right)
	{
		return {left, right};
	}

	template<typename Left, typename Right> alternative<typename Left::peg_type, typename Right::peg_type> operator/(Left const &left, Right const &right)
	{
		return {left, right};
	}

	template<typename Left, typename Right> action<typename Left::peg_type, Right> operator%(Left const &left, Right const &right)
	{
		return {left, right};
	}
}
