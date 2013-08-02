#pragma once

namespace peg
{
	struct context
	{
		const char *pos;
		const char *end;

		char operator*()
		{
			return *pos;
		}

		context &operator++()
		{
			++pos;
			return *this;
		}

		bool eof()
		{
			return pos == end;
		}
	};

	struct eof
	{
		bool parse(context &ctx)
		{
			return ctx.eof();
		}
	};

	struct any
	{
		bool parse(context &ctx)
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
		char c;
		bool parse(context &ctx)
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
		char min;
		char max;
		bool parse(context &ctx)
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
		const char *cstr;
		bool parse(context &ctx)
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
		const char *cstr;
		bool parse(context &ctx)
		{
			const char *save = ctx.pos;
			for (const char *str = cstr; *str && !ctx.eof(); ++str, ++ctx)
			{
				if (*str != *ctx)
				{
					ctx.pos = save;
					return false;
				}
			}
			return true;
		}
	};

	template<typename Subject> struct greedy_option
	{
		Subject subject;
		bool parse(context &ctx)
		{
			const char *save = ctx.pos;
			if (!subject.parse(ctx))
			{
				ctx.pos = save;
			}
			return true;
		}
	};

	template<typename Subject> struct kleene_star
	{
		Subject subject;
		bool parse(context &ctx)
		{
			const char *save = ctx.pos;
			if (!subject.parse(ctx))
			{
				ctx.pos = save;
				return true;
			}
			save = ctx.pos;
			while (subject.parse(ctx))
			{
				save = ctx.pos;
			}
			ctx.pos = save;
			return true;
		}
	};

	template<typename Subject> struct kleene_plus
	{
		Subject subject;
		bool parse(context &ctx)
		{
			if (!subject.parse(ctx))
			{
				return false;
			}
			const char *save = ctx.pos;
			while (subject.parse(ctx))
			{
				save = ctx.pos;
			}
			ctx.pos = save;
			return true;
		}
	};

	template<typename Subject> struct and_predicate
	{
		Subject subject;
		bool parse(context &ctx)
		{
			const char *save = ctx.pos;
			bool result = subject.parse(ctx);
			ctx.pos = save;
			return result;
		}
	};

	template<typename Subject> struct not_predicate
	{
		Subject subject;
		bool parse(context &ctx)
		{
			const char *save = ctx.pos;
			bool result = !subject.parse(ctx);
			ctx.pos = save;
			return result;
		}
	};

	template<typename LeftType, typename RightType> struct sequence
	{
		LeftType left;
		RightType right;
		bool parse(context &ctx)
		{
			if (!left.parse(ctx))
			{
				return false;
			}
			return right.parse(ctx);
		}
	};

	template<typename LeftType, typename RightType> struct alternative
	{
		LeftType left;
		RightType right;
		bool parse(context &ctx)
		{
			const char *save = ctx.pos;
			if (left.parse(ctx))
			{
				return true;
			}
			ctx.pos = save;
			return right.parse(ctx);
		}
	};

	template<typename Subject, typename Action> struct action
	{
		Subject subject;
		Action action;
		bool parse(context &ctx)
		{
			const char *save = ctx.pos;
			if (!subject.parse(ctx))
			{
				return false;
			}
			action(save, ctx.pos);
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

	template<typename Subject> greedy_option<Subject> operator-(Subject subject)
	{
		return {subject};
	}

	template<typename Subject> kleene_star<Subject> operator*(Subject subject)
	{
		return {subject};
	}

	template<typename Subject> kleene_plus<Subject> operator+(Subject subject)
	{
		return {subject};
	}

	template<typename Subject> and_predicate<Subject> operator&(Subject subject)
	{
		return {subject};
	}

	template<typename Subject> not_predicate<Subject> operator!(Subject subject)
	{
		return {subject};
	}

	template<typename LeftType, typename RightType> sequence<LeftType, RightType> operator>>(LeftType const &left, RightType const &right)
	{
		return {left, right};
	}

	template<typename LeftType, typename RightType> alternative<LeftType, RightType> operator/(LeftType const &left, RightType const &right)
	{
		return {left, right};
	}

	template<typename LeftType, typename RightType> action<LeftType, RightType> operator%(LeftType const &left, RightType const &right)
	{
		return {left, right};
	}

	template<typename Parser> bool parse(const char *begin, const char *end, Parser p)
	{
		context ctx{begin, end};
		return p.parse(ctx);
	}
}
