# P01 call-site ledger: packages

Baseline: `5327c2b6789e47801a03e663b007fd2a6f9da0c9`. See [method and caveats](p01-call-sites.md). The following fenced TSV is data, not executable source.

1006 unique source spans.

```text
file	line	column	start	end	syntax	name	target	first_argument_type	result_type	evidence	action	packet
packages/http/src/client.panack	26	38	686	710	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/client.panack	27	46	758	785	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/http/src/client.panack	30	23	869	888	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/client.panack	33	14	946	993	namespace	push	src/stdlib/bytes.panack:push	Bytes	Bytes	bound-node	Owned: Bytes.push	P08/P10/P11
packages/http/src/client.panack	33	39	971	992	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	36	3	1024	1048	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/http/src/client.panack	40	15	1109	1126	call	byte_length	packages/http/src/client.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	52	17	1439	1460	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	54	51	1531	1556	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	58	22	1686	1715	receiver	append	core:append	Array[Nat]	Array[Nat]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/http/src/client.panack	82	49	2581	2604	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	84	18	2671	2700	receiver	append	core:append	Array[Nat]	Array[Nat]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/http/src/client.panack	85	8	2708	2724	receiver	len	core:len	Array[Nat]	Nat	bound-node	owned:Str.len / Array[T].len / Bytes.length	P06/P08; callers P10/P11; remove P12
packages/http/src/client.panack	92	21	2863	2897	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	92	27	2869	2896	call	InvalidUrl	packages/http/src/types.panack:InvalidUrl	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	98	19	3019	3040	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	109	31	3333	3386	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	109	37	3339	3385	call	InvalidUrl	packages/http/src/types.panack:InvalidUrl	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	110	12	3400	3409	call	Ok	core:Ok	Nat	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	115	17	3491	3512	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/client.panack	116	15	3527	3544	call	byte_length	packages/http/src/client.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	121	17	3657	3678	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	126	15	3816	3850	call	starts_with_at	core:starts_with_at	Str	Bool	bound-node	owned:Str.starts_with_at	P06/P08; callers P10/P11; remove P12
packages/http/src/client.panack	127	47	3897	3930	call	starts_with_at	core:starts_with_at	Str	Bool	bound-node	owned:Str.starts_with_at	P06/P08; callers P10/P11; remove P12
packages/http/src/client.panack	129	5	4023	4077	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/client.panack:ParsedUrl,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	129	11	4029	4076	call	InvalidUrl	packages/http/src/types.panack:InvalidUrl	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	132	35	4161	4189	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	133	9	4207	4235	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	133	47	4245	4273	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	137	30	4381	4387	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	141	19	4500	4521	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	144	21	4608	4614	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	144	38	4625	4637	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	149	7	4769	4838	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/client.panack:ParsedUrl,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	149	13	4775	4837	call	InvalidUrl	packages/http/src/types.panack:InvalidUrl	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	150	15	4853	4881	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	151	7	4915	4939	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/client.panack:ParsedUrl,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	151	13	4921	4938	call	UnsupportedHost	packages/http/src/types.panack:UnsupportedHost		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	154	31	5073	5092	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/client.panack	157	22	5189	5245	namespace	push	src/stdlib/bytes.panack:push	Bytes	Bytes	bound-node	Owned: Bytes.push	P08/P10/P11
packages/http/src/client.panack	157	51	5218	5244	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	160	11	5302	5330	call	ascii_host_valid	packages/http/src/client.panack:ascii_host_valid	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	161	9	5341	5365	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/client.panack:ParsedUrl,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	161	15	5347	5364	call	UnsupportedHost	packages/http/src/types.panack:UnsupportedHost		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	164	21	5460	5490	call	Ok	core:Ok	Nat	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	165	29	5520	5565	call	parse_port	packages/http/src/client.panack:parse_port	Bytes	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	168	27	5630	5642	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/client.panack:ParsedUrl,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	171	18	5751	5779	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	172	23	5810	5847	call	slice_text	packages/http/src/client.panack:slice_text	Bytes	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	174	17	5887	5924	call	slice_text	packages/http/src/client.panack:slice_text	Bytes	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	177	35	5989	6013	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/client.panack	178	32	6045	6070	call	byte_length	packages/http/src/client.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	178	65	6078	6102	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	180	28	6159	6184	call	byte_length	packages/http/src/client.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	181	27	6213	6242	call	byte_at	packages/http/src/client.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	186	15	6402	6476	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/client.panack:ParsedUrl,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	186	21	6408	6475	call	InvalidUrl	packages/http/src/types.panack:InvalidUrl	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	188	27	6524	6552	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/http/src/client.panack	189	46	6598	6604	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	190	15	6655	6706	call	Ok	core:Ok	packages/http/src/client.panack:ParsedUrl	Result[packages/http/src/client.panack:ParsedUrl,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	190	18	6658	6705	call	ParsedUrl	packages/http/src/client.panack:ParsedUrl	Str	packages/http/src/client.panack:ParsedUrl	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	200	37	6848	6860	call	DnsFailure	packages/http/src/types.panack:DnsFailure		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	201	49	6911	6927	call	ConnectFailure	packages/http/src/types.panack:ConnectFailure		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	202	87	7016	7028	call	TlsFailure	packages/http/src/types.panack:TlsFailure		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	203	51	7081	7090	call	Timeout	packages/http/src/types.panack:Timeout		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	204	55	7147	7172	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	205	10	7184	7225	call	TransportFailure	packages/http/src/types.panack:TransportFailure	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	209	9	7293	7350	call	get_with_limits	packages/http/src/client.panack:get_with_limits	Str	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	209	30	7314	7349	call	ClientLimits	packages/http/src/client.panack:ClientLimits	Nat	packages/http/src/client.panack:ClientLimits	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	215	5	7627	7649	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	215	11	7633	7648	call	InvalidLimits	packages/http/src/types.panack:InvalidLimits		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	217	11	7671	7685	call	parse_url	packages/http/src/client.panack:parse_url	Str	Result[packages/http/src/client.panack:ParsedUrl,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	218	23	7710	7722	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	222	26	7853	8003	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/client.panack	225	40	8043	8137	call	exchange	packages/http/src/transport.panack:exchange	Str	Result[Bytes,Str]	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	228	29	8186	8217	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/client.panack	228	35	8192	8216	call	transport_error	packages/http/src/client.panack:transport_error	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/client.panack	229	24	8242	8303	call	parse_response	packages/http/src/parser.panack:parse_response	Bytes	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	37	38	835	859	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/parser.panack	38	46	907	934	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/http/src/parser.panack	41	28	1032	1038	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	43	40	1099	1105	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	44	8	1115	1136	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	44	39	1146	1171	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	45	15	1194	1206	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	54	28	1343	1349	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	56	22	1393	1410	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	56	52	1423	1429	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	57	8	1439	1460	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	57	39	1470	1495	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	58	9	1513	1538	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	58	44	1548	1573	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	59	15	1596	1608	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	68	23	1751	1770	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/parser.panack	71	14	1828	1875	namespace	push	src/stdlib/bytes.panack:push	Bytes	Bytes	bound-node	Owned: Bytes.push	P08/P10/P11
packages/http/src/parser.panack	71	39	1853	1874	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	78	3	1976	2022	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/http/src/parser.panack	78	20	1993	2021	call	copy_bytes	packages/http/src/parser.panack:copy_bytes	Bytes	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	86	3	2264	2282	call	ascii_digit	packages/http/src/parser.panack:ascii_digit	Nat	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	86	25	2286	2304	call	ascii_upper	packages/http/src/parser.panack:ascii_upper	Nat	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	86	47	2308	2326	call	ascii_lower	packages/http/src/parser.panack:ascii_lower	Nat	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	93	23	2627	2646	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/parser.panack	95	20	2688	2711	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser.panack	96	18	2729	2749	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	97	17	2768	2792	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	98	14	2806	2880	namespace	push	src/stdlib/bytes.panack:push	Bytes	Bytes	bound-node	Owned: Bytes.push	P08/P10/P11
packages/http/src/parser.panack	98	42	2834	2851	call	ascii_upper	packages/http/src/parser.panack:ascii_upper	Nat	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	101	3	2911	2935	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/http/src/parser.panack	106	26	3047	3068	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	106	57	3078	3099	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	116	8	3286	3307	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	116	39	3317	3338	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	125	22	3532	3552	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	125	52	3562	3582	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	126	5	3594	3642	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Header,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	126	11	3600	3641	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	128	30	3683	3689	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	130	36	3748	3754	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	131	10	3766	3787	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	131	48	3804	3816	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	135	17	3894	3948	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Header,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	135	23	3900	3947	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	140	15	4094	4132	call	token_byte	packages/http/src/parser.panack:token_byte	Nat	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	140	26	4105	4131	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	145	29	4306	4333	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	152	11	4536	4593	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Header,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	152	17	4542	4592	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	154	30	4640	4676	call	trim_ows_start	packages/http/src/parser.panack:trim_ows_start	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	155	28	4704	4740	call	trim_ows_end	packages/http/src/parser.panack:trim_ows_end	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	156	11	4751	4835	call	Ok	core:Ok	packages/http/src/types.panack:Header	Result[packages/http/src/types.panack:Header,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	156	14	4754	4834	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	156	21	4761	4791	call	ascii_text	packages/http/src/parser.panack:ascii_text	Bytes	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	156	53	4793	4833	call	ascii_text	packages/http/src/parser.panack:ascii_text	Bytes	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	164	21	4978	5022	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	164	27	4984	5021	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	171	19	5187	5208	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	184	20	5509	5545	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	184	26	5515	5544	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	185	23	5570	5616	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	185	29	5576	5615	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	186	12	5630	5639	call	Ok	core:Ok	Nat	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	191	28	5756	5762	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	192	29	5791	5797	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	195	8	5850	5871	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	196	19	5898	5904	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	196	36	5915	5927	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	197	25	5954	5960	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	197	43	5972	5984	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	198	23	6009	6022	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	203	15	6089	6138	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:RequestLine,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	203	21	6095	6137	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	205	17	6195	6244	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:RequestLine,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	205	23	6201	6243	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	208	9	6393	6442	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:RequestLine,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	208	15	6399	6441	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	213	15	6582	6622	call	token_byte	packages/http/src/parser.panack:token_byte	Nat	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	213	26	6593	6621	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	219	23	6842	6870	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	230	34	7283	7312	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	231	13	7334	7363	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	231	52	7373	7402	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	232	13	7424	7453	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	232	52	7463	7492	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	235	25	7604	7633	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	252	11	8263	8312	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:RequestLine,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	252	17	8269	8311	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	253	19	8331	8368	call	ascii_text	packages/http/src/parser.panack:ascii_text	Bytes	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	254	11	8395	8459	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:RequestLine,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	254	17	8401	8458	call	UnsupportedVersion	packages/http/src/types.panack:UnsupportedVersion	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	254	36	8420	8457	call	ascii_text	packages/http/src/parser.panack:ascii_text	Bytes	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	256	11	8487	8593	call	Ok	core:Ok	packages/http/src/parser.panack:RequestLine	Result[packages/http/src/parser.panack:RequestLine,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	256	14	8490	8592	call	RequestLine	packages/http/src/parser.panack:RequestLine	Str	packages/http/src/parser.panack:RequestLine	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	256	26	8502	8533	call	ascii_text	packages/http/src/parser.panack:ascii_text	Bytes	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	257	13	8547	8591	call	ascii_text	packages/http/src/parser.panack:ascii_text	Bytes	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	265	9	8731	8765	call	find_crlf	packages/http/src/parser.panack:find_crlf	Bytes	Option[Nat]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	266	15	8782	8831	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:RequestHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	266	21	8788	8830	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	267	29	8861	8895	call	parse_request_line	packages/http/src/parser.panack:parse_request_line	Bytes	Result[packages/http/src/parser.panack:RequestLine,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	268	23	8920	8932	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:RequestHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	274	43	9136	9142	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	278	41	9301	9307	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	279	48	9355	9361	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	280	17	9380	9419	call	find_crlf	packages/http/src/parser.panack:find_crlf	Bytes	Option[Nat]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	281	34	9455	9508	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	281	39	9460	9507	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	282	32	9543	9574	call	parse_header	packages/http/src/parser.panack:parse_header	Bytes	Result[packages/http/src/types.panack:Header,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	283	42	9618	9629	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	285	29	9691	9715	call	lower_ascii	packages/http/src/parser.panack:lower_ascii	Str	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	288	52	9849	9889	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	288	57	9854	9888	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	290	40	9984	9990	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	291	30	10022	10069	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	291	35	10027	10068	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	293	27	10123	10252	call	parse_decimal	packages/http/src/parser.panack:parse_decimal	Bytes	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	293	41	10137	10167	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser.panack	294	25	10196	10239	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	294	37	10208	10238	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser.panack	295	50	10304	10315	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	296	56	10374	10386	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	301	30	10561	10596	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	301	35	10566	10595	call	UnsupportedTransferEncoding	packages/http/src/types.panack:UnsupportedTransferEncoding		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	304	47	10719	10744	call	lower_ascii	packages/http/src/parser.panack:lower_ascii	Str	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	307	27	10822	10844	receiver	append	core:append	Array[packages/http/src/types.panack:Header]	Array[packages/http/src/types.panack:Header]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/http/src/parser.panack	313	22	10951	10957	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	313	61	10990	11002	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:RequestHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	313	85	11014	11037	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:RequestHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	313	91	11020	11036	call	InvalidRequest	packages/http/src/types.panack:InvalidRequest		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	314	35	11076	11133	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:RequestHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	314	41	11082	11132	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	315	69	11204	11240	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:RequestHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	315	75	11210	11239	call	UnsupportedTransferEncoding	packages/http/src/types.panack:UnsupportedTransferEncoding		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	316	60	11302	11308	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	317	11	11321	11392	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:RequestHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	317	17	11327	11391	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	320	11	11428	11523	call	Ok	core:Ok	packages/http/src/parser.panack:RequestHead	Result[packages/http/src/parser.panack:RequestHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	320	14	11431	11522	call	RequestHead	packages/http/src/parser.panack:RequestHead	Str	packages/http/src/parser.panack:RequestHead	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	329	24	11661	11680	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	329	53	11690	11709	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	339	8	11902	11923	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	342	16	11980	12017	call	skip_chunk_ows	packages/http/src/parser.panack:skip_chunk_ows	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	344	29	12077	12110	call	token_byte	packages/http/src/parser.panack:token_byte	Nat	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	344	40	12088	12109	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	350	18	12235	12268	call	skip_chunk_ows	packages/http/src/parser.panack:skip_chunk_ows	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	351	28	12296	12317	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	352	20	12345	12382	call	skip_chunk_ows	packages/http/src/parser.panack:skip_chunk_ows	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	353	30	12412	12433	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	357	27	12584	12605	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	365	34	12882	12903	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	377	35	13344	13377	call	token_byte	packages/http/src/parser.panack:token_byte	Nat	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	377	46	13355	13376	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	382	20	13514	13547	call	skip_chunk_ows	packages/http/src/parser.panack:skip_chunk_ows	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	384	28	13585	13606	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	396	8	13873	13894	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	401	31	14076	14120	call	chunk_extensions_valid	packages/http/src/parser.panack:chunk_extensions_valid	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	405	17	14222	14243	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	406	41	14284	14290	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	407	28	14320	14335	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	408	27	14364	14370	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	409	28	14400	14415	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	410	27	14444	14450	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	411	29	14481	14496	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	412	14	14512	14518	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	421	17	14719	14740	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	425	16	14834	14876	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	425	22	14840	14875	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	426	30	14908	14944	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	426	36	14914	14943	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	427	10	14956	14964	call	Ok	core:Ok	Nat	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	435	35	15209	15215	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	436	46	15261	15267	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	437	11	15280	15322	call	find_crlf	packages/http/src/parser.panack:find_crlf	Bytes	Option[Nat]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	437	35	15304	15321	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	441	43	15485	15504	call	http_header_limit	packages/http/src/parser.panack:http_header_limit		Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	442	20	15526	15577	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	442	25	15531	15576	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	452	43	15874	15893	call	http_header_limit	packages/http/src/parser.panack:http_header_limit		Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	453	20	15915	15966	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	453	25	15920	15965	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	455	17	16000	16031	call	parse_header	packages/http/src/parser.panack:parse_header	Bytes	Result[packages/http/src/types.panack:Header,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	456	40	16073	16084	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	458	27	16142	16166	call	lower_ascii	packages/http/src/parser.panack:lower_ascii	Str	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	460	26	16285	16348	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	460	31	16290	16347	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	469	16	16465	16471	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	469	55	16504	16516	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[Nat],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	469	79	16528	16551	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[Nat],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	469	85	16534	16550	call	InvalidRequest	packages/http/src/types.panack:InvalidRequest		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	470	30	16585	16632	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[Nat],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	470	36	16591	16631	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	471	23	16657	16667	call	Ok	core:Ok	Option[Nat]	Result[Option[Nat],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	471	26	16660	16666	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	472	22	16691	16707	call	Ok	core:Ok	Option[Nat]	Result[Option[Nat],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	472	25	16694	16706	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	473	10	16719	16764	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[Nat],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	473	16	16725	16763	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	479	21	16948	16967	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/parser.panack	482	35	17049	17055	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	483	46	17101	17107	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	484	11	17120	17162	call	find_crlf	packages/http/src/parser.panack:find_crlf	Bytes	Option[Nat]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	484	35	17144	17161	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	486	31	17233	17285	call	parse_chunk_size	packages/http/src/parser.panack:parse_chunk_size	Bytes	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	487	36	17323	17334	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	491	19	17444	17495	call	parse_trailers	packages/http/src/parser.panack:parse_trailers	Bytes	Result[Option[Nat],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	492	42	17539	17550	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	498	21	17764	17781	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	499	22	17825	17860	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	499	27	17830	17859	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	500	21	17881	17898	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	502	21	17974	18006	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	502	63	18016	18052	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	503	22	18082	18131	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	503	27	18087	18130	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	505	20	18170	18241	namespace	join	src/stdlib/bytes.panack:join	Bytes	Bytes	bound-node	Owned: Bytes.join	P08/P10/P11
packages/http/src/parser.panack	505	43	18193	18240	call	copy_bytes	packages/http/src/parser.panack:copy_bytes	Bytes	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	512	16	18340	18346	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	512	55	18379	18391	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/parser.panack:ChunkedBody],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	512	79	18403	18426	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/parser.panack:ChunkedBody],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	512	85	18409	18425	call	InvalidRequest	packages/http/src/types.panack:InvalidRequest		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	513	30	18460	18514	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/parser.panack:ChunkedBody],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	513	36	18466	18513	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	514	23	18539	18549	call	Ok	core:Ok	Option[packages/http/src/parser.panack:ChunkedBody]	Result[Option[packages/http/src/parser.panack:ChunkedBody],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	514	26	18542	18548	call	None	core:None		Option[packages/http/src/parser.panack:ChunkedBody]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	515	11	18562	18579	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	515	40	18591	18645	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/parser.panack:ChunkedBody],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	515	46	18597	18644	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	516	22	18669	18704	call	Ok	core:Ok	Option[packages/http/src/parser.panack:ChunkedBody]	Result[Option[packages/http/src/parser.panack:ChunkedBody],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	516	25	18672	18703	call	Some	core:Some	packages/http/src/parser.panack:ChunkedBody	Option[packages/http/src/parser.panack:ChunkedBody]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	516	30	18677	18702	call	ChunkedBody	packages/http/src/parser.panack:ChunkedBody	Bytes	packages/http/src/parser.panack:ChunkedBody	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	517	10	18716	18763	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/parser.panack:ChunkedBody],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	517	16	18722	18762	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	521	3	18872	18945	call	parse_request_for_methods	packages/http/src/parser.panack:parse_request_for_methods	Bytes	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	539	9	19355	19376	call	find_header_end	packages/http/src/parser.panack:find_header_end	Bytes	Option[Nat]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	540	18	19396	19413	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	540	38	19416	19435	call	http_header_limit	packages/http/src/parser.panack:http_header_limit		Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	541	7	19444	19483	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	541	13	19450	19482	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	542	21	19504	19559	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	542	27	19510	19558	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	543	12	19573	19583	call	Ok	core:Ok	Option[packages/http/src/types.panack:Request]	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	543	15	19576	19582	call	None	core:None		Option[packages/http/src/types.panack:Request]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	544	45	19631	19650	call	http_header_limit	packages/http/src/parser.panack:http_header_limit		Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	545	7	19659	19698	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	545	13	19665	19697	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	547	13	19724	19766	call	request_head	packages/http/src/parser.panack:request_head	Bytes	Result[packages/http/src/parser.panack:RequestHead,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	548	25	19793	19805	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	553	18	19959	19976	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	554	26	20031	20082	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	554	32	20037	20081	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	554	87	20092	20102	call	Ok	core:Ok	Option[packages/http/src/types.panack:Request]	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	554	90	20095	20101	call	None	core:None		Option[packages/http/src/types.panack:Request]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	555	25	20129	20146	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	556	17	20192	20246	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	556	23	20198	20245	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	557	26	20272	20310	call	method_supported	packages/http/src/parser.panack:method_supported	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	558	17	20329	20366	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	558	23	20335	20365	call	UnsupportedMethod	packages/http/src/types.panack:UnsupportedMethod	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	560	17	20406	20535	call	Ok	core:Ok	Option[packages/http/src/types.panack:Request]	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	560	20	20409	20534	call	Some	core:Some	packages/http/src/types.panack:Request	Option[packages/http/src/types.panack:Request]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	560	25	20414	20533	call	Request	packages/http/src/types.panack:Request	Str	packages/http/src/types.panack:Request	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	561	19	20480	20532	call	copy_bytes	packages/http/src/parser.panack:copy_bytes	Bytes	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	561	53	20514	20531	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	565	21	20627	20713	call	decode_chunked	packages/http/src/parser.panack:decode_chunked	Bytes	Result[Option[packages/http/src/parser.panack:ChunkedBody],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	567	33	20748	20760	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	569	29	20837	20847	call	Ok	core:Ok	Option[packages/http/src/types.panack:Request]	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	569	32	20840	20846	call	None	core:None		Option[packages/http/src/types.panack:Request]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	570	40	20888	20926	call	method_supported	packages/http/src/parser.panack:method_supported	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	571	21	20949	20986	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	571	27	20955	20985	call	UnsupportedMethod	packages/http/src/types.panack:UnsupportedMethod	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	573	21	21034	21105	call	Ok	core:Ok	Option[packages/http/src/types.panack:Request]	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	573	24	21037	21104	call	Some	core:Some	packages/http/src/types.panack:Request	Option[packages/http/src/types.panack:Request]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	573	29	21042	21103	call	Request	packages/http/src/types.panack:Request	Str	packages/http/src/types.panack:Request	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	577	23	21182	21199	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	578	15	21234	21285	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	578	21	21240	21284	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	579	24	21309	21347	call	method_supported	packages/http/src/parser.panack:method_supported	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	580	15	21364	21401	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	580	21	21370	21400	call	UnsupportedMethod	packages/http/src/types.panack:UnsupportedMethod	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	582	15	21437	21515	call	Ok	core:Ok	Option[packages/http/src/types.panack:Request]	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	582	18	21440	21514	call	Some	core:Some	packages/http/src/types.panack:Request	Option[packages/http/src/types.panack:Request]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	582	23	21445	21513	call	Request	packages/http/src/types.panack:Request	Str	packages/http/src/types.panack:Request	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	582	71	21493	21512	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/parser.panack	592	28	21666	21672	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	594	22	21715	21732	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	594	52	21745	21751	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	595	8	21761	21782	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	595	39	21792	21817	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	596	9	21835	21860	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	596	44	21870	21895	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	597	15	21918	21930	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	605	5	22121	22145	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	606	6	22160	22184	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	606	40	22194	22218	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	608	5	22274	22294	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	608	35	22304	22328	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	609	5	22342	22366	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	609	39	22376	22400	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	610	5	22414	22438	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	610	39	22448	22472	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	611	5	22486	22510	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	613	5	22564	22621	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	613	11	22570	22620	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	615	18	22650	22674	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	616	19	22693	22718	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	617	18	22736	22761	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	620	28	22900	22925	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	623	19	23021	23042	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	629	7	23257	23314	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	629	13	23263	23313	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	635	41	23641	23698	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	635	47	23647	23697	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	636	31	23731	23788	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	636	37	23737	23787	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	637	14	23804	23814	call	Ok	core:Ok	Nat	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	643	21	23950	23994	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	643	27	23956	23993	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	650	19	24142	24163	call	byte_at	packages/http/src/parser.panack:byte_at	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	661	17	24445	24491	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	661	23	24451	24490	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	662	25	24518	24555	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	662	31	24524	24554	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	663	12	24569	24578	call	Ok	core:Ok	Nat	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	669	9	24713	24752	call	find_crlf	packages/http/src/parser.panack:find_crlf	Bytes	Option[Nat]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	670	15	24769	24826	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:ResponseHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	670	21	24775	24825	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	671	31	24858	24905	call	parse_response_status	packages/http/src/parser.panack:parse_response_status	Bytes	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	672	23	24930	24942	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:ResponseHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	675	43	25043	25049	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	678	41	25176	25182	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	680	48	25266	25272	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	681	17	25291	25330	call	find_crlf	packages/http/src/parser.panack:find_crlf	Bytes	Option[Nat]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	682	34	25366	25419	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	682	39	25371	25418	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	683	32	25454	25485	call	parse_header	packages/http/src/parser.panack:parse_header	Bytes	Result[packages/http/src/types.panack:Header,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	684	42	25529	25540	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	686	29	25602	25626	call	lower_ascii	packages/http/src/parser.panack:lower_ascii	Str	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	688	40	25712	25718	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	689	30	25750	25797	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	689	35	25755	25796	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	691	36	25860	25890	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser.panack	692	27	25917	25981	call	parse_response_decimal	packages/http/src/parser.panack:parse_response_decimal	Bytes	Result[Nat,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	692	60	25950	25968	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	693	50	26033	26044	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	694	56	26103	26115	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	698	55	26270	26305	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	698	60	26275	26304	call	UnsupportedTransferEncoding	packages/http/src/types.panack:UnsupportedTransferEncoding		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	701	47	26428	26453	call	lower_ascii	packages/http/src/parser.panack:lower_ascii	Str	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	704	27	26531	26553	receiver	append	core:append	Array[packages/http/src/types.panack:Header]	Array[packages/http/src/types.panack:Header]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/http/src/parser.panack	710	22	26660	26666	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	710	61	26699	26711	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:ResponseHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	710	85	26723	26747	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:ResponseHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	710	91	26729	26746	call	InvalidResponse	packages/http/src/types.panack:InvalidResponse		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	711	69	26820	26856	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:ResponseHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	711	75	26826	26855	call	UnsupportedTransferEncoding	packages/http/src/types.panack:UnsupportedTransferEncoding		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	712	60	26918	26924	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	713	11	26937	27008	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/parser.panack:ResponseHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	713	17	26943	27007	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	715	11	27036	27124	call	Ok	core:Ok	packages/http/src/parser.panack:ResponseHead	Result[packages/http/src/parser.panack:ResponseHead,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	715	14	27039	27123	call	ResponseHead	packages/http/src/parser.panack:ResponseHead	Nat	packages/http/src/parser.panack:ResponseHead	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	723	42	27300	27319	call	http_header_limit	packages/http/src/parser.panack:http_header_limit		Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	725	5	27375	27397	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	725	11	27381	27396	call	InvalidLimits	packages/http/src/types.panack:InvalidLimits		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	730	44	27538	27544	call	None	core:None		Option[packages/http/src/parser.panack:ResponseHead]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	731	37	27581	27587	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	733	38	27654	27660	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	734	13	27675	27709	call	find_header_end_from	packages/http/src/parser.panack:find_header_end_from	Bytes	Option[Nat]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	735	30	27741	27910	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	735	38	27749	27766	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	736	11	27794	27840	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	737	18	27858	27907	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	746	53	28286	28338	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	746	58	28291	28337	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	748	19	28376	28427	call	response_head	packages/http/src/parser.panack:response_head	Bytes	Result[packages/http/src/parser.panack:ResponseHead,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	749	42	28471	28482	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	752	50	28636	28682	call	Some	core:Some	packages/http/src/types.panack:HttpError	Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	752	55	28641	28681	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	755	30	28787	28797	call	Some	core:Some	packages/http/src/parser.panack:ResponseHead	Option[packages/http/src/parser.panack:ResponseHead]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	763	18	28919	28925	call	None	core:None		Option[packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	763	57	28958	28970	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	763	81	28982	29006	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	763	87	28988	29005	call	InvalidResponse	packages/http/src/types.panack:InvalidResponse		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	765	17	29057	29108	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	765	23	29063	29107	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	769	29	29238	29255	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	774	31	29450	29509	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	774	37	29456	29508	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	775	18	29529	29589	call	Ok	core:Ok	packages/http/src/types.panack:Response	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	775	21	29532	29588	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	775	57	29568	29587	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/parser.panack	779	15	29713	29765	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	779	21	29719	29764	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	781	15	29823	29878	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	781	21	29829	29877	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	783	15	29914	30007	call	Ok	core:Ok	packages/http/src/types.panack:Response	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	783	18	29917	30006	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	783	54	29953	30005	call	copy_bytes	packages/http/src/parser.panack:copy_bytes	Bytes	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	783	88	29987	30004	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	786	21	30083	30152	call	decode_chunked	packages/http/src/parser.panack:decode_chunked	Bytes	Result[Option[packages/http/src/parser.panack:ChunkedBody],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	787	33	30187	30199	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	789	29	30276	30336	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	789	35	30282	30335	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	790	33	30370	30420	call	Ok	core:Ok	packages/http/src/types.panack:Response	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	790	36	30373	30419	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	794	15	30516	30553	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	794	21	30522	30552	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	796	15	30589	30682	call	Ok	core:Ok	packages/http/src/types.panack:Response	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/parser.panack	796	18	30592	30681	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	796	54	30628	30680	call	copy_bytes	packages/http/src/parser.panack:copy_bytes	Bytes	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/parser.panack	796	88	30662	30679	call	byte_length	packages/http/src/parser.panack:byte_length	Bytes	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	11	3	557	559	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
packages/http/src/parser_test.panack	15	9	617	647	call	parse_request	packages/http/src/parser.panack:parse_request	Bytes	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	23	9	881	911	call	parse_request	packages/http/src/parser.panack:parse_request	Bytes	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	28	9	1100	1121	receiver	len	core:len	Array[packages/http/src/types.panack:Header]	Nat	bound-node	owned:Str.len / Array[T].len / Bytes.length	P06/P08; callers P10/P11; remove P12
packages/http/src/parser_test.panack	29	25	1191	1222	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	35	9	1307	1340	call	parse_request	packages/http/src/parser.panack:parse_request	Bytes	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	42	24	1478	1613	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	43	6	1497	1521	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	43	32	1523	1552	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	43	63	1554	1583	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	44	5	1590	1612	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	45	9	1622	1650	call	serialize_response	packages/http/src/server.panack:serialize_response	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	47	17	1696	1718	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	53	9	1901	1937	call	parse_response	packages/http/src/parser.panack:parse_response	Bytes	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	55	67	2033	2055	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	60	9	2137	2173	call	parse_response	packages/http/src/parser.panack:parse_response	Bytes	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	67	21	2283	2355	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	68	3	2358	2422	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	68	10	2365	2421	call	parsed_request_matches	packages/http/src/parser_test.panack:parsed_request_matches	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	70	24	2447	2542	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	72	25	2567	2620	namespace	join	src/stdlib/bytes.panack:join	Bytes	Bytes	bound-node	Owned: Bytes.join	P08/P10/P11
packages/http/src/parser_test.panack	72	55	2597	2619	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	73	3	2623	2664	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	73	10	2630	2663	call	parse_is_incomplete	packages/http/src/parser_test.panack:parse_is_incomplete	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	74	22	2686	2740	namespace	join	src/stdlib/bytes.panack:join	Bytes	Bytes	bound-node	Owned: Bytes.join	P08/P10/P11
packages/http/src/parser_test.panack	74	53	2717	2739	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	75	3	2743	2809	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	75	10	2750	2808	call	parsed_request_matches	packages/http/src/parser_test.panack:parsed_request_matches	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	77	20	2830	3008	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	80	3	3011	3082	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	80	10	3018	3081	call	parsed_request_matches	packages/http/src/parser_test.panack:parsed_request_matches	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	81	32	3114	3252	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	84	3	3255	3336	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	84	10	3262	3335	call	parse_has_error	packages/http/src/parser_test.panack:parse_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	84	47	3299	3334	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	86	27	3364	3428	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	87	3	3431	3496	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	87	10	3438	3495	call	parse_has_error	packages/http/src/parser_test.panack:parse_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	87	42	3470	3494	call	UnsupportedMethod	packages/http/src/types.panack:UnsupportedMethod	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	88	24	3520	3584	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	89	3	3587	3655	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	89	10	3594	3654	call	parse_has_error	packages/http/src/parser_test.panack:parse_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	89	39	3623	3653	call	UnsupportedVersion	packages/http/src/types.panack:UnsupportedVersion	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	90	30	3685	3749	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	91	3	3752	3838	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	91	10	3759	3837	call	parse_has_error	packages/http/src/parser_test.panack:parse_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	91	45	3794	3836	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	92	23	3861	3955	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	94	3	3958	4034	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	94	10	3965	4033	call	parse_has_error	packages/http/src/parser_test.panack:parse_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	94	38	3993	4032	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	95	22	4056	4195	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	98	3	4198	4298	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	98	10	4205	4297	call	parse_has_error	packages/http/src/parser_test.panack:parse_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	98	37	4232	4296	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	99	29	4327	4448	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	102	3	4451	4535	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	102	10	4458	4534	call	parse_has_error	packages/http/src/parser_test.panack:parse_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	102	44	4492	4533	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	103	31	4566	4663	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	105	3	4666	4740	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	105	10	4673	4739	call	parse_has_error	packages/http/src/parser_test.panack:parse_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	105	46	4709	4738	call	UnsupportedTransferEncoding	packages/http/src/types.panack:UnsupportedTransferEncoding		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	106	18	4758	4807	namespace	join	src/stdlib/bytes.panack:join	Bytes	Bytes	bound-node	Owned: Bytes.join	P08/P10/P11
packages/http/src/parser_test.panack	106	45	4785	4806	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	107	3	4810	4886	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	107	10	4817	4885	call	parse_has_error	packages/http/src/parser_test.panack:parse_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	107	33	4840	4884	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	108	25	4911	4963	namespace	join	src/stdlib/bytes.panack:join	Bytes	Bytes	bound-node	Owned: Bytes.join	P08/P10/P11
packages/http/src/parser_test.panack	108	55	4941	4962	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	109	3	4966	5121	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	109	16	4979	5016	call	parse_request	packages/http/src/parser.panack:parse_request	Bytes	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	110	30	5048	5092	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	114	30	5152	5171	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/parser_test.panack	115	43	5214	5249	namespace	push	src/stdlib/bytes.panack:push	Bytes	Bytes	bound-node	Owned: Bytes.push	P08/P10/P11
packages/http/src/parser_test.panack	116	3	5254	5402	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	116	16	5267	5309	call	parse_request	packages/http/src/parser.panack:parse_request	Bytes	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	117	30	5341	5373	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	121	3	5406	5443	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	121	10	5413	5442	call	serialized_response_matches	packages/http/src/parser_test.panack:serialized_response_matches		Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	123	3	5447	5566	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	123	10	5454	5565	call	parsed_request_matches	packages/http/src/parser_test.panack:parsed_request_matches	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	123	33	5477	5547	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	125	3	5569	5713	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	125	10	5576	5712	call	parsed_request_matches	packages/http/src/parser_test.panack:parsed_request_matches	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	125	33	5599	5692	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	127	3	5716	5891	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	127	10	5723	5890	call	parsed_request_matches	packages/http/src/parser_test.panack:parsed_request_matches	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	127	33	5746	5866	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	130	3	5894	6022	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	130	10	5901	6021	call	parse_has_error	packages/http/src/parser_test.panack:parse_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	130	26	5917	5990	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	131	58	5992	6020	call	UnsupportedMethod	packages/http/src/types.panack:UnsupportedMethod	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	132	3	6025	6355	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	132	16	6038	6177	call	serialize_head_response	packages/http/src/server.panack:serialize_head_response	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	132	40	6062	6176	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	133	8	6083	6119	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	133	46	6121	6150	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	133	78	6153	6175	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	135	17	6223	6245	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	138	3	6358	6599	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	138	16	6371	6442	call	serialize_head_response	packages/http/src/server.panack:serialize_head_response	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	138	40	6395	6441	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	138	58	6413	6440	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	140	17	6488	6510	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	143	3	6602	6809	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	143	16	6615	6678	call	serialize_head_response	packages/http/src/server.panack:serialize_head_response	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	143	40	6639	6677	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	143	58	6657	6676	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/parser_test.panack	145	17	6724	6746	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	147	3	6812	6997	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	147	16	6825	6920	call	serialize_head_response	packages/http/src/server.panack:serialize_head_response	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	147	40	6849	6919	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	147	55	6864	6893	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	147	87	6896	6918	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	148	30	6952	6969	call	InvalidResponse	packages/http/src/types.panack:InvalidResponse		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	153	3	7002	7140	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	153	10	7009	7139	call	response_matches	packages/http/src/parser_test.panack:response_matches	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	153	27	7026	7127	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	155	3	7143	7251	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	155	10	7150	7250	call	response_matches	packages/http/src/parser_test.panack:response_matches	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	155	27	7167	7238	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	157	3	7254	7398	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	157	10	7261	7397	call	response_matches	packages/http/src/parser_test.panack:response_matches	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	157	27	7278	7384	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	159	3	7401	7595	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	159	10	7408	7594	call	response_matches	packages/http/src/parser_test.panack:response_matches	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	159	27	7425	7575	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	162	3	7598	7696	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	162	10	7605	7695	call	response_matches	packages/http/src/parser_test.panack:response_matches	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	162	27	7622	7675	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	163	3	7699	7785	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	163	10	7706	7784	call	response_matches	packages/http/src/parser_test.panack:response_matches	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	163	27	7723	7774	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	164	3	7788	7932	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	164	10	7795	7931	call	response_has_error	packages/http/src/parser_test.panack:response_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	164	29	7814	7879	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	165	5	7885	7930	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	166	3	7935	8101	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	166	10	7942	8100	call	response_has_error	packages/http/src/parser_test.panack:response_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	166	29	7961	8052	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	168	5	8058	8099	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	169	3	8104	8310	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	169	10	8111	8309	call	response_has_error	packages/http/src/parser_test.panack:response_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	169	29	8130	8238	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	171	5	8244	8308	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	172	3	8313	8440	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	172	10	8320	8439	call	response_has_error	packages/http/src/parser_test.panack:response_has_error	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	172	29	8339	8382	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	173	5	8388	8438	call	ProtocolError	packages/http/src/types.panack:ProtocolError	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	174	3	8443	8607	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	174	16	8456	8528	call	parse_response	packages/http/src/parser.panack:parse_response	Bytes	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	174	31	8471	8518	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	175	30	8560	8575	call	InvalidLimits	packages/http/src/types.panack:InvalidLimits		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	178	3	8610	8803	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	178	16	8623	8726	call	serialize_response	packages/http/src/server.panack:serialize_response	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	178	35	8642	8725	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	178	50	8657	8695	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	179	7	8704	8724	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	180	30	8758	8775	call	InvalidResponse	packages/http/src/types.panack:InvalidResponse		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	183	3	8806	8992	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	183	16	8819	8915	call	serialize_response	packages/http/src/server.panack:serialize_response	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	183	35	8838	8914	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	183	50	8853	8882	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	184	7	8891	8913	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	185	30	8947	8964	call	InvalidResponse	packages/http/src/types.panack:InvalidResponse		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	188	3	8995	9187	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	188	16	9008	9110	call	serialize_response	packages/http/src/server.panack:serialize_response	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	188	35	9027	9109	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	188	50	9042	9080	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	189	7	9089	9108	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/parser_test.panack	190	30	9142	9159	call	InvalidResponse	packages/http/src/types.panack:InvalidResponse		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	193	3	9190	9398	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	193	16	9203	9261	call	serialize_response	packages/http/src/server.panack:serialize_response	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	193	35	9222	9260	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	193	53	9240	9259	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/parser_test.panack	195	17	9307	9329	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	198	3	9401	9554	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	198	16	9414	9477	call	serialize_response	packages/http/src/server.panack:serialize_response	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	198	35	9433	9476	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	198	53	9451	9475	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	199	30	9509	9526	call	InvalidResponse	packages/http/src/types.panack:InvalidResponse		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	202	3	9557	9829	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	202	16	9570	9686	call	parse_request_for_methods	packages/http/src/parser.panack:parse_request_for_methods	Bytes	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	203	5	9601	9662	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	211	3	9832	10053	call	expect	packages/http/src/parser_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	211	16	9845	9966	call	parse_request_for_methods	packages/http/src/parser.panack:parse_request_for_methods	Bytes	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	212	5	9876	9934	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/parser_test.panack	214	30	9998	10023	call	UnsupportedMethod	packages/http/src/types.panack:UnsupportedMethod	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/parser_test.panack	217	3	10056	10058	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
packages/http/src/server.panack	72	20	2529	2552	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/server.panack	73	15	2567	2594	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	75	18	2638	2665	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	76	17	2684	2715	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/http/src/server.panack	89	20	3192	3215	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/server.panack	92	18	3274	3301	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	93	17	3320	3351	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/http/src/server.panack	101	20	3504	3527	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/server.panack	102	23	3550	3569	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/server.panack	104	18	3609	3636	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	105	17	3655	3686	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/http/src/server.panack	106	14	3700	3781	namespace	push	src/stdlib/bytes.panack:push	Bytes	Bytes	bound-node	Owned: Bytes.push	P08/P10/P11
packages/http/src/server.panack	109	3	3812	3836	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/http/src/server.panack	116	22	4044	4077	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	119	17	4172	4190	call	lower	packages/http/src/server.panack:lower	Str	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	120	9	4199	4229	call	header_name_valid	packages/http/src/server.panack:header_name_valid	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	120	44	4234	4266	call	header_value_valid	packages/http/src/server.panack:header_value_valid	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	131	29	4651	4670	call	lower	packages/http/src/server.panack:lower	Str	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	135	60	4798	4831	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	143	3	5032	5071	call	serialize_response_mode	packages/http/src/server.panack:serialize_response_mode	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	147	3	5152	5192	call	serialize_response_mode	packages/http/src/server.panack:serialize_response_mode	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	151	7	5295	5327	call	response_headers_valid	packages/http/src/server.panack:response_headers_valid	packages/http/src/types.panack:Response	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	152	5	5334	5358	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	152	11	5340	5357	call	InvalidResponse	packages/http/src/types.panack:InvalidResponse		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	153	13	5371	5404	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	154	5	5421	5453	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	154	11	5427	5452	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	157	24	5522	5543	call	reason_phrase	packages/http/src/server.panack:reason_phrase	Nat	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	158	25	5568	5632	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/server.panack	162	19	5714	5732	call	lower	packages/http/src/server.panack:lower	Str	Str	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	166	24	5910	5965	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/server.panack	167	12	5977	6003	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	167	41	6006	6031	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	170	20	6107	6138	namespace	join	src/stdlib/bytes.panack:join	Bytes	Bytes	bound-node	Owned: Bytes.join	P08/P10/P11
packages/http/src/server.panack	175	26	6246	6279	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	177	24	6350	6409	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/server.panack	178	10	6419	6445	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	178	39	6448	6475	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	181	18	6545	6578	namespace	join	src/stdlib/bytes.panack:join	Bytes	Bytes	bound-node	Owned: Bytes.join	P08/P10/P11
packages/http/src/server.panack	184	22	6614	6659	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/server.panack	185	25	6684	6710	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	185	54	6713	6740	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	186	7	6749	6782	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	188	7	6834	6866	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	188	13	6840	6865	call	LimitExceeded	packages/http/src/types.panack:LimitExceeded	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	190	16	6895	6928	namespace	join	src/stdlib/bytes.panack:join	Bytes	Bytes	bound-node	Owned: Bytes.join	P08/P10/P11
packages/http/src/server.panack	191	7	6935	7011	call	Ok	core:Ok	Bytes	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	191	41	6969	7008	namespace	join	src/stdlib/bytes.panack:join	Bytes	Bytes	bound-node	Owned: Bytes.join	P08/P10/P11
packages/http/src/server.panack	218	3	7752	7812	call	simple_response_with_headers	packages/http/src/server.panack:simple_response_with_headers	Nat	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	224	17	7942	7967	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/server.panack	225	24	7991	8086	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	226	5	8012	8079	receiver	append	core:append	Array[packages/http/src/types.panack:Header]	Array[packages/http/src/types.panack:Header]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/http/src/server.panack	226	20	8027	8078	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	227	3	8089	8261	namespace	value_or	src/stdlib/result.panack:value_or	Result[Bytes,packages/http/src/types.panack:HttpError]	Bytes	bound-node	Owned: Result[T,E].value_or	P08/P10/P11
packages/http/src/server.panack	227	19	8105	8149	call	serialize_response_mode	packages/http/src/server.panack:serialize_response_mode	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	227	65	8151	8260	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/server.panack	232	17	8375	8394	call	error_status	packages/http/src/server.panack:error_status	packages/http/src/types.panack:HttpError	Nat	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	234	19	8434	8472	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/http/src/server.panack	239	26	8635	8787	call	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	240	7	8658	8780	receiver	append	core:append	Array[packages/http/src/types.panack:Header]	Array[packages/http/src/types.panack:Header]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/http/src/server.panack	240	7	8658	8711	receiver	append	core:append	Array[packages/http/src/types.panack:Header]	Array[packages/http/src/types.panack:Header]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/http/src/server.panack	240	35	8686	8710	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	241	9	8728	8779	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	242	5	8792	8911	namespace	value_or	src/stdlib/result.panack:value_or	Result[Bytes,packages/http/src/types.panack:HttpError]	Bytes	bound-node	Owned: Result[T,E].value_or	P08/P10/P11
packages/http/src/server.panack	242	21	8808	8852	call	serialize_response_mode	packages/http/src/server.panack:serialize_response_mode	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	242	67	8854	8910	call	simple_response	packages/http/src/server.panack:simple_response	Nat	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	244	5	8944	9032	call	simple_response_with_headers	packages/http/src/server.panack:simple_response_with_headers	Nat	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	246	5	9065	9164	call	simple_response_with_headers	packages/http/src/server.panack:simple_response_with_headers	Nat	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	248	5	9197	9287	call	simple_response_with_headers	packages/http/src/server.panack:simple_response_with_headers	Nat	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	250	5	9320	9424	call	simple_response_with_headers	packages/http/src/server.panack:simple_response_with_headers	Nat	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	252	5	9440	9584	call	simple_response_with_headers	packages/http/src/server.panack:simple_response_with_headers	Nat	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	260	3	9777	9805	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	260	40	9814	9841	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/http/src/server.panack	261	5	9855	9882	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/http/src/server.panack	261	42	9892	9919	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/http/src/server.panack	262	5	9933	9960	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/http/src/server.panack	262	42	9970	9997	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/http/src/server.panack	265	52	10058	10064	call	Ok	core:Ok	Unit	Result[Unit,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	265	55	10061	10063	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
packages/http/src/server.panack	267	3	10128	10160	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Unit,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	267	9	10134	10159	call	TransportFailure	packages/http/src/types.panack:TransportFailure	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	274	9	10308	10329	receiver	call	core:$method_call	AsyncFn[packages/http/src/types.panack:Request,Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]]	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	owned:callable family.call	P06/P08; callers P10/P11; remove P12
packages/http/src/server.panack	279	5	10483	10594	call	ConnectionContext	packages/http/src/server.panack:ConnectionContext	AsyncFn[packages/http/src/types.panack:Request,Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]]	packages/http/src/server.panack:ConnectionContext[AsyncFn[packages/http/src/types.panack:Request,Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]]]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	279	40	10518	10540	ref	invoke_legacy_handler	ref:packages/http/src/server.panack:invoke_legacy_handler		AsyncFn[AsyncFn[packages/http/src/types.panack:Request,Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]],packages/http/src/types.panack:Request,Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	281	9	10603	10689	call	serve_connection_with_context	packages/http/src/server.panack:serve_connection_with_context	packages/http/src/server.panack:ConnectionContext[AsyncFn[packages/http/src/types.panack:Request,Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]]]	Result[Bytes,Str]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	287	9	10800	10887	call	serve_connection_with_policy	packages/http/src/server.panack:serve_connection_with_policy	packages/http/src/server.panack:ConnectionContext[packages/http/src/server.panack:serve_connection_with_context parameter 0]	Result[Bytes,Str]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	287	50	10841	10886	call	ConnectionPolicy	packages/http/src/server.panack:ConnectionPolicy	Array[Str]	packages/http/src/server.panack:ConnectionPolicy	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	294	25	11041	11060	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/server.panack	295	36	11096	11124	call	Error	core:Error	Str	Result[Bytes,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	299	17	11208	11236	call	tcp_server_read_chunk	core:tcp_server_read_chunk	Nat	Result[Bytes,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	302	21	11338	11442	call	Ok	core:Ok	Bytes	Result[Bytes,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	302	24	11341	11441	call	simple_response_with_headers	packages/http/src/server.panack:simple_response_with_headers	Nat	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	305	21	11480	11494	call	Error	core:Error	Str	Result[Bytes,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	310	20	11578	11611	namespace	join	src/stdlib/bytes.panack:join	Bytes	Bytes	bound-node	Owned: Bytes.join	P08/P10/P11
packages/http/src/server.panack	311	21	11632	11657	call	is_head_request	packages/http/src/server.panack:is_head_request	Bytes	Bool	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	312	21	11678	11703	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	313	15	11723	11802	call	parse_request_for_methods	packages/http/src/parser.panack:parse_request_for_methods	Bytes	Result[Option[packages/http/src/types.panack:Request],packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	315	23	11855	11903	call	Ok	core:Ok	Bytes	Result[Bytes,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	315	26	11858	11902	call	response_for_error	packages/http/src/server.panack:response_for_error	packages/http/src/types.panack:HttpError	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	321	27	12085	12137	receiver	call	core:$method_call	AsyncFn[packages/http/src/server.panack:serve_connection_with_policy parameter 0,packages/http/src/types.panack:Request,Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]]	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	owned:callable family.call	P06/P08; callers P10/P11; remove P12
packages/http/src/server.panack	323	29	12202	12318	call	Ok	core:Ok	Bytes	Result[Bytes,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	323	32	12205	12317	call	simple_response_with_headers	packages/http/src/server.panack:simple_response_with_headers	Nat	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	327	39	12410	12454	call	serialize_response_mode	packages/http/src/server.panack:serialize_response_mode	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	329	31	12523	12641	call	Ok	core:Ok	Bytes	Result[Bytes,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	329	34	12526	12640	call	simple_response_with_headers	packages/http/src/server.panack:simple_response_with_headers	Nat	Bytes	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	333	34	12732	12756	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/http/src/server.panack	334	31	12814	12850	call	Error	core:Error	Str	Result[Bytes,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	337	31	12944	12952	call	Ok	core:Ok	Bytes	Result[Bytes,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	357	33	13315	13515	call	TcpServerLimits	src/stdlib/tcp.panack:TcpServerLimits	Nat	src/stdlib/tcp.panack:TcpServerLimits	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	366	27	13542	13608	call	ServeContext	packages/http/src/server.panack:ServeContext	AsyncFn[packages/http/src/types.panack:Request,Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]]	packages/http/src/server.panack:ServeContext	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	367	15	13623	13694	call	tcp_serve_stream	core:tcp_serve_stream	Str	Result[Array[Result[Unit,Str]],Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	367	47	13655	13672	ref	serve_connection	ref:packages/http/src/server.panack:serve_connection		AsyncFn[packages/http/src/server.panack:ServeContext,Result[Bytes,Str]]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	369	7	13772	13794	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Array[Result[Unit,packages/http/src/types.panack:HttpError]],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	369	13	13778	13793	call	InvalidLimits	packages/http/src/types.panack:InvalidLimits		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	370	14	13808	13840	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[Array[Result[Unit,packages/http/src/types.panack:HttpError]],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/server.panack	370	20	13814	13839	call	TransportFailure	packages/http/src/types.panack:TransportFailure	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	375	35	14001	14036	receiver	append	core:append	Array[Result[Unit,packages/http/src/types.panack:HttpError]]	Array[Result[Unit,packages/http/src/types.panack:HttpError]]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/http/src/server.panack	375	50	14016	14035	call	successful_report	packages/http/src/server.panack:successful_report		Result[Unit,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	376	41	14080	14118	receiver	append	core:append	Array[Result[Unit,packages/http/src/types.panack:HttpError]]	Array[Result[Unit,packages/http/src/types.panack:HttpError]]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/http/src/server.panack	376	56	14095	14117	call	failed_report	packages/http/src/server.panack:failed_report	Str	Result[Unit,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/http/src/server.panack	379	7	14145	14156	call	Ok	core:Ok	Array[Result[Unit,packages/http/src/types.panack:HttpError]]	Result[Array[Result[Unit,packages/http/src/types.panack:HttpError]],packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/service.panack	26	19	899	1037	namespace	TcpServiceLimits	src/stdlib/tcp_service.panack:TcpServiceLimits	Nat	src/stdlib/tcp_service.panack:TcpServiceLimits	bound-node	retain-explicit-declaration	P11
packages/http/src/service.panack	28	15	1052	1158	namespace	serve	src/stdlib/tcp_service.panack:serve	Str	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,Str]	bound-node	Retain explicit module function	P08/P10/P11
packages/http/src/service.panack	31	20	1180	1191	call	Ok	core:Ok	src/stdlib/tcp_service.panack:TcpServiceSummary	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/service.panack	33	7	1268	1290	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/service.panack	33	13	1274	1289	call	InvalidLimits	packages/http/src/types.panack:InvalidLimits		packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/service.panack	34	14	1304	1336	call	Error	core:Error	packages/http/src/types.panack:HttpError	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/http/src/service.panack	34	20	1310	1335	call	TransportFailure	packages/http/src/types.panack:TransportFailure	Str	packages/http/src/types.panack:HttpError	bound-node	retain-explicit-declaration	P11
packages/http/src/transport.panack	5	9	282	359	call	http_transport_exchange	core:http_transport_exchange	Str	Result[Bytes,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/mime.panack	2	6	49	76	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	2	37	80	106	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	3	11	148	174	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	4	11	215	240	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	4	40	244	270	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	5	11	318	345	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	5	42	349	375	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	6	11	409	435	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	7	11	478	504	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	8	11	537	563	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	9	11	594	620	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	10	11	647	673	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	10	41	677	704	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	11	11	732	758	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	12	11	785	812	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	13	11	840	867	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	14	11	895	921	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	15	11	951	979	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	16	11	1007	1034	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	17	11	1061	1087	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	18	11	1113	1139	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	19	11	1165	1191	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/mime.panack	20	11	1224	1251	receiver	ends_with	core:$core_ends_with	Str	Bool	bound-node	owned:Str.ends_with	P06/P08; callers P10/P11; remove P12
packages/static_http/src/paths.panack	12	35	226	242	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	13	40	284	300	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	14	41	343	359	call	Some	core:Some	Nat	Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	15	10	371	377	call	None	core:None		Option[Nat]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	19	16	453	467	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/paths.panack	21	14	509	553	namespace	push	src/stdlib/bytes.panack:push	Bytes	Bytes	bound-node	Owned: Bytes.push	P08/P10/P11
packages/static_http/src/paths.panack	21	34	529	552	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/static_http/src/paths.panack	28	16	718	732	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/paths.panack	32	13	818	842	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/static_http/src/paths.panack	37	15	944	977	call	hex	packages/static_http/src/paths.panack:hex	Nat	Option[Nat]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths.panack	37	19	948	976	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/static_http/src/paths.panack	39	31	1049	1082	call	hex	packages/static_http/src/paths.panack:hex	Nat	Option[Nat]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths.panack	39	35	1053	1081	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/static_http/src/paths.panack	46	26	1355	1383	namespace	push	src/stdlib/bytes.panack:push	Bytes	Bytes	bound-node	Owned: Bytes.push	P08/P10/P11
packages/static_http/src/paths.panack	57	18	1592	1618	namespace	push	src/stdlib/bytes.panack:push	Bytes	Bytes	bound-node	Owned: Bytes.push	P08/P10/P11
packages/static_http/src/paths.panack	62	15	1677	1704	call	Error	core:Error	Str	Result[Str,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	63	11	1717	1738	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/static_http/src/paths.panack	63	40	1746	1752	call	Ok	core:Ok	Str	Result[Str,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	65	11	1774	1798	namespace	from_native	src/stdlib/path.panack:from_native	Bytes	Result[Path,src/stdlib/path.panack:PathError]	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/paths.panack	66	25	1825	1852	call	Error	core:Error	Str	Result[Str,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	67	27	1880	1900	namespace	to_text	src/stdlib/path.panack:to_text	Path	Result[Str,src/stdlib/path.panack:PathError]	bound-node	Owned: Path.to_text	P08/P10/P11
packages/static_http/src/paths.panack	68	27	1929	1955	call	Error	core:Error	Str	Result[Str,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	69	22	1978	1987	call	Ok	core:Ok	Str	Result[Str,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	76	11	2062	2088	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/static_http/src/paths.panack	77	17	2105	2119	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/paths.panack	79	20	2158	2178	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/static_http/src/paths.panack	80	13	2193	2217	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/static_http/src/paths.panack	82	14	2252	2277	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/static_http/src/paths.panack	84	17	2363	2377	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/paths.panack	86	17	2407	2434	namespace	push	src/stdlib/bytes.panack:push	Bytes	Bytes	bound-node	Owned: Bytes.push	P08/P10/P11
packages/static_http/src/paths.panack	89	10	2454	2479	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/static_http/src/paths.panack	94	11	2583	2607	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/static_http/src/paths.panack	95	12	2619	2639	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/static_http/src/paths.panack	96	37	2676	2703	call	Error	core:Error	Str	Result[packages/static_http/src/paths.panack:Target,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	97	11	2716	2735	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/static_http/src/paths.panack	97	39	2744	2780	call	Error	core:Error	Str	Result[packages/static_http/src/paths.panack:Target,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	103	15	2900	2924	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/static_http/src/paths.panack	108	17	3096	3129	call	Error	core:Error	Str	Result[packages/static_http/src/paths.panack:Target,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	110	13	3155	3173	call	decode	packages/static_http/src/paths.panack:decode	Bytes	Result[Str,Str]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths.panack	111	27	3202	3216	call	Error	core:Error	Str	Result[packages/static_http/src/paths.panack:Target,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	112	29	3246	3270	call	segments_valid	packages/static_http/src/paths.panack:segments_valid	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths.panack	113	11	3283	3313	call	Error	core:Error	Str	Result[packages/static_http/src/paths.panack:Target,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	116	18	3380	3408	receiver	nat	core:nat	Rat	Nat	bound-node	owned:Rat.nat	P06/P08; callers P10/P11; remove P12
packages/static_http/src/paths.panack	117	19	3427	3449	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/static_http/src/paths.panack	119	13	3516	3553	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/static_http/src/paths.panack	119	30	3533	3552	call	copy	packages/static_http/src/paths.panack:copy	Bytes	Bytes	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths.panack	120	15	3576	3618	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/static_http/src/paths.panack	120	32	3593	3617	call	copy	packages/static_http/src/paths.panack:copy	Bytes	Bytes	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths.panack	122	11	3641	3678	call	Ok	core:Ok	packages/static_http/src/paths.panack:Target	Result[packages/static_http/src/paths.panack:Target,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/paths.panack	122	14	3644	3677	call	Target	packages/static_http/src/paths.panack:Target	Str	packages/static_http/src/paths.panack:Target	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	6	3	137	139	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
packages/static_http/src/paths_test.panack	10	9	226	245	namespace	parse	packages/static_http/src/paths.panack:parse	Str	Result[packages/static_http/src/paths.panack:Target,Str]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	17	9	428	447	namespace	parse	packages/static_http/src/paths.panack:parse	Str	Result[packages/static_http/src/paths.panack:Target,Str]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	24	3	526	559	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	24	10	533	558	call	valid	packages/static_http/src/paths_test.panack:valid	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	25	3	562	617	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	25	10	569	616	call	valid	packages/static_http/src/paths_test.panack:valid	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	26	3	620	676	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	26	10	627	675	call	valid	packages/static_http/src/paths_test.panack:valid	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	27	3	679	727	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	27	10	686	726	call	valid	packages/static_http/src/paths_test.panack:valid	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	28	3	730	790	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	28	10	737	789	call	valid	packages/static_http/src/paths_test.panack:valid	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	29	3	793	854	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	29	10	800	853	call	valid	packages/static_http/src/paths_test.panack:valid	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	34	5	1141	1164	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	34	12	1148	1163	call	invalid	packages/static_http/src/paths_test.panack:invalid	Str	Bool	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	36	3	1171	1240	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	36	10	1178	1209	namespace	content_type	packages/static_http/src/mime.panack:content_type	Str	Str	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	37	3	1243	1310	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	37	10	1250	1280	namespace	content_type	packages/static_http/src/mime.panack:content_type	Str	Str	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	38	3	1313	1388	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	38	10	1320	1351	namespace	content_type	packages/static_http/src/mime.panack:content_type	Str	Str	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	39	3	1391	1446	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	39	10	1398	1429	namespace	content_type	packages/static_http/src/mime.panack:content_type	Str	Str	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	40	3	1449	1506	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	40	10	1456	1489	namespace	content_type	packages/static_http/src/mime.panack:content_type	Str	Str	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	41	3	1509	1579	call	expect	packages/static_http/src/paths_test.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	41	10	1516	1548	namespace	content_type	packages/static_http/src/mime.panack:content_type	Str	Str	bound-node	retain-explicit-declaration	P11
packages/static_http/src/paths_test.panack	42	3	1582	1584	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
packages/static_http/src/service.panack	28	16	642	656	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/service.panack	29	11	667	695	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/static_http/src/service.panack	30	19	714	734	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/static_http/src/service.panack	31	13	749	772	namespace	at	src/stdlib/bytes.panack:at	Bytes	Nat	bound-node	Owned: Bytes.at	P08/P10/P11
packages/static_http/src/service.panack	32	14	786	866	namespace	push	src/stdlib/bytes.panack:push	Bytes	Bytes	bound-node	Owned: Bytes.push	P08/P10/P11
packages/static_http/src/service.panack	34	3	873	897	namespace	decode_utf8	src/stdlib/text.panack:decode_utf8	Bytes	Str	bound-node	Owned: Bytes.decode_utf8	P08/P10/P11
packages/static_http/src/service.panack	40	12	1021	1039	call	lower	packages/static_http/src/service.panack:lower	Str	Str	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	48	11	1273	1355	namespace	serialize_response	packages/http/src/server.panack:serialize_response	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	48	35	1297	1354	namespace	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	48	77	1339	1353	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/service.panack	50	19	1407	1426	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/static_http/src/service.panack	56	13	1584	1665	receiver	append	core:append	Array[packages/http/src/types.panack:Header]	Array[packages/http/src/types.panack:Header]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/static_http/src/service.panack	56	42	1613	1664	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	57	3	1668	1732	namespace	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	57	34	1699	1731	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/static_http/src/service.panack	61	19	1838	1876	namespace	stderr	src/stdlib/async_host.panack:stderr	Str	Result[Unit,src/stdlib/host.panack:HostError]	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/service.panack	62	3	1879	1925	call	error_response	packages/static_http/src/service.panack:error_response	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	70	5	2276	2323	call	error_response	packages/static_http/src/service.panack:error_response	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	73	11	2447	2536	call	diagnostic	packages/static_http/src/service.panack:diagnostic	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	75	11	2558	2617	call	diagnostic	packages/static_http/src/service.panack:diagnostic	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	82	11	2823	2885	call	diagnostic	packages/static_http/src/service.panack:diagnostic	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	84	16	2912	3047	namespace	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	85	7	2937	3025	receiver	append	core:append	Array[packages/http/src/types.panack:Header]	Array[packages/http/src/types.panack:Header]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/static_http/src/service.panack	85	36	2966	3024	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	85	59	2989	3023	namespace	content_type	packages/static_http/src/mime.panack:content_type	Str	Str	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	85	77	3007	3022	call	lower	packages/static_http/src/service.panack:lower	Str	Str	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	87	11	3058	3091	namespace	serialize_response	packages/http/src/server.panack:serialize_response	packages/http/src/types.panack:Response	Result[Bytes,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	88	31	3124	3196	call	diagnostic	packages/static_http/src/service.panack:diagnostic	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	89	22	3219	3238	namespace	length	src/stdlib/bytes.panack:length	Bytes	Nat	bound-node	Owned: Bytes.length	P08/P10/P11
packages/static_http/src/service.panack	90	15	3293	3365	call	diagnostic	packages/static_http/src/service.panack:diagnostic	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	99	9	3615	3639	namespace	from_text	src/stdlib/path.panack:from_text	Str	Result[Path,src/stdlib/path.panack:PathError]	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/service.panack	100	23	3664	3714	call	error_response	packages/static_http/src/service.panack:error_response	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	101	33	3748	3844	namespace	read_beneath	src/stdlib/async_filesystem.panack:read_beneath	Path	Result[src/stdlib/package_filesystem.panack:FilesystemRootRead,src/stdlib/host.panack:HostError]	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/service.panack	103	31	3877	3913	call	read_failure	packages/static_http/src/service.panack:read_failure	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	105	15	3986	4048	call	diagnostic	packages/static_http/src/service.panack:diagnostic	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	107	9	4089	4200	namespace	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	107	28	4108	4173	receiver	append	core:append	Array[packages/http/src/types.panack:Header]	Array[packages/http/src/types.panack:Header]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/static_http/src/service.panack	107	57	4137	4172	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	108	11	4185	4199	namespace	empty	src/stdlib/bytes.panack:empty		Bytes	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/service.panack	109	22	4222	4260	call	file_response	packages/static_http/src/service.panack:file_response	packages/static_http/src/service.panack:Context	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	117	5	4486	4612	namespace	Response	packages/http/src/types.panack:Response	Nat	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	117	24	4505	4563	receiver	append	core:append	Array[packages/http/src/types.panack:Header]	Array[packages/http/src/types.panack:Header]	bound-node	owned:Array[T].append	P06/P08; callers P10/P11; remove P12
packages/static_http/src/service.panack	117	53	4534	4562	call	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	118	7	4571	4611	namespace	encode_utf8	src/stdlib/text.panack:encode_utf8	Str	Bytes	bound-node	Owned: Str.encode_utf8	P08/P10/P11
packages/static_http/src/service.panack	120	11	4634	4661	namespace	parse	packages/static_http/src/paths.panack:parse	Str	Result[packages/static_http/src/paths.panack:Target,Str]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	121	25	4688	4742	call	error_response	packages/static_http/src/service.panack:error_response	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	123	15	4796	4827	call	index_response	packages/static_http/src/service.panack:index_response	packages/static_http/src/service.panack:Context	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	125	15	4857	4888	namespace	from_text	src/stdlib/path.panack:from_text	Str	Result[Path,src/stdlib/path.panack:PathError]	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/service.panack	126	29	4919	4969	call	error_response	packages/static_http/src/service.panack:error_response	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	127	39	5009	5111	namespace	read_beneath	src/stdlib/async_filesystem.panack:read_beneath	Path	Result[src/stdlib/package_filesystem.panack:FilesystemRootRead,src/stdlib/host.panack:HostError]	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/service.panack	129	31	5144	5189	call	file_response	packages/static_http/src/service.panack:file_response	packages/static_http/src/service.panack:Context	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	131	21	5315	5346	call	index_response	packages/static_http/src/service.panack:index_response	packages/static_http/src/service.panack:Context	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	132	28	5374	5410	call	read_failure	packages/static_http/src/service.panack:read_failure	packages/static_http/src/service.panack:Config	packages/http/src/types.panack:Response	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	138	3	5455	5467	call	Ok	core:Ok	packages/http/src/types.panack:Response	Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/service.panack	142	12	5559	5642	namespace	ConnectionPolicy	packages/http/src/server.panack:ConnectionPolicy	Array[Str]	packages/http/src/server.panack:ConnectionPolicy	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	143	9	5651	5710	namespace	serve_connection_with_policy	packages/http/src/server.panack:serve_connection_with_policy	packages/http/src/server.panack:ConnectionContext[packages/static_http/src/service.panack:Context]	Result[Bytes,Str]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	170	7	6892	6920	call	headers_valid	packages/static_http/src/service.panack:headers_valid	packages/static_http/src/service.panack:Config	Bool	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	170	38	6923	6966	call	Error	core:Error	Str	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/service.panack	172	17	6994	7038	namespace	root_identity	src/stdlib/async_filesystem.panack:root_identity	Path	Result[src/stdlib/package_filesystem.panack:FilesystemRootIdentity,src/stdlib/host.panack:HostError]	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/src/service.panack	173	25	7065	7112	call	Error	core:Error	Str	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/service.panack	175	19	7156	7188	call	Context	packages/static_http/src/service.panack:Context	packages/static_http/src/service.panack:Config	packages/static_http/src/service.panack:Context	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	176	55	7243	7367	namespace	ConnectionContext	packages/http/src/server.panack:ConnectionContext	packages/static_http/src/service.panack:Context	packages/http/src/server.panack:ConnectionContext[packages/static_http/src/service.panack:Context]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	176	87	7275	7283	ref	respond	ref:packages/static_http/src/service.panack:respond		AsyncFn[packages/static_http/src/service.panack:Context,packages/http/src/types.panack:Request,Result[packages/http/src/types.panack:Response,packages/http/src/types.panack:HttpError]]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	178	21	7388	7513	namespace	serve	packages/http/src/service.panack:serve	Str	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,packages/http/src/types.panack:HttpError]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	178	87	7454	7461	ref	worker	ref:packages/static_http/src/service.panack:worker		AsyncFn[packages/http/src/server.panack:ConnectionContext[packages/static_http/src/service.panack:Context],Result[Bytes,Str]]	bound-node	retain-explicit-declaration	P11
packages/static_http/src/service.panack	180	26	7541	7552	call	Ok	core:Ok	src/stdlib/tcp_service.panack:TcpServiceSummary	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/service.panack	181	29	7582	7637	call	Error	core:Error	Str	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/src/service.panack	181	61	7614	7636	call	service_error	packages/static_http/src/service.panack:service_error	packages/http/src/types.panack:HttpError	Str	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	6	3	144	146	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
packages/static_http/tests/src/main.panack	10	3	230	259	call	Error	core:Error	Str	Result[Unit,Str]	bound-node	explicit-service/internal-primitive	P10/P11/P12
packages/static_http/tests/src/main.panack	14	19	339	453	namespace	Config	packages/static_http/src/service.panack:Config	Path	packages/static_http/src/service.panack:Config	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	14	34	354	368	namespace	current	src/stdlib/path.panack:current		Path	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/tests/src/main.panack	15	5	390	443	namespace	ServiceLimits	packages/http/src/service.panack:ServiceLimits	Nat	packages/http/src/service.panack:ServiceLimits	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	16	19	472	519	namespace	serve	packages/static_http/src/service.panack:serve	packages/static_http/src/service.panack:Config	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,Str]	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	16	48	501	518	ref	unexpected_ready	ref:packages/static_http/tests/src/main.panack:unexpected_ready		AsyncFn[src/stdlib/tcp_service.panack:TcpServiceReady,Result[Unit,Str]]	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	17	3	522	645	call	expect	packages/static_http/tests/src/main.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	25	11	787	839	call	rejected_headers	packages/static_http/tests/src/main.panack:rejected_headers	Array[packages/http/src/types.panack:Header]	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	25	29	805	837	namespace	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	27	9	852	911	call	rejected_headers	packages/static_http/tests/src/main.panack:rejected_headers	Array[packages/http/src/types.panack:Header]	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	27	27	870	909	namespace	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	28	9	920	990	call	rejected_headers	packages/static_http/tests/src/main.panack:rejected_headers	Array[packages/http/src/types.panack:Header]	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	28	27	938	988	namespace	Header	packages/http/src/types.panack:Header	Str	packages/http/src/types.panack:Header	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	29	11	1001	1106	namespace	Config	packages/static_http/src/service.panack:Config	Path	packages/static_http/src/service.panack:Config	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	29	26	1016	1030	namespace	current	src/stdlib/path.panack:current		Path	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/tests/src/main.panack	30	5	1052	1101	namespace	ServiceLimits	packages/http/src/service.panack:ServiceLimits	Nat	packages/http/src/service.panack:ServiceLimits	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	31	3	1109	1270	call	expect	packages/static_http/tests/src/main.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	31	22	1128	1167	namespace	serve	packages/static_http/src/service.panack:serve	packages/static_http/src/service.panack:Config	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,Str]	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	31	43	1149	1166	ref	unexpected_ready	ref:packages/static_http/tests/src/main.panack:unexpected_ready		AsyncFn[src/stdlib/tcp_service.panack:TcpServiceReady,Result[Unit,Str]]	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	35	9	1279	1306	namespace	from_text	src/stdlib/path.panack:from_text	Str	Result[Path,src/stdlib/path.panack:PathError]	bound-node	Retain explicit module function	P08/P10/P11
packages/static_http/tests/src/main.panack	36	23	1331	1344	call	expect	packages/static_http/tests/src/main.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	38	17	1380	1483	namespace	Config	packages/static_http/src/service.panack:Config	Path	packages/static_http/src/service.panack:Config	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	39	9	1425	1478	namespace	ServiceLimits	packages/http/src/service.panack:ServiceLimits	Nat	packages/http/src/service.panack:ServiceLimits	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	40	7	1490	1667	call	expect	packages/static_http/tests/src/main.panack:expect	Bool	Unit	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	40	26	1509	1550	namespace	serve	packages/static_http/src/service.panack:serve	packages/static_http/src/service.panack:Config	Result[src/stdlib/tcp_service.panack:TcpServiceSummary,Str]	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	40	49	1532	1549	ref	unexpected_ready	ref:packages/static_http/tests/src/main.panack:unexpected_ready		AsyncFn[src/stdlib/tcp_service.panack:TcpServiceReady,Result[Unit,Str]]	bound-node	retain-explicit-declaration	P11
packages/static_http/tests/src/main.panack	46	3	1680	1682	call	$unit	core:$unit		Unit	bound-node	retain-constructor	P05/P12
```
