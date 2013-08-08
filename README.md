pegpig
======

Recursive-descent C++ parser generator from Parsing Expression Grammar

Operators
---------
Expression|C++|Class|Description
---|---|---|---
 ||eof|End of input
`.`||any|Any character
`'{'`|`'{'_ch`|one|Literal character
`[0-9]`|`"[0-9]"_rng`|range|Character class
`[eE]`|`"eE"_set`|set|Character class
`"unsigned"`|`"unsigned"_lit`|literal|Literal string
`(e)`|`(e)`||Grouping
`e?`|`-e`|greedy_option|Optional
`e*`|`*e`|kleen_star|Zero-or-more
`e+`|`+e`|kleen_plus|One-or-more
`&e`|`&e`|and_predicate|And-predicate
`!e`|`!e`|not_predicate| Not-predicate
`e1 e2`|`e1 > e2`|sequence|Sequence
`e1 / e2`|`e1 / e2`|alternative|Prioritized choice
 |`e % action`|action|Semantic action
