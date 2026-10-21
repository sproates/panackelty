# P01 call-site ledger: tools

Baseline: `5327c2b6789e47801a03e663b007fd2a6f9da0c9`. See [method and caveats](p01-call-sites.md). The following fenced TSV is data, not executable source.

40 unique source spans.

```text
file	line	column	start	end	syntax	name	target	first_argument_type	result_type	evidence	action	packet
tools/http-server/src/main.panack	8	11	215	238	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
tools/http-server/src/main.panack	9	15	253	273	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
tools/http-server/src/main.panack	9	43	281	301	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
tools/http-server/src/main.panack	11	19	345	365	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
tools/http-server/src/main.panack	12	13	380	403	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
tools/http-server/src/main.panack	19	43	594	602	call	Ok	core:Ok	Nat	Result[Nat,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
tools/http-server/src/main.panack	20	10	614	662	call	Error	core:Error	Str	Result[Nat,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
tools/http-server/src/main.panack	24	14	749	763	namespace	current	src/stdlib/path.panack:current		Path	bound-node	Retain explicit module function	P08/P10/P11
tools/http-server/src/main.panack	30	18	910	924	call	len	core:len	Array[Str]	Nat	bound-node	owned:Str.len / Array[T].len / Bytes.length	P06/P08; callers P10/P11; remove P12
tools/http-server/src/main.panack	33	17	1027	1086	receiver	append	core:append	Array[packages/http/src/types.panack:Header]	Array[packages/http/src/types.panack:Header]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
tools/http-server/src/main.panack	33	32	1042	1085	namespace	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
tools/http-server/src/main.panack	36	17	1169	1226	receiver	append	core:append	Array[packages/http/src/types.panack:Header]	Array[packages/http/src/types.panack:Header]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
tools/http-server/src/main.panack	36	32	1184	1225	namespace	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
tools/http-server/src/main.panack	39	24	1361	1375	call	len	core:len	Array[Str]	Nat	bound-node	owned:Str.len / Array[T].len / Bytes.length	P06/P08; callers P10/P11; remove P12
tools/http-server/src/main.panack	44	17	1531	1552	namespace	from_text	src/stdlib/path.panack:from_text	Str	Result[Path,src/stdlib/path.panack:PathError]	bound-node	Retain explicit module function	P08/P10/P11
tools/http-server/src/main.panack	51	17	1778	1796	call	port_number	tools/http-server/src/main.panack:port_number	Str	Result[Nat,Str]	bound-node	retain-explicit-declaration	P11
tools/http-server/src/main.panack	62	22	2044	2058	call	Error	core:Error	Str	Result[packages/static_http/src/service.panack:Config,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
tools/http-server/src/main.panack	64	5	2074	2177	call	Ok	core:Ok	packages/static_http/src/service.panack:Config	Result[packages/static_http/src/service.panack:Config,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
tools/http-server/src/main.panack	64	8	2077	2176	namespace	Config	packages/static_http/src/service.panack:Config	Path	packages/static_http/src/service.panack:Config	bound-node	retain-explicit-declaration	P11
tools/http-server/src/main.panack	64	44	2113	2166	namespace	ServiceLimits	packages/http/src/service.panack:ServiceLimits	Nat	packages/http/src/service.panack:ServiceLimits	bound-node	retain-explicit-declaration	P11
tools/http-server/src/main.panack	71	15	2319	2368	namespace	stdout	src/stdlib/async_host.panack:stdout	Str	Result[Unit,src/stdlib/host.panack:HostError]	bound-node	Retain explicit module function	P08/P10/P11
tools/http-server/src/main.panack	72	17	2387	2393	call	Ok	core:Ok	Unit	Result[Unit,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
tools/http-server/src/main.panack	72	20	2390	2392	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
tools/http-server/src/main.panack	73	23	2417	2455	call	Error	core:Error	Str	Result[Unit,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
tools/http-server/src/main.panack	78	21	2504	2520	namespace	arguments	src/stdlib/async_host.panack:arguments		Array[Str]	bound-node	Retain explicit module function	P08/P10/P11
tools/http-server/src/main.panack	79	6	2526	2540	call	len	core:len	Array[Str]	Nat	bound-node	owned:Str.len / Array[T].len / Bytes.length	P06/P08; callers P10/P11; remove P12
tools/http-server/src/main.panack	80	21	2596	2772	namespace	stdout	src/stdlib/async_host.panack:stdout	Str	Result[Unit,src/stdlib/host.panack:HostError]	bound-node	Retain explicit module function	P08/P10/P11
tools/http-server/src/main.panack	81	5	2777	2779	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
tools/http-server/src/main.panack	82	13	2792	2806	call	len	core:len	Array[Str]	Nat	bound-node	owned:Str.len / Array[T].len / Bytes.length	P06/P08; callers P10/P11; remove P12
tools/http-server/src/main.panack	83	21	2865	2904	namespace	stdout	src/stdlib/async_host.panack:stdout	Str	Result[Unit,src/stdlib/host.panack:HostError]	bound-node	Retain explicit module function	P08/P10/P11
tools/http-server/src/main.panack	84	5	2909	2911	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
tools/http-server/src/main.panack	86	11	2933	2957	call	configuration	tools/http-server/src/main.panack:configuration	Array[Str]	Result[packages/static_http/src/service.panack:Config,Str]	bound-node	retain-explicit-declaration	P11
tools/http-server/src/main.panack	88	25	3010	3048	namespace	stderr	src/stdlib/async_host.panack:stderr	Str	Result[Unit,src/stdlib/host.panack:HostError]	bound-node	Retain explicit module function	P08/P10/P11
tools/http-server/src/main.panack	89	15	3063	3075	namespace	exit	src/stdlib/async_host.panack:exit	Nat	Unit	bound-node	Retain explicit module function	P08/P10/P11
tools/http-server/src/main.panack	91	35	3119	3150	namespace	serve	packages/static_http/src/service.panack:serve	packages/static_http/src/service.panack:Config	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,Str]	bound-node	retain-explicit-declaration	P11
tools/http-server/src/main.panack	91	59	3143	3149	ref	ready	ref:tools/http-server/src/main.panack:ready		AsyncFn[src/stdlib/tcp_service.panack:TcpServiceReady,Result[Unit,Str]]	bound-node	retain-explicit-declaration	P11
tools/http-server/src/main.panack	93	27	3207	3245	namespace	stderr	src/stdlib/async_host.panack:stderr	Str	Result[Unit,src/stdlib/host.panack:HostError]	bound-node	Retain explicit module function	P08/P10/P11
tools/http-server/src/main.panack	94	17	3262	3274	namespace	exit	src/stdlib/async_host.panack:exit	Nat	Unit	bound-node	Retain explicit module function	P08/P10/P11
tools/http-server/src/main.panack	96	26	3311	3313	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
tools/http-server/src/main.panack	100	3	3336	3338	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
```
