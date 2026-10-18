# P01 call-site ledger: examples

Baseline: `5327c2b6789e47801a03e663b007fd2a6f9da0c9`. See [method and caveats](p01-call-sites.md). The following fenced TSV is data, not executable source.

233 unique source spans.

```text
file	line	column	start	end	syntax	name	target	first_argument_type	result_type	evidence	action	packet
examples/callables.panack	10	32	169	176	ref	square	ref:examples/callables.panack:square		PureFn[Nat,Nat]	bound-node	retain-explicit-declaration	P11
examples/callables.panack	11	20	196	223	receiver	map	core:$method_map	Array[Nat]	Array[Nat]	bound-node	owned:Array[T].map	P06/P08; callers P10/P11; remove P12
examples/callables.panack	12	3	226	240	call	print	core:print	Array[Nat]	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/callables.panack	13	3	243	275	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/callables.panack	13	9	249	274	receiver	reduce	core:$method_reduce	Array[Nat]	Nat	bound-node	owned:Array[T].reduce	P06/P08; callers P10/P11; remove P12
examples/callables.panack	13	27	267	273	ref	total	ref:examples/callables.panack:total		PureFn[Nat,Nat,Nat]	bound-node	retain-explicit-declaration	P11
examples/callables.panack	14	3	278	302	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/callables.panack	14	9	284	301	receiver	call	core:$method_call	PureFn[Nat,Nat]	Nat	bound-node	owned:callable family.call	P06/P08; callers P10/P11; remove P12
examples/collections_and_bytes.panack	8	21	193	211	receiver	append	core:append	Array[Nat]	Array[Nat]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
examples/collections_and_bytes.panack	9	3	214	229	call	print	core:print	Array[Nat]	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/collections_and_bytes.panack	10	3	232	247	call	print	core:print	Array[Nat]	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/collections_and_bytes.panack	12	31	279	284	call	map	core:map		Map[Str,Nat]	bound-node	retain-constructor	P05/P12
examples/collections_and_bytes.panack	13	13	297	324	receiver	put	core:$method_put	Map[Str,Nat]	Map[Str,Nat]	bound-node	owned:Map[K,V].put	P06/P08; callers P10/P11; remove P12
examples/collections_and_bytes.panack	14	3	327	357	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/collections_and_bytes.panack	14	9	333	356	receiver	get	core:$method_get	Map[Str,Nat]	Nat	bound-node	owned:Map[K,V].get	P06/P08; callers P10/P11; remove P12
examples/collections_and_bytes.panack	16	25	383	388	call	set	core:set		Set[Str]	bound-node	retain-constructor	P05/P12
examples/collections_and_bytes.panack	17	11	399	422	receiver	add	core:$method_add	Set[Str]	Set[Str]	bound-node	owned:Set[T].add	P06/P08; callers P10/P11; remove P12
examples/collections_and_bytes.panack	18	3	425	455	call	print	core:print	Bool	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/collections_and_bytes.panack	18	9	431	454	receiver	has	core:$method_has	Set[Str]	Bool	bound-node	owned:Map[K,V].has / Set[T].has	P06/P08; callers P10/P11; remove P12
examples/collections_and_bytes.panack	20	20	476	528	namespace	push	src/stdlib/bytes.panack:push	Bytes	Bytes	bound-node	Owned: Bytes.push	P08/P10/P11
examples/collections_and_bytes.panack	20	37	493	523	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
examples/collections_and_bytes.panack	21	3	531	565	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/collections_and_bytes.panack	21	9	537	564	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
examples/collections_and_bytes.panack	22	3	568	604	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/collections_and_bytes.panack	22	9	574	603	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
examples/collections_and_bytes.panack	25	11	675	730	receiver	sort_by	core:$core_sort_by	Array[Str]	Array[Str]	bound-node	owned:Array[T].sort_by	P06/P08; callers P10/P11; remove P12
examples/collections_and_bytes.panack	25	53	717	729	ref	name_before	ref:examples/collections_and_bytes.panack:name_before		PureFn[Str,Str,Bool]	bound-node	retain-explicit-declaration	P11
examples/collections_and_bytes.panack	27	8	760	785	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
examples/collections_and_bytes.panack	27	36	788	799	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/decimal.panack	6	3	95	164	call	print	core:print	Dec	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/decimal.panack	6	9	101	163	call	compound	examples/decimal.panack:compound	Dec	Dec	bound-node	retain-explicit-declaration	P11
examples/euler001.panack	10	8	149	171	call	divisible	examples/euler001.panack:divisible	Nat	Bool	bound-node	retain-explicit-declaration	P11
examples/euler001.panack	10	34	175	197	call	divisible	examples/euler001.panack:divisible	Nat	Bool	bound-node	retain-explicit-declaration	P11
examples/euler001.panack	11	18	217	236	call	sum_below	examples/euler001.panack:sum_below	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/euler001.panack	13	7	256	275	call	sum_below	examples/euler001.panack:sum_below	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/euler001.panack	19	3	306	328	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/euler001.panack	19	9	312	327	call	sum_below	examples/euler001.panack:sum_below	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/euler001_iterative.panack	14	3	201	233	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/euler001_iterative.panack	14	9	207	232	call	sum_multiples_below	examples/euler001_iterative.panack:sum_multiples_below	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/euler002.panack	21	3	388	428	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/euler002.panack	21	9	394	427	call	sum_even_fibonacci_below	examples/euler002.panack:sum_even_fibonacci_below	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/euler003.panack	8	19	254	281	call	quotient	core:quotient	Nat	Nat	bound-node	owned:Nat.divide	P06/P08; callers P10/P11; remove P12
examples/euler003.panack	18	3	364	405	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/euler003.panack	18	9	370	404	call	largest_prime_factor	examples/euler003.panack:largest_prime_factor	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/euler004.panack	8	17	258	281	call	quotient	core:quotient	Nat	Nat	bound-node	owned:Nat.divide	P06/P08; callers P10/P11; remove P12
examples/euler004.panack	15	12	359	380	call	reverse_digits	examples/euler004.panack:reverse_digits	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/euler004.panack	24	31	581	611	call	is_numeric_palindrome	examples/euler004.panack:is_numeric_palindrome	Nat	Bool	bound-node	retain-explicit-declaration	P11
examples/euler004.panack	34	3	689	736	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/euler004.panack	34	9	695	735	call	largest_three_digit_palindrome_product	examples/euler004.panack:largest_three_digit_palindrome_product		Nat	bound-node	retain-explicit-declaration	P11
examples/euler005.panack	16	3	325	377	call	quotient	core:quotient	Nat	Nat	bound-node	owned:Nat.divide	P06/P08; callers P10/P11; remove P12
examples/euler005.panack	16	18	340	376	call	greatest_common_divisor	examples/euler005.panack:greatest_common_divisor	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/euler005.panack	22	14	506	542	call	least_common_multiple	examples/euler005.panack:least_common_multiple	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/euler005.panack	28	3	576	612	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/euler005.panack	28	9	582	611	call	smallest_multiple_through	examples/euler005.panack:smallest_multiple_through	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/fibonacci.panack	13	6	334	345	receiver	has	core:$method_has	Map[Nat,Nat]	Bool	bound-node	owned:Map[K,V].has / Set[T].has	P06/P08; callers P10/P11; remove P12
examples/fibonacci.panack	14	5	352	388	call	MemoizedFibonacci	examples/fibonacci.panack:MemoizedFibonacci	Nat	examples/fibonacci.panack:MemoizedFibonacci	bound-node	retain-explicit-declaration	P11
examples/fibonacci.panack	14	23	370	381	receiver	get	core:$method_get	Map[Nat,Nat]	Nat	bound-node	owned:Map[K,V].get	P06/P08; callers P10/P11; remove P12
examples/fibonacci.panack	16	5	414	450	call	MemoizedFibonacci	examples/fibonacci.panack:MemoizedFibonacci	Nat	examples/fibonacci.panack:MemoizedFibonacci	bound-node	retain-explicit-declaration	P11
examples/fibonacci.panack	16	26	435	449	receiver	put	core:$method_put	Map[Nat,Nat]	Map[Nat,Nat]	bound-node	owned:Map[K,V].put	P06/P08; callers P10/P11; remove P12
examples/fibonacci.panack	18	5	476	512	call	MemoizedFibonacci	examples/fibonacci.panack:MemoizedFibonacci	Nat	examples/fibonacci.panack:MemoizedFibonacci	bound-node	retain-explicit-declaration	P11
examples/fibonacci.panack	18	26	497	511	receiver	put	core:$method_put	Map[Nat,Nat]	Map[Nat,Nat]	bound-node	owned:Map[K,V].put	P06/P08; callers P10/P11; remove P12
examples/fibonacci.panack	20	27	550	561	call	previous	examples/fibonacci.panack:previous	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/fibonacci.panack	21	34	595	619	call	previous	examples/fibonacci.panack:previous	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/fibonacci.panack	22	42	661	701	call	fibonacci_memoized	examples/fibonacci.panack:fibonacci_memoized	Nat	examples/fibonacci.panack:MemoizedFibonacci	bound-node	retain-explicit-declaration	P11
examples/fibonacci.panack	23	49	750	813	call	fibonacci_memoized	examples/fibonacci.panack:fibonacci_memoized	Nat	examples/fibonacci.panack:MemoizedFibonacci	bound-node	retain-explicit-declaration	P11
examples/fibonacci.panack	25	5	888	955	call	MemoizedFibonacci	examples/fibonacci.panack:MemoizedFibonacci	Nat	examples/fibonacci.panack:MemoizedFibonacci	bound-node	retain-explicit-declaration	P11
examples/fibonacci.panack	25	30	913	954	receiver	put	core:$method_put	Map[Nat,Nat]	Map[Nat,Nat]	bound-node	owned:Map[K,V].put	P06/P08; callers P10/P11; remove P12
examples/fibonacci.panack	30	31	1033	1061	call	fibonacci_memoized	examples/fibonacci.panack:fibonacci_memoized	Nat	examples/fibonacci.panack:MemoizedFibonacci	bound-node	retain-explicit-declaration	P11
examples/fibonacci.panack	30	53	1055	1060	call	map	core:map		Map[Nat,Nat]	bound-node	retain-constructor	P05/P12
examples/fibonacci.panack	48	27	1329	1352	call	fibonacci_recursive	examples/fibonacci.panack:fibonacci_recursive	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/fibonacci.panack	49	27	1379	1402	call	fibonacci_iterative	examples/fibonacci.panack:fibonacci_iterative	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/fibonacci.panack	50	3	1405	1443	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/fibonacci.panack	51	3	1446	1485	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/fizzbuzz.panack	15	5	227	249	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/fizzbuzz.panack	15	11	233	248	call	fizzbuzz	examples/fizzbuzz.panack:fizzbuzz	Nat	Str	bound-node	retain-explicit-declaration	P11
examples/generic_functions.panack	8	5	176	203	call	repeat	examples/generic_functions.panack:repeat	examples/generic_functions.panack:repeat parameter 0	examples/generic_functions.panack:repeat parameter 0	bound-node	retain-explicit-declaration	P11
examples/generic_functions.panack	15	3	249	268	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/generic_functions.panack	15	9	255	267	call	identity	examples/generic_functions.panack:identity	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/generic_functions.panack	16	3	271	301	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/generic_functions.panack	16	9	277	300	receiver	identity	examples/generic_functions.panack:identity	Str	Str	bound-node	remove-free-function-sugar	P11/P12
examples/generic_functions.panack	17	3	304	351	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/generic_functions.panack	17	9	310	350	namespace	value_or	src/stdlib/option.panack:value_or	Option[Nat]	Nat	bound-node	Owned: Option[T].value_or	P08/P10/P11
examples/generic_functions.panack	17	29	330	346	receiver	first	core:$core_first	Array[Nat]	Option[Nat]	bound-node	owned:Array[T].first	P06/P08; callers P10/P11; remove P12
examples/generic_functions.panack	18	3	354	406	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/generic_functions.panack	18	9	360	405	namespace	value_or	src/stdlib/option.panack:value_or	Option[Str]	Str	bound-node	Owned: Option[T].value_or	P08/P10/P11
examples/generic_functions.panack	18	29	380	395	receiver	first	core:$core_first	Array[Str]	Option[Str]	bound-node	owned:Array[T].first	P06/P08; callers P10/P11; remove P12
examples/generic_functions.panack	19	28	434	454	call	Error	core:Error	Str	Result[Nat,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/generic_functions.panack	20	3	457	494	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/generic_functions.panack	20	9	463	493	namespace	value_or	src/stdlib/result.panack:value_or	Result[Nat,Str]	Nat	bound-node	Owned: Result[T,E].value_or	P08/P10/P11
examples/generic_functions.panack	21	3	497	543	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/generic_functions.panack	21	9	503	542	namespace	value_or	src/stdlib/result.panack:value_or	Result[Nat,Str]	Nat	bound-node	Owned: Result[T,E].value_or	P08/P10/P11
examples/generic_functions.panack	21	38	532	537	call	Ok	core:Ok	Nat	Result[Nat,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/generic_functions.panack	22	3	546	568	call	print	core:print	Bool	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/generic_functions.panack	22	9	552	567	call	repeat	examples/generic_functions.panack:repeat	Bool	Bool	bound-node	retain-explicit-declaration	P11
examples/guards.panack	9	3	121	142	call	print	core:print	Int	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/guards.panack	9	9	127	141	call	triple	examples/guards.panack:triple	examples/guards.panack:Positive	Int	bound-node	retain-explicit-declaration	P11
examples/lexer_foundation.panack	29	18	352	363	call	len	core:len	Str	Nat	bound-node	owned:Str.len / Array[T].len / Bytes.length	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	29	33	367	391	call	is_digit	core:is_digit	Str	Bool	bound-node	owned:Str.is_digit	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	37	18	530	541	call	len	core:len	Str	Nat	bound-node	owned:Str.len / Array[T].len / Bytes.length	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	37	34	546	571	call	is_letter	core:is_letter	Str	Bool	bound-node	owned:Str.is_letter	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	37	63	575	599	call	is_digit	core:is_digit	Str	Bool	bound-node	owned:Str.is_digit	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	47	18	789	800	call	len	core:len	Str	Nat	bound-node	owned:Str.len / Array[T].len / Bytes.length	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	50	8	847	871	call	is_whitespace	core:is_whitespace	Str	Bool	bound-node	owned:Str.is_whitespace	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	52	15	914	933	call	is_digit	core:is_digit	Str	Bool	bound-node	owned:Str.is_digit	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	53	18	953	980	call	scan_number	examples/lexer_foundation.panack:scan_number	Str	Nat	bound-node	retain-explicit-declaration	P11
examples/lexer_foundation.panack	54	19	999	1025	call	slice	core:slice	Str	Str	bound-node	owned:Str.slice	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	55	16	1041	1091	call	append	core:append	Array[examples/lexer_foundation.panack:Token]	Array[examples/lexer_foundation.panack:Token]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	55	31	1056	1090	call	Token	examples/lexer_foundation.panack:Token	examples/lexer_foundation.panack:TokenKind	examples/lexer_foundation.panack:Token	bound-node	retain-explicit-declaration	P11
examples/lexer_foundation.panack	55	37	1062	1070	call	Number	examples/lexer_foundation.panack:Number		examples/lexer_foundation.panack:TokenKind	bound-node	retain-explicit-declaration	P11
examples/lexer_foundation.panack	57	15	1125	1145	call	is_letter	core:is_letter	Str	Bool	bound-node	owned:Str.is_letter	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	58	18	1185	1216	call	scan_identifier	examples/lexer_foundation.panack:scan_identifier	Str	Nat	bound-node	retain-explicit-declaration	P11
examples/lexer_foundation.panack	59	19	1235	1261	call	slice	core:slice	Str	Str	bound-node	owned:Str.slice	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	60	16	1277	1331	call	append	core:append	Array[examples/lexer_foundation.panack:Token]	Array[examples/lexer_foundation.panack:Token]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	60	31	1292	1330	call	Token	examples/lexer_foundation.panack:Token	examples/lexer_foundation.panack:TokenKind	examples/lexer_foundation.panack:Token	bound-node	retain-explicit-declaration	P11
examples/lexer_foundation.panack	60	37	1298	1310	call	Identifier	examples/lexer_foundation.panack:Identifier		examples/lexer_foundation.panack:TokenKind	bound-node	retain-explicit-declaration	P11
examples/lexer_foundation.panack	64	16	1407	1467	call	append	core:append	Array[examples/lexer_foundation.panack:Token]	Array[examples/lexer_foundation.panack:Token]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
examples/lexer_foundation.panack	64	31	1422	1466	call	Token	examples/lexer_foundation.panack:Token	examples/lexer_foundation.panack:TokenKind	examples/lexer_foundation.panack:Token	bound-node	retain-explicit-declaration	P11
examples/lexer_foundation.panack	64	37	1428	1441	call	Punctuation	examples/lexer_foundation.panack:Punctuation		examples/lexer_foundation.panack:TokenKind	bound-node	retain-explicit-declaration	P11
examples/lexer_foundation.panack	69	3	1500	1510	call	Ok	core:Ok	Array[examples/lexer_foundation.panack:Token]	Result[Array[examples/lexer_foundation.panack:Token],Array[examples/lexer_foundation.panack:Diagnostic]]	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/lexer_foundation.panack	83	17	1776	1797	call	kind_name	examples/lexer_foundation.panack:kind_name	examples/lexer_foundation.panack:TokenKind	Str	bound-node	retain-explicit-declaration	P11
examples/lexer_foundation.panack	92	19	1975	1996	call	render_tokens	examples/lexer_foundation.panack:render_tokens	Array[examples/lexer_foundation.panack:Token]	Str	bound-node	retain-explicit-declaration	P11
examples/lexer_foundation.panack	98	3	2064	2102	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/lexer_foundation.panack	98	9	2070	2101	call	render	examples/lexer_foundation.panack:render	Result[Array[examples/lexer_foundation.panack:Token],Array[examples/lexer_foundation.panack:Diagnostic]]	Str	bound-node	retain-explicit-declaration	P11
examples/lexer_foundation.panack	98	16	2077	2100	call	lex	examples/lexer_foundation.panack:lex	Str	Result[Array[examples/lexer_foundation.panack:Token],Array[examples/lexer_foundation.panack:Diagnostic]]	bound-node	retain-explicit-declaration	P11
examples/namespaces/public_api/details.panack	6	3	107	131	call	make_label	examples/namespaces/public_api/details.panack:make_label	Str	Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/public_api/main.panack	4	3	51	81	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/namespaces/public_api/main.panack	4	9	57	80	namespace	trip_label	examples/namespaces/public_api/details.panack:trip_label		Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/budget.panack	6	3	104	118	call	daily_budget	examples/namespaces/trip_planner/budget.panack:daily_budget		Nat	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/budget.panack	6	20	121	129	call	nights	examples/namespaces/trip_planner/budget.panack:nights		Nat	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/budget.panack	10	11	170	187	call	estimated_total	examples/namespaces/trip_planner/budget.panack:estimated_total		Nat	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/budget.panack	11	15	202	210	call	nights	examples/namespaces/trip_planner/budget.panack:nights		Nat	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/budget.panack	16	3	270	314	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/namespaces/trip_planner/itinerary.panack	8	23	233	251	namespace	first_leg	examples/namespaces/trip_planner/routes.panack:first_leg		examples/namespaces/trip_planner/routes.panack:Leg	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/itinerary.panack	9	24	275	294	namespace	second_leg	examples/namespaces/trip_planner/routes.panack:second_leg		examples/namespaces/trip_planner/routes.panack:Leg	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/itinerary.panack	10	16	310	332	namespace	describe	examples/namespaces/trip_planner/routes.panack:describe	examples/namespaces/trip_planner/routes.panack:Leg	Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/itinerary.panack	11	17	349	372	namespace	describe	examples/namespaces/trip_planner/routes.panack:describe	examples/namespaces/trip_planner/routes.panack:Leg	Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/itinerary.panack	16	3	443	479	namespace	describe	examples/namespaces/trip_planner/weather.panack:describe	examples/namespaces/trip_planner/weather.panack:Forecast	Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/itinerary.panack	16	20	460	478	namespace	forecast	examples/namespaces/trip_planner/weather.panack:forecast		examples/namespaces/trip_planner/weather.panack:Forecast	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/itinerary.panack	20	3	515	535	call	budget_description	examples/namespaces/trip_planner/budget.panack:describe		Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/itinerary.panack	24	11	575	588	call	destination	examples/namespaces/trip_planner/itinerary.panack:destination		Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/itinerary.panack	25	10	598	618	call	budget_description	examples/namespaces/trip_planner/budget.panack:describe		Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/itinerary.panack	30	3	662	709	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/namespaces/trip_planner/main.panack	5	3	132	162	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/namespaces/trip_planner/main.panack	5	9	138	161	namespace	destination	examples/namespaces/trip_planner/itinerary.panack:destination		Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/main.panack	6	3	165	194	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/namespaces/trip_planner/main.panack	6	9	171	193	namespace	route_line	examples/namespaces/trip_planner/itinerary.panack:route_line		Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/main.panack	7	3	197	228	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/namespaces/trip_planner/main.panack	7	9	203	227	namespace	weather_line	examples/namespaces/trip_planner/itinerary.panack:weather_line		Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/main.panack	8	3	231	261	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/namespaces/trip_planner/main.panack	8	9	237	260	namespace	budget_line	examples/namespaces/trip_planner/itinerary.panack:budget_line		Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/main.panack	9	3	264	290	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/namespaces/trip_planner/main.panack	9	9	270	289	namespace	summary	examples/namespaces/trip_planner/itinerary.panack:summary		Str	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/routes.panack	8	3	87	119	call	Leg	examples/namespaces/trip_planner/routes.panack:Leg	Str	examples/namespaces/trip_planner/routes.panack:Leg	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/routes.panack	12	3	154	184	call	Leg	examples/namespaces/trip_planner/routes.panack:Leg	Str	examples/namespaces/trip_planner/routes.panack:Leg	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/routes.panack	23	3	361	405	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/namespaces/trip_planner/weather.panack	8	3	114	146	call	Forecast	examples/namespaces/trip_planner/weather.panack:Forecast	Str	examples/namespaces/trip_planner/weather.panack:Forecast	bound-node	retain-explicit-declaration	P11
examples/namespaces/trip_planner/weather.panack	19	3	369	414	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/namespaces/twin_sensors/main.panack	5	31	120	136	namespace	latest	examples/namespaces/twin_sensors/weather.panack:latest		examples/namespaces/twin_sensors/weather.panack:Reading	bound-node	retain-explicit-declaration	P11
examples/namespaces/twin_sensors/main.panack	6	34	170	189	namespace	latest	examples/namespaces/twin_sensors/navigation.panack:latest		examples/namespaces/twin_sensors/navigation.panack:Reading	bound-node	retain-explicit-declaration	P11
examples/namespaces/twin_sensors/main.panack	11	3	318	370	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/namespaces/twin_sensors/main.panack	12	3	373	422	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/namespaces/twin_sensors/navigation.panack	6	30	79	96	call	Reading	examples/namespaces/twin_sensors/navigation.panack:Reading	Nat	examples/namespaces/twin_sensors/navigation.panack:Reading	bound-node	retain-explicit-declaration	P11
examples/namespaces/twin_sensors/weather.panack	6	30	79	95	call	Reading	examples/namespaces/twin_sensors/weather.panack:Reading	Nat	examples/namespaces/twin_sensors/weather.panack:Reading	bound-node	retain-explicit-declaration	P11
examples/network/http_server/src/main.panack	7	3	264	411	call	Ok	core:Ok	packages/http/src/types.panack:Response	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/network/http_server/src/main.panack	7	6	267	410	namespace	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
examples/network/http_server/src/main.panack	8	6	291	347	namespace	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
examples/network/http_server/src/main.panack	9	5	354	409	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
examples/network/http_server/src/main.panack	14	3	493	495	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
examples/network/http_server/src/main.panack	18	18	537	636	namespace	serve	packages/http/src/server.panack:serve	Str	Result[Array[Result[Unit,packages/http/src/types.panack:HttpError]],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
examples/network/http_server/src/main.panack	18	48	567	575	ref	respond	ref:examples/network/http_server/src/main.panack:respond		AsyncFn[packages/http/src/types.panack:Request,Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]]	bound-node	retain-explicit-declaration	P11
examples/network/http_server/src/main.panack	19	5	581	635	namespace	ServerLimits	packages/http/src/server.panack:ServerLimits	Nat	packages/http/src/server.panack:ServerLimits	bound-node	retain-explicit-declaration	P11
examples/network/http_server/src/main.panack	21	23	676	689	call	expect	examples/network/http_server/src/main.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
examples/network/http_server/src/main.panack	23	7	718	744	call	expect	examples/network/http_server/src/main.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
examples/network/http_server/src/main.panack	23	14	725	738	receiver	len	core:len	Array[Result[Unit,packages/http/src/types.panack:HttpError]]	Nat	bound-node	owned:Str.len / Array[T].len / Bytes.length	P06/P08; callers P10/P11; remove P12
examples/network/http_server/src/main.panack	24	38	782	784	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
examples/network/http_server/src/main.panack	24	60	804	817	call	expect	examples/network/http_server/src/main.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
examples/network/http_server/src/main.panack	27	3	832	834	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
examples/network/tcp_exchange.panack	7	3	344	346	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
examples/network/tcp_exchange.panack	11	9	406	458	call	tcp_exchange	core:tcp_exchange	Str	Result[Bytes,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/network/tcp_exchange.panack	15	13	495	538	call	utf8_encode	core:utf8_encode	Str	Bytes	bound-node	owned:Str.encode_utf8	P06/P08; callers P10/P11; remove P12
examples/network/tcp_exchange.panack	16	15	553	566	call	echo	examples/network/tcp_exchange.panack:echo	Bytes	Result[Bytes,Str]	bound-node	retain-explicit-declaration	P11
examples/network/tcp_exchange.panack	17	21	589	616	call	expect	examples/network/tcp_exchange.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
examples/network/tcp_exchange.panack	18	23	640	653	call	expect	examples/network/tcp_exchange.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
examples/network/tcp_exchange.panack	20	3	660	662	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
examples/network/tcp_serve.panack	7	3	319	330	call	Ok	core:Ok	Bytes	Result[Bytes,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/network/tcp_serve.panack	12	3	412	414	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
examples/network/tcp_serve.panack	16	12	450	502	call	TcpServerLimits	src/stdlib/tcp.panack:TcpServerLimits	Nat	src/stdlib/tcp.panack:TcpServerLimits	bound-node	retain-explicit-declaration	P11
examples/network/tcp_serve.panack	17	15	517	561	call	tcp_serve	core:tcp_serve	Str	Result[Array[Result[Unit,Str]],Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/network/tcp_serve.panack	17	44	546	552	ref	reply	ref:examples/network/tcp_serve.panack:reply		AsyncFn[Bytes,Result[Bytes,Str]]	bound-node	retain-explicit-declaration	P11
examples/network/tcp_serve.panack	19	7	592	619	call	expect	examples/network/tcp_serve.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
examples/network/tcp_serve.panack	19	14	599	613	receiver	len	core:len	Array[Result[Unit,Str]]	Nat	bound-node	owned:Str.len / Array[T].len / Bytes.length	P06/P08; callers P10/P11; remove P12
examples/network/tcp_serve.panack	20	39	658	660	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
examples/network/tcp_serve.panack	20	61	680	693	call	expect	examples/network/tcp_serve.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
examples/network/tcp_serve.panack	22	23	725	738	call	expect	examples/network/tcp_serve.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
examples/network/tcp_serve.panack	24	3	745	747	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
examples/option_result.panack	11	5	229	254	call	Error	core:Error	Str	Result[Nat,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/option_result.panack	13	5	270	306	call	Ok	core:Ok	Nat	Result[Nat,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/option_result.panack	13	8	273	305	call	quotient	core:quotient	Nat	Nat	bound-node	owned:Nat.divide	P06/P08; callers P10/P11; remove P12
examples/option_result.panack	25	3	466	494	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/option_result.panack	25	9	472	493	call	value_or	examples/option_result.panack:value_or	Option[Nat]	Nat	bound-node	retain-explicit-declaration	P11
examples/option_result.panack	25	18	481	489	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/option_result.panack	26	3	497	523	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/option_result.panack	26	9	503	522	call	value_or	examples/option_result.panack:value_or	Option[Nat]	Nat	bound-node	retain-explicit-declaration	P11
examples/option_result.panack	26	18	512	518	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/option_result.panack	27	3	526	561	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/option_result.panack	27	9	532	560	call	describe	examples/option_result.panack:describe	Result[Nat,Str]	Str	bound-node	retain-explicit-declaration	P11
examples/option_result.panack	27	18	541	559	call	safe_divide	examples/option_result.panack:safe_divide	Nat	Result[Nat,Str]	bound-node	retain-explicit-declaration	P11
examples/option_result.panack	28	3	564	598	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/option_result.panack	28	9	570	597	call	describe	examples/option_result.panack:describe	Result[Nat,Str]	Str	bound-node	retain-explicit-declaration	P11
examples/option_result.panack	28	18	579	596	call	safe_divide	examples/option_result.panack:safe_divide	Nat	Result[Nat,Str]	bound-node	retain-explicit-declaration	P11
examples/palindrome.panack	8	20	231	240	call	len	core:len	Str	Nat	bound-node	owned:Str.len / Array[T].len / Bytes.length	P06/P08; callers P10/P11; remove P12
examples/palindrome.panack	12	13	328	343	call	previous	examples/palindrome.panack:previous	Nat	Nat	bound-node	retain-explicit-declaration	P11
examples/palindrome.panack	24	18	509	528	call	is_palindrome	examples/palindrome.panack:is_palindrome	Str	Bool	bound-node	retain-explicit-declaration	P11
examples/palindrome.panack	29	3	572	598	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/palindrome.panack	29	9	578	597	call	describe	examples/palindrome.panack:describe	Str	Str	bound-node	retain-explicit-declaration	P11
examples/palindrome.panack	30	3	601	630	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/palindrome.panack	30	9	607	629	call	describe	examples/palindrome.panack:describe	Str	Str	bound-node	retain-explicit-declaration	P11
examples/palindrome.panack	31	3	633	666	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/palindrome.panack	31	9	639	665	call	describe	examples/palindrome.panack:describe	Str	Str	bound-node	retain-explicit-declaration	P11
examples/palindrome.panack	32	3	669	688	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/palindrome.panack	32	9	675	687	call	describe	examples/palindrome.panack:describe	Str	Str	bound-node	retain-explicit-declaration	P11
examples/strings.panack	7	3	158	190	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/strings.panack	7	9	164	189	call	describe	examples/strings.panack:describe	Str	Str	bound-node	retain-explicit-declaration	P11
examples/strings.panack	8	3	193	224	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/strings.panack	8	9	199	223	receiver	slice	core:slice	Str	Str	bound-node	owned:Str.slice	P06/P08; callers P10/P11; remove P12
examples/strings.panack	9	3	227	265	call	print	core:print	Bool	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/strings.panack	9	9	233	264	receiver	starts_with	core:starts_with	Str	Bool	bound-node	owned:Str.starts_with	P06/P08; callers P10/P11; remove P12
examples/strings.panack	10	3	268	290	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/strings.panack	10	9	274	289	receiver	reverse	core:reverse	Str	Str	bound-node	owned:Str.reverse	P06/P08; callers P10/P11; remove P12
examples/vm_arithmetic.panack	3	3	40	53	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/vm_branch_call.panack	10	3	111	129	call	print	core:print	Str	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
examples/vm_branch_call.panack	10	9	117	128	call	classify	examples/vm_branch_call.panack:classify	Nat	Str	bound-node	retain-explicit-declaration	P11
examples/vm_loop.panack	6	3	88	100	call	print	core:print	Nat	Void	bound-node	explicit-service/internal-primitive	P10/P11/P12
```
