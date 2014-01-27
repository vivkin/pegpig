pegpig
======

Recursive-descent C++ parser generator from Parsing Expression Grammar

Operators
---------
Expression|C++|Class|Description
---|---|---|---
 |`eof`|eof_parser|End of input
`.`|`any`|any_parser|Any character
`'{'`|`'{'` or `ch('{')`|char_parser|Literal character
`[0-9]`|`rng["0-9"]`|char_range|Character class
`[eE]`|`set{"eE"}`|char_set|Character class
`"int"`|`"int"` or `lit{"int"}`|literal|Literal string
`(e)`|`(e)`||Grouping
`e?`|`-e`|greedy_option|Optional
`e*`|`*e`|kleen_star|Zero-or-more
`e+`|`+e`|kleen_plus|One-or-more
`&e`|`&e`|and_predicate|And-predicate
`!e`|`!e`|not_predicate| Not-predicate
`e1 e2`|`e1 > e2`|sequence|Sequence
`e1 / e2`|`e1 / e2`|alternative|Prioritized choice
 |`e >= action` or `e % action`|action|Semantic action
