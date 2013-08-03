#pragma once

namespace pig
{
	struct context
	{
		typedef const char *state_t;

		state_t cur;

		char operator*()
		{
			return *cur;
		}

		context &operator++()
		{
			++cur;
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
			return *cur == 0;
		}
	};

	struct eof
	{
		typedef eof peg_type;
		template<typename Context> bool parse(Context &ctx)
		{
			return ctx.eof();
		}
	};

	struct any
	{
		typedef any peg_type;
		template<typename Context> bool parse(Context &ctx)
		{
			if (!ctx.eof())
			{
				++ctx;
				return true;
			}
			return false;
		}
	};

	struct one
	{
		typedef one peg_type;
		char c;
		template<typename Context> bool parse(Context &ctx)
		{
			if (!ctx.eof() && *ctx == c)
			{
				++ctx;
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
		template<typename Context> bool parse(Context &ctx)
		{
			if (!ctx.eof() && (*ctx >= min && *ctx <= max))
			{
				++ctx;
				return true;
			}
			return false;
		}
	};

	struct set
	{
		typedef set peg_type;
		const char *cstr;
		template<typename Context> bool parse(Context &ctx)
		{
			if (!ctx.eof())
			{
				char ref = *ctx;
				for (const char *str = cstr; *str; ++str)
				{
					if (*str == ref)
					{
						++ctx;
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
		template<typename Context> bool parse(Context &ctx)
		{
			auto state = ctx.save();
			for (const char *str = cstr; *str && !ctx.eof(); ++str, ++ctx)
			{
				if (*str != *ctx)
				{
					ctx.restore(state);
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
		template<typename Context> bool parse(Context &ctx)
		{
			auto state = ctx.save();
			if (!subject.parse(ctx))
			{
				ctx.restore(state);
			}
			return true;
		}
	};

	template<typename Subject> struct kleene_star
	{
		typedef kleene_star<Subject> peg_type;
		Subject subject;
		template<typename Context> bool parse(Context &ctx)
		{
			auto state = ctx.save();
			if (!subject.parse(ctx))
			{
				ctx.restore(state);
				return true;
			}
			state = ctx.save();
			while (subject.parse(ctx))
			{
				state = ctx.save();
			}
			ctx.restore(state);
			return true;
		}
	};

	template<typename Subject> struct kleene_plus
	{
		typedef kleene_plus<Subject> peg_type;
		Subject subject;
		template<typename Context> bool parse(Context &ctx)
		{
			if (!subject.parse(ctx))
			{
				return false;
			}
			auto state = ctx.save();
			while (subject.parse(ctx))
			{
				state = ctx.save();
			}
			ctx.restore(state);
			return true;
		}
	};

	template<typename Subject> struct and_predicate
	{
		typedef and_predicate<Subject> peg_type;
		Subject subject;
		template<typename Context> bool parse(Context &ctx)
		{
			auto state = ctx.save();
			bool result = subject.parse(ctx);
			ctx.restore(state);
			return result;
		}
	};

	template<typename Subject> struct not_predicate
	{
		typedef not_predicate<Subject> peg_type;
		Subject subject;
		template<typename Context> bool parse(Context &ctx)
		{
			auto state = ctx.save();
			bool result = !subject.parse(ctx);
			ctx.restore(state);
			return result;
		}
	};

	template<typename Left, typename Right> struct sequence
	{
		typedef sequence<Left, Right> peg_type;
		Left left;
		Right right;
		template<typename Context> bool parse(Context &ctx)
		{
			if (!left.parse(ctx))
			{
				return false;
			}
			return right.parse(ctx);
		}
	};

	template<typename Left, typename Right> struct alternative
	{
		typedef alternative<Left, Right> peg_type;
		Left left;
		Right right;
		template<typename Context> bool parse(Context &ctx)
		{
			auto state = ctx.save();
			if (left.parse(ctx))
			{
				return true;
			}
			ctx.restore(state);
			return right.parse(ctx);
		}
	};

	template<typename Subject, typename Action> struct action
	{
		typedef action<Subject, Action> peg_type;
		Subject subject;
		Action action;
		template<typename Context> bool parse(Context &ctx)
		{
			auto state = ctx.save();
			if (!subject.parse(ctx))
			{
				return false;
			}
			action(state, ctx.save());
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

	template<typename Subject> greedy_option<typename Subject::peg_type> operator-(Subject subject)
	{
		return {subject};
	}

	template<typename Subject> kleene_star<typename Subject::peg_type> operator*(Subject subject)
	{
		return {subject};
	}

	template<typename Subject> kleene_plus<typename Subject::peg_type> operator+(Subject subject)
	{
		return {subject};
	}

	template<typename Subject> and_predicate<typename Subject::peg_type> operator&(Subject subject)
	{
		return {subject};
	}

	template<typename Subject> not_predicate<typename Subject::peg_type> operator!(Subject subject)
	{
		return {subject};
	}

	template<typename Left, typename Right> sequence<typename Left::peg_type, typename Right::peg_type> operator>>(Left const &left, Right const &right)
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

	template<typename Grammar, typename Context> bool parse(Grammar g, Context ctx)
	{
		return g.parse(ctx);
	}
}
