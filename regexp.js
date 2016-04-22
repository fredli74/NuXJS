"use strict";

var v = 1;
var EMPTY_CHAR = v, NEWLINE_CHAR = (v <<= 1), SPACE_CHAR = (v <<= 1), WORD_CHAR = (v <<= 1), DECIMAL_CHAR = (v <<= 1);
var LETTER_CHAR = (v <<= 1), HEX_CHAR = (v <<= 1), ESCAPE_CHAR = (v <<= 1), SPECIAL_CHAR = (v <<= 1);
var OK_IN_STRING_LITERAL = (v <<= 1), IDENTITY_ESCAPE_CHAR = (v <<= 1);
var CC = { };

(function() {
	function setupCharClass(mask, chars) { for (var i in chars) CC[chars[i]] |= mask; }

	setupCharClass(SPECIAL_CHAR, "^$.*+?()[]{}|");
	setupCharClass(DECIMAL_CHAR | HEX_CHAR | WORD_CHAR, "0123456789");
	setupCharClass(HEX_CHAR | LETTER_CHAR | WORD_CHAR, "abcdefABCDEF");
	setupCharClass(LETTER_CHAR | WORD_CHAR, "ghijklmnopqrstuvwxyzGHIJKLMNOPQRSTUVWXYZ");
	setupCharClass(NEWLINE_CHAR | SPACE_CHAR, "\n\r\u2028\u2029");
	setupCharClass(SPACE_CHAR, " \t\v\f\xA0");
	CC['_'] |= WORD_CHAR;
	CC['undefined'] |= EMPTY_CHAR;
	CC[''] |= EMPTY_CHAR;
	setupCharClass(ESCAPE_CHAR, "fnrtv");
	for (var i = 32; i <= 126; ++i) {
		var c = String.fromCharCode(i);
		if (c !== '"' && c !== '\\') {
			CC[String.fromCharCode(i)] |= OK_IN_STRING_LITERAL;
		}
	}
	var IDENTITY_ESCAPE_RANGES = [ 0, 48, 58, 65, 91, 95, 96, 97, 123, 128 ];
	for (var i = IDENTITY_ESCAPE_RANGES.length - 2; i >= 0; i -= 2) {
		for (var j = IDENTITY_ESCAPE_RANGES[i], k = IDENTITY_ESCAPE_RANGES[i + 1]; j < k; ++j) {
			CC[String.fromCharCode(j)] |= IDENTITY_ESCAPE_CHAR;
		}
	}
})();

					var $lowerToUpper = {
							"\xdf":"SS","\u0131":"I","\u0149":"\u02bcn","\u017f":"S","\u01c5":"\u01c4","\u01c8":"\u01c7","\u01cb":"\u01ca","\u01f0":"j\u030c","\u01f2":"\u01f1","\u0345":"\u0399","\u0390":"\u03b9\u0308\u030d","\u03b0":"\u03c5\u0308\u030d","\u03c2":"\u03a3","\u03d0":"\u0392","\u03d1":"\u0398","\u03d5":"\u03a6","\u03d6":"\u03a0","\u03f0":"\u039a","\u03f1":"\u03a1","\u03f2":"\u03a3","\u0587":"\u0535\u0552","\u1e96":"h\u0331","\u1e97":"t\u0308","\u1e98":"w\u030a","\u1e99":"y\u030a","\u1e9a":"a\u02be","\u1e9b":"\u1e60","\u1f50":"\u03c5\u0313","\u1f52":"\u03c5\u0313\u0300",
							"\u1f54":"\u03c5\u0313\u0301","\u1f56":"\u03c5\u0313\u0342","\u1f80":"\u1f08\u03b9","\u1f81":"\u1f09\u03b9","\u1f82":"\u1f0a\u03b9","\u1f83":"\u1f0b\u03b9","\u1f84":"\u1f0c\u03b9","\u1f85":"\u1f0d\u03b9","\u1f86":"\u1f0e\u03b9","\u1f87":"\u1f0f\u03b9","\u1f88":"\u1f08\u03b9","\u1f90":"\u1f28\u03b9","\u1f91":"\u1f29\u03b9","\u1f92":"\u1f2a\u03b9","\u1f93":"\u1f2b\u03b9","\u1f94":"\u1f2c\u03b9","\u1f95":"\u1f2d\u03b9","\u1f96":"\u1f2e\u03b9","\u1f97":"\u1f2f\u03b9","\u1fa0":"\u1f68\u03b9","\u1fa1":"\u1f69\u03b9","\u1fa2":"\u1f6a\u03b9","\u1fa3":"\u1f6b\u03b9",
							"\u1fa4":"\u1f6c\u03b9","\u1fa5":"\u1f6d\u03b9","\u1fa6":"\u1f6e\u03b9","\u1fa7":"\u1f6f\u03b9","\u1fb2":"\u1f70\u03b9","\u1fb3":"\u0391\u03b9","\u1fb4":"\u1f71\u03b9","\u1fb6":"\u03b1\u0342","\u1fb7":"\u1fb6\u03b9","\u1fbe":"\u0399","\u1fc2":"\u1f74\u03b9","\u1fc3":"\u0397\u03b9","\u1fc4":"\u1f75\u03b9","\u1fc6":"\u03b7\u0342","\u1fc7":"\u1fc6\u03b9","\u1fd2":"\u03b9\u0308\u0300","\u1fd3":"\u03b9\u0308\u0301","\u1fd6":"\u03b9\u0342","\u1fd7":"\u03b9\u0308\u0342","\u1fe2":"\u03c5\u0308\u0300","\u1fe3":"\u03c5\u0308\u0301","\u1fe4":"\u03c1\u0313","\u1fe6":"\u03c5\u0342",
							"\u1fe7":"\u03c5\u0308\u0342","\u1ff2":"\u1f7c\u03b9","\u1ff3":"\u03a9\u03b9","\u1ff4":"\u1f7d\u03b9","\u1ff6":"\u03c9\u0342","\u1ff7":"\u1ff6\u03b9","\ufb00":"FF","\ufb01":"FI","\ufb02":"FL","\ufb03":"FFI","\ufb04":"FFL","\ufb05":"ST","\ufb06":"ST","\ufb13":"\u0544\u0546","\ufb14":"\u0544\u0535","\ufb15":"\u0544\u053b","\ufb16":"\u054e\u0546","\ufb17":"\u0544\u053d"
						};
						var $upperToLower = {
							"\u0130":"i","\u01c5":"\u01c6","\u01c8":"\u01c9","\u01cb":"\u01cc","\u01f2":"\u01f3","\u10a0":"\u10d0","\u10a1":"\u10d1","\u10a2":"\u10d2","\u10a3":"\u10d3","\u10a4":"\u10d4","\u10a5":"\u10d5","\u10a6":"\u10d6","\u10a7":"\u10d7","\u10a8":"\u10d8","\u10a9":"\u10d9","\u10aa":"\u10da","\u10ab":"\u10db","\u10ac":"\u10dc","\u10ad":"\u10dd","\u10ae":"\u10de","\u10af":"\u10df","\u10b0":"\u10e0","\u10b1":"\u10e1","\u10b2":"\u10e2","\u10b3":"\u10e3","\u10b4":"\u10e4","\u10b5":"\u10e5","\u10b6":"\u10e6","\u10b7":"\u10e7","\u10b8":"\u10e8","\u10b9":"\u10e9","\u10ba":"\u10ea",
							"\u10bb":"\u10eb","\u10bc":"\u10ec","\u10bd":"\u10ed","\u10be":"\u10ee","\u10bf":"\u10ef","\u10c0":"\u10f0","\u10c1":"\u10f1","\u10c2":"\u10f2","\u10c3":"\u10f3","\u10c4":"\u10f4","\u10c5":"\u10f5","\u1f88":"\u1f80","\u1f89":"\u1f81","\u1f8a":"\u1f82","\u1f8b":"\u1f83","\u1f8c":"\u1f84","\u1f8d":"\u1f85","\u1f8e":"\u1f86","\u1f8f":"\u1f87","\u1f98":"\u1f90","\u1f99":"\u1f91","\u1f9a":"\u1f92","\u1f9b":"\u1f93","\u1f9c":"\u1f94","\u1f9d":"\u1f95","\u1f9e":"\u1f96","\u1f9f":"\u1f97","\u1fa8":"\u1fa0","\u1fa9":"\u1fa1","\u1faa":"\u1fa2","\u1fab":"\u1fa3","\u1fac":"\u1fa4",
							"\u1fad":"\u1fa5","\u1fae":"\u1fa6","\u1faf":"\u1fa7","\u1fbc":"\u1fb3","\u1fcc":"\u1fc3","\u1ffc":"\u1ff3"
						};
						(function() {
							var c, bidirectional = {
								A:"a",B:"b",C:"c",D:"d",E:"e",F:"f",G:"g",H:"h",I:"i",J:"j",K:"k",L:"l",M:"m",N:"n",O:"o",P:"p",Q:"q",R:"r",S:"s",T:"t",U:"u",V:"v",W:"w",X:"x",Y:"y",Z:"z","\xc0":"\xe0","\xc1":"\xe1","\xc2":"\xe2","\xc3":"\xe3","\xc4":"\xe4","\xc5":"\xe5","\xc6":"\xe6","\xc7":"\xe7","\xc8":"\xe8","\xc9":"\xe9","\xca":"\xea","\xcb":"\xeb","\xcc":"\xec","\xcd":"\xed","\xce":"\xee","\xcf":"\xef","\xd0":"\xf0","\xd1":"\xf1","\xd2":"\xf2","\xd3":"\xf3","\xd4":"\xf4","\xd5":"\xf5","\xd6":"\xf6","\xd8":"\xf8","\xd9":"\xf9","\xda":"\xfa","\xdb":"\xfb","\xdc":"\xfc","\xdd":"\xfd","\xde":"\xfe",
								"\u0100":"\u0101","\u0102":"\u0103","\u0104":"\u0105","\u0106":"\u0107","\u0108":"\u0109","\u010a":"\u010b","\u010c":"\u010d","\u010e":"\u010f","\u0110":"\u0111","\u0112":"\u0113","\u0114":"\u0115","\u0116":"\u0117","\u0118":"\u0119","\u011a":"\u011b","\u011c":"\u011d","\u011e":"\u011f","\u0120":"\u0121","\u0122":"\u0123","\u0124":"\u0125","\u0126":"\u0127","\u0128":"\u0129","\u012a":"\u012b","\u012c":"\u012d","\u012e":"\u012f","\u0132":"\u0133","\u0134":"\u0135","\u0136":"\u0137","\u0139":"\u013a","\u013b":"\u013c","\u013d":"\u013e","\u013f":"\u0140","\u0141":"\u0142",
								"\u0143":"\u0144","\u0145":"\u0146","\u0147":"\u0148","\u014a":"\u014b","\u014c":"\u014d","\u014e":"\u014f","\u0150":"\u0151","\u0152":"\u0153","\u0154":"\u0155","\u0156":"\u0157","\u0158":"\u0159","\u015a":"\u015b","\u015c":"\u015d","\u015e":"\u015f","\u0160":"\u0161","\u0162":"\u0163","\u0164":"\u0165","\u0166":"\u0167","\u0168":"\u0169","\u016a":"\u016b","\u016c":"\u016d","\u016e":"\u016f","\u0170":"\u0171","\u0172":"\u0173","\u0174":"\u0175","\u0176":"\u0177","\u0178":"\xff","\u0179":"\u017a","\u017b":"\u017c","\u017d":"\u017e","\u0181":"\u0253","\u0182":"\u0183",
								"\u0184":"\u0185","\u0186":"\u0254","\u0187":"\u0188","\u0189":"\u0256","\u018a":"\u0257","\u018b":"\u018c","\u018e":"\u01dd","\u018f":"\u0259","\u0190":"\u025b","\u0191":"\u0192","\u0193":"\u0260","\u0194":"\u0263","\u0196":"\u0269","\u0197":"\u0268","\u0198":"\u0199","\u019c":"\u026f","\u019d":"\u0272","\u019f":"\u0275","\u01a0":"\u01a1","\u01a2":"\u01a3","\u01a4":"\u01a5","\u01a6":"\u0280","\u01a7":"\u01a8","\u01a9":"\u0283","\u01ac":"\u01ad","\u01ae":"\u0288","\u01af":"\u01b0","\u01b1":"\u028a","\u01b2":"\u028b","\u01b3":"\u01b4","\u01b5":"\u01b6","\u01b7":"\u0292",
								"\u01b8":"\u01b9","\u01bc":"\u01bd","\u01c4":"\u01c6","\u01c7":"\u01c9","\u01ca":"\u01cc","\u01cd":"\u01ce","\u01cf":"\u01d0","\u01d1":"\u01d2","\u01d3":"\u01d4","\u01d5":"\u01d6","\u01d7":"\u01d8","\u01d9":"\u01da","\u01db":"\u01dc","\u01de":"\u01df","\u01e0":"\u01e1","\u01e2":"\u01e3","\u01e4":"\u01e5","\u01e6":"\u01e7","\u01e8":"\u01e9","\u01ea":"\u01eb","\u01ec":"\u01ed","\u01ee":"\u01ef","\u01f1":"\u01f3","\u01f4":"\u01f5","\u01fa":"\u01fb","\u01fc":"\u01fd","\u01fe":"\u01ff","\u0200":"\u0201","\u0202":"\u0203","\u0204":"\u0205","\u0206":"\u0207","\u0208":"\u0209",
								"\u020a":"\u020b","\u020c":"\u020d","\u020e":"\u020f","\u0210":"\u0211","\u0212":"\u0213","\u0214":"\u0215","\u0216":"\u0217","\u0386":"\u03ac","\u0388":"\u03ad","\u0389":"\u03ae","\u038a":"\u03af","\u038c":"\u03cc","\u038e":"\u03cd","\u038f":"\u03ce","\u0391":"\u03b1","\u0392":"\u03b2","\u0393":"\u03b3","\u0394":"\u03b4","\u0395":"\u03b5","\u0396":"\u03b6","\u0397":"\u03b7","\u0398":"\u03b8","\u0399":"\u03b9","\u039a":"\u03ba","\u039b":"\u03bb","\u039c":"\u03bc","\u039d":"\u03bd","\u039e":"\u03be","\u039f":"\u03bf","\u03a0":"\u03c0","\u03a1":"\u03c1","\u03a3":"\u03c3",
								"\u03a4":"\u03c4","\u03a5":"\u03c5","\u03a6":"\u03c6","\u03a7":"\u03c7","\u03a8":"\u03c8","\u03a9":"\u03c9","\u03aa":"\u03ca","\u03ab":"\u03cb","\u03e2":"\u03e3","\u03e4":"\u03e5","\u03e6":"\u03e7","\u03e8":"\u03e9","\u03ea":"\u03eb","\u03ec":"\u03ed","\u03ee":"\u03ef","\u0401":"\u0451","\u0402":"\u0452","\u0403":"\u0453","\u0404":"\u0454","\u0405":"\u0455","\u0406":"\u0456","\u0407":"\u0457","\u0408":"\u0458","\u0409":"\u0459","\u040a":"\u045a","\u040b":"\u045b","\u040c":"\u045c","\u040e":"\u045e","\u040f":"\u045f","\u0410":"\u0430","\u0411":"\u0431","\u0412":"\u0432",
								"\u0413":"\u0433","\u0414":"\u0434","\u0415":"\u0435","\u0416":"\u0436","\u0417":"\u0437","\u0418":"\u0438","\u0419":"\u0439","\u041a":"\u043a","\u041b":"\u043b","\u041c":"\u043c","\u041d":"\u043d","\u041e":"\u043e","\u041f":"\u043f","\u0420":"\u0440","\u0421":"\u0441","\u0422":"\u0442","\u0423":"\u0443","\u0424":"\u0444","\u0425":"\u0445","\u0426":"\u0446","\u0427":"\u0447","\u0428":"\u0448","\u0429":"\u0449","\u042a":"\u044a","\u042b":"\u044b","\u042c":"\u044c","\u042d":"\u044d","\u042e":"\u044e","\u042f":"\u044f","\u0460":"\u0461","\u0462":"\u0463","\u0464":"\u0465",
								"\u0466":"\u0467","\u0468":"\u0469","\u046a":"\u046b","\u046c":"\u046d","\u046e":"\u046f","\u0470":"\u0471","\u0472":"\u0473","\u0474":"\u0475","\u0476":"\u0477","\u0478":"\u0479","\u047a":"\u047b","\u047c":"\u047d","\u047e":"\u047f","\u0480":"\u0481","\u0490":"\u0491","\u0492":"\u0493","\u0494":"\u0495","\u0496":"\u0497","\u0498":"\u0499","\u049a":"\u049b","\u049c":"\u049d","\u049e":"\u049f","\u04a0":"\u04a1","\u04a2":"\u04a3","\u04a4":"\u04a5","\u04a6":"\u04a7","\u04a8":"\u04a9","\u04aa":"\u04ab","\u04ac":"\u04ad","\u04ae":"\u04af","\u04b0":"\u04b1","\u04b2":"\u04b3",
								"\u04b4":"\u04b5","\u04b6":"\u04b7","\u04b8":"\u04b9","\u04ba":"\u04bb","\u04bc":"\u04bd","\u04be":"\u04bf","\u04c1":"\u04c2","\u04c3":"\u04c4","\u04c7":"\u04c8","\u04cb":"\u04cc","\u04d0":"\u04d1","\u04d2":"\u04d3","\u04d4":"\u04d5","\u04d6":"\u04d7","\u04d8":"\u04d9","\u04da":"\u04db","\u04dc":"\u04dd","\u04de":"\u04df","\u04e0":"\u04e1","\u04e2":"\u04e3","\u04e4":"\u04e5","\u04e6":"\u04e7","\u04e8":"\u04e9","\u04ea":"\u04eb","\u04ee":"\u04ef","\u04f0":"\u04f1","\u04f2":"\u04f3","\u04f4":"\u04f5","\u04f8":"\u04f9","\u0531":"\u0561","\u0532":"\u0562","\u0533":"\u0563",
								"\u0534":"\u0564","\u0535":"\u0565","\u0536":"\u0566","\u0537":"\u0567","\u0538":"\u0568","\u0539":"\u0569","\u053a":"\u056a","\u053b":"\u056b","\u053c":"\u056c","\u053d":"\u056d","\u053e":"\u056e","\u053f":"\u056f","\u0540":"\u0570","\u0541":"\u0571","\u0542":"\u0572","\u0543":"\u0573","\u0544":"\u0574","\u0545":"\u0575","\u0546":"\u0576","\u0547":"\u0577","\u0548":"\u0578","\u0549":"\u0579","\u054a":"\u057a","\u054b":"\u057b","\u054c":"\u057c","\u054d":"\u057d","\u054e":"\u057e","\u054f":"\u057f","\u0550":"\u0580","\u0551":"\u0581","\u0552":"\u0582","\u0553":"\u0583",
								"\u0554":"\u0584","\u0555":"\u0585","\u0556":"\u0586","\u1e00":"\u1e01","\u1e02":"\u1e03","\u1e04":"\u1e05","\u1e06":"\u1e07","\u1e08":"\u1e09","\u1e0a":"\u1e0b","\u1e0c":"\u1e0d","\u1e0e":"\u1e0f","\u1e10":"\u1e11","\u1e12":"\u1e13","\u1e14":"\u1e15","\u1e16":"\u1e17","\u1e18":"\u1e19","\u1e1a":"\u1e1b","\u1e1c":"\u1e1d","\u1e1e":"\u1e1f","\u1e20":"\u1e21","\u1e22":"\u1e23","\u1e24":"\u1e25","\u1e26":"\u1e27","\u1e28":"\u1e29","\u1e2a":"\u1e2b","\u1e2c":"\u1e2d","\u1e2e":"\u1e2f","\u1e30":"\u1e31","\u1e32":"\u1e33","\u1e34":"\u1e35","\u1e36":"\u1e37","\u1e38":"\u1e39",
								"\u1e3a":"\u1e3b","\u1e3c":"\u1e3d","\u1e3e":"\u1e3f","\u1e40":"\u1e41","\u1e42":"\u1e43","\u1e44":"\u1e45","\u1e46":"\u1e47","\u1e48":"\u1e49","\u1e4a":"\u1e4b","\u1e4c":"\u1e4d","\u1e4e":"\u1e4f","\u1e50":"\u1e51","\u1e52":"\u1e53","\u1e54":"\u1e55","\u1e56":"\u1e57","\u1e58":"\u1e59","\u1e5a":"\u1e5b","\u1e5c":"\u1e5d","\u1e5e":"\u1e5f","\u1e60":"\u1e61","\u1e62":"\u1e63","\u1e64":"\u1e65","\u1e66":"\u1e67","\u1e68":"\u1e69","\u1e6a":"\u1e6b","\u1e6c":"\u1e6d","\u1e6e":"\u1e6f","\u1e70":"\u1e71","\u1e72":"\u1e73","\u1e74":"\u1e75","\u1e76":"\u1e77","\u1e78":"\u1e79",
								"\u1e7a":"\u1e7b","\u1e7c":"\u1e7d","\u1e7e":"\u1e7f","\u1e80":"\u1e81","\u1e82":"\u1e83","\u1e84":"\u1e85","\u1e86":"\u1e87","\u1e88":"\u1e89","\u1e8a":"\u1e8b","\u1e8c":"\u1e8d","\u1e8e":"\u1e8f","\u1e90":"\u1e91","\u1e92":"\u1e93","\u1e94":"\u1e95","\u1ea0":"\u1ea1","\u1ea2":"\u1ea3","\u1ea4":"\u1ea5","\u1ea6":"\u1ea7","\u1ea8":"\u1ea9","\u1eaa":"\u1eab","\u1eac":"\u1ead","\u1eae":"\u1eaf","\u1eb0":"\u1eb1","\u1eb2":"\u1eb3","\u1eb4":"\u1eb5","\u1eb6":"\u1eb7","\u1eb8":"\u1eb9","\u1eba":"\u1ebb","\u1ebc":"\u1ebd","\u1ebe":"\u1ebf","\u1ec0":"\u1ec1","\u1ec2":"\u1ec3",
								"\u1ec4":"\u1ec5","\u1ec6":"\u1ec7","\u1ec8":"\u1ec9","\u1eca":"\u1ecb","\u1ecc":"\u1ecd","\u1ece":"\u1ecf","\u1ed0":"\u1ed1","\u1ed2":"\u1ed3","\u1ed4":"\u1ed5","\u1ed6":"\u1ed7","\u1ed8":"\u1ed9","\u1eda":"\u1edb","\u1edc":"\u1edd","\u1ede":"\u1edf","\u1ee0":"\u1ee1","\u1ee2":"\u1ee3","\u1ee4":"\u1ee5","\u1ee6":"\u1ee7","\u1ee8":"\u1ee9","\u1eea":"\u1eeb","\u1eec":"\u1eed","\u1eee":"\u1eef","\u1ef0":"\u1ef1","\u1ef2":"\u1ef3","\u1ef4":"\u1ef5","\u1ef6":"\u1ef7","\u1ef8":"\u1ef9","\u1f08":"\u1f00","\u1f09":"\u1f01","\u1f0a":"\u1f02","\u1f0b":"\u1f03","\u1f0c":"\u1f04",
								"\u1f0d":"\u1f05","\u1f0e":"\u1f06","\u1f0f":"\u1f07","\u1f18":"\u1f10","\u1f19":"\u1f11","\u1f1a":"\u1f12","\u1f1b":"\u1f13","\u1f1c":"\u1f14","\u1f1d":"\u1f15","\u1f28":"\u1f20","\u1f29":"\u1f21","\u1f2a":"\u1f22","\u1f2b":"\u1f23","\u1f2c":"\u1f24","\u1f2d":"\u1f25","\u1f2e":"\u1f26","\u1f2f":"\u1f27","\u1f38":"\u1f30","\u1f39":"\u1f31","\u1f3a":"\u1f32","\u1f3b":"\u1f33","\u1f3c":"\u1f34","\u1f3d":"\u1f35","\u1f3e":"\u1f36","\u1f3f":"\u1f37","\u1f48":"\u1f40","\u1f49":"\u1f41","\u1f4a":"\u1f42","\u1f4b":"\u1f43","\u1f4c":"\u1f44","\u1f4d":"\u1f45","\u1f59":"\u1f51",
								"\u1f5b":"\u1f53","\u1f5d":"\u1f55","\u1f5f":"\u1f57","\u1f68":"\u1f60","\u1f69":"\u1f61","\u1f6a":"\u1f62","\u1f6b":"\u1f63","\u1f6c":"\u1f64","\u1f6d":"\u1f65","\u1f6e":"\u1f66","\u1f6f":"\u1f67","\u1fb8":"\u1fb0","\u1fb9":"\u1fb1","\u1fba":"\u1f70","\u1fbb":"\u1f71","\u1fc8":"\u1f72","\u1fc9":"\u1f73","\u1fca":"\u1f74","\u1fcb":"\u1f75","\u1fd8":"\u1fd0","\u1fd9":"\u1fd1","\u1fda":"\u1f76","\u1fdb":"\u1f77","\u1fe8":"\u1fe0","\u1fe9":"\u1fe1","\u1fea":"\u1f7a","\u1feb":"\u1f7b","\u1fec":"\u1fe5","\u1ff8":"\u1f78","\u1ff9":"\u1f79","\u1ffa":"\u1f7c","\u1ffb":"\u1f7d",
								"\u2160":"\u2170","\u2161":"\u2171","\u2162":"\u2172","\u2163":"\u2173","\u2164":"\u2174","\u2165":"\u2175","\u2166":"\u2176","\u2167":"\u2177","\u2168":"\u2178","\u2169":"\u2179","\u216a":"\u217a","\u216b":"\u217b","\u216c":"\u217c","\u216d":"\u217d","\u216e":"\u217e","\u216f":"\u217f","\u24b6":"\u24d0","\u24b7":"\u24d1","\u24b8":"\u24d2","\u24b9":"\u24d3","\u24ba":"\u24d4","\u24bb":"\u24d5","\u24bc":"\u24d6","\u24bd":"\u24d7","\u24be":"\u24d8","\u24bf":"\u24d9","\u24c0":"\u24da","\u24c1":"\u24db","\u24c2":"\u24dc","\u24c3":"\u24dd","\u24c4":"\u24de","\u24c5":"\u24df",
								"\u24c6":"\u24e0","\u24c7":"\u24e1","\u24c8":"\u24e2","\u24c9":"\u24e3","\u24ca":"\u24e4","\u24cb":"\u24e5","\u24cc":"\u24e6","\u24cd":"\u24e7","\u24ce":"\u24e8","\u24cf":"\u24e9","\uff21":"\uff41","\uff22":"\uff42","\uff23":"\uff43","\uff24":"\uff44","\uff25":"\uff45","\uff26":"\uff46","\uff27":"\uff47","\uff28":"\uff48","\uff29":"\uff49","\uff2a":"\uff4a","\uff2b":"\uff4b","\uff2c":"\uff4c","\uff2d":"\uff4d","\uff2e":"\uff4e","\uff2f":"\uff4f","\uff30":"\uff50","\uff31":"\uff51","\uff32":"\uff52","\uff33":"\uff53","\uff34":"\uff54","\uff35":"\uff55","\uff36":"\uff56",
								"\uff37":"\uff57","\uff38":"\uff58","\uff39":"\uff59","\uff3a":"\uff5a"
							};
							for (c in bidirectional) $lowerToUpper[$upperToLower[c] = bidirectional[c]] = c;
						})();


function $regExpCanonicalize(s) {
	var t = '', c, d;
	for (var i = 0, l = s.length; i < l; ++i) {
		t += ((d = $lowerToUpper[c = s[i]]) && d.length === 1 && (c < '\x80' || d >= '\x80') ? d : c);
	}
	return t;
}
/*
print($regExpCanonicalize('abcdABCD1234'));
var test = $regExpCanonicalize('\xdf\u0131\u0185');
print(test.charCodeAt(0).toString(16));
print(test.charCodeAt(1).toString(16));
print(test.charCodeAt(2).toString(16));
*/

function compileRegExp(s, ignoreCase, multiline) {
	function isClass(char, mask) {
		return ((CC[char] & mask) !== 0);
	}

	function areClass(str, mask) { // FIX : only used once
		for (var i = str.length - 1; i >= 0; --i) {
			if ((CC[str[i]] & mask) === 0) {
				return false;
			}
		}
		return true;
	}

	var CHAR_CLASS_RULES = {
		'D': [ DECIMAL_CHAR, true ], 'd': [ DECIMAL_CHAR, false ],
		'S': [ SPACE_CHAR, true ], 's': [ SPACE_CHAR, false ],
		'W': [ WORD_CHAR, true ], 'w': [ WORD_CHAR, false ],
		'.': [ NEWLINE_CHAR, true ]
	};

	var p = 0, functions = '', functionCounter = 0, captureCounter = 0, closureVars = '', maxBackReference = 0;

	function parseNumber(defaultValue) {
		var n = defaultValue;
		if (isClass(s[p], DECIMAL_CHAR)) {
			n = 0;
			do {
				n = n * 10 + (s.charCodeAt(p) - 48);
				++p;
			} while (isClass(s[p], DECIMAL_CHAR))
		}
		return n;
	}

	function parseQuantifier() {
		var min = 0, max = Infinity, greedy = true;
		switch (s[p]) {
			case '*': ++p; break;
			case '+': ++p; min = 1; break;
			case '?': ++p; max = 1; break;
			case '{': {
				var b = p, n;
				++p;
				if ((min = max = parseNumber(-1)) < 0) {
					p = b;
					return null;
				}
				if (s[p] === ',') {
					++p;
					max = parseNumber(Infinity);
				}
				if (s[p] !== '}') {
					p = b;
					return null;
				}
				if (min > max) {
					throw SyntaxError("Min greater than max in regular expression quantifier"); // FIX
				}
				++p;
				break;
			}
			default: return null;
		}
		if (s[p] === '?') {
			++p;
			greedy = false;
		}
		return { min: min, max: max, greedy: greedy };
	}

	function escapeCharacter(c) {
		if (isClass(c, OK_IN_STRING_LITERAL)) {
			return c;
		} else {
			// TODO : shorter escapes for \n etc
			return (c <= '~' ? "\\x" : "\\u") + ("0000" + c.charCodeAt(0).toString(16)).slice(c <= '~' ? -2 : -4);
		}
	}

	function canonicalizeAndEscape(c) {
		return escapeCharacter(ignoreCase ? $regExpCanonicalize(c) : c);
	}

	function parseLiteralCharacter() {
		var c0, c1, sub;
		if ((c0 = s[p]) === '\\') {
			switch (c1 = s[p + 1]) {
				case '0': {
					if (!isClass(s[p + 2], DECIMAL_CHAR)) {
						p += 2;
						return '\0';
					}
					break;
				}
				case 'c': {
					if (isClass(s[p + 2], LETTER_CHAR)) {
						p += 3;
						return String.fromCharCode(s[p - 1].charCodeAt(0) & 31);
					}
					break;
				}
				case 'x':
				case 'u': {
					var n = (c1 === 'x' ? 2 : 4);
					if (areClass(sub = s.substring(p + 2, p + 2 + n), HEX_CHAR)) {
						p += 2 + n;
						return String.fromCharCode(parseInt(sub, 16));
					}
					break;
				}
				default: {
					if (isClass(c1, ESCAPE_CHAR)) {
						p += 2;
						return eval('"\\' + c1 + '"');
					} else if (isClass(c1, IDENTITY_ESCAPE_CHAR)) {
						p += 2;
						return c1;
					}
					break;
				}
			}
		} else if (c0) {
			++p;
			return c0;
		}
	}

	function parseLiteralSequence() {
		var literalSequence = [ ], v;
		while (!isClass(s[p], SPECIAL_CHAR) && (v = parseLiteralCharacter())) {
			literalSequence.push(canonicalizeAndEscape(v));
		}
		return (literalSequence.length ? literalSequence : null);
	}

	function parseClassAtom() {
		var v, rule, c;
		if ((c = s[p]) !== ']' && (v = parseLiteralCharacter())) {
			return v;
		} else if (c === '\\') {
			if ((c = s[p + 1]) === 'b') {
				p += 2;
				return '\b';
			}
			if (rule = CHAR_CLASS_RULES[c]) {
				p += 2;
				return rule;
			}
		}
	}
	
	function positionToCode(offset) {
		return (offset === 0 ? 'p' : ((offset < 0 ? 'p' : 'p+') + offset));
	}

	function literalSequenceToCode(literalSequence, offset) {
		if (literalSequence.length === 0) {
			return 'true';
		} else if (literalSequence.length === 1) {
			return 's[' + positionToCode(offset) + ']==="' + literalSequence[0] + '"';
		} else if (literalSequence.length === 2) {
			return 's[' + positionToCode(offset) + ']==="' + literalSequence[0] + '" && s[' + positionToCode(offset + 1) + ']==="' + literalSequence[1] + '"';
		} else {
			return 'submatch(s,' + positionToCode(offset) + ',"' + literalSequence.join('') + '")';
		}
	}

	function and(a, b) {
		switch (a) {
			case 'false': return 'false';
			case 'true': return b;
			default: return (b === 'true' ? a : a + ' && ' + b);
		}
	}

	function or(a, b) {
		switch (a) {
			case 'false': return b;
			case 'true': return 'true';
			default: return (b === 'false' ? a: a + ' || ' + b);
		}
	}
	
	function addFunction(name, definition) {
		// TODO : sometimes functions are identical (e.g. class-tests), reuse here or in '[' parsing directly?
		functions += "\tfunction " + name + '(p) { ' + definition + "; }\n";
	}

	function quantify(code, offset, repeatCode, tail, quantity, stepSize) {
		// TODO : eliminate unnecessary b=p+0,e=p+Infinity and stuff
		var functionName = 'q' + (++functionCounter);
		var head = 'var ' + (stepSize ? 'h=' + stepSize + ',b=p+' + quantity.min + '*h,e=p+' + quantity.max + '*h' : 'b=p+' + quantity.min + ',e=p+' + quantity.max) + '; ';
		if (stepSize) {
			head += 'if (h===0 || h!==h) return ' + tail + '; ';
		} 
		if (quantity.greedy) {
			addFunction(functionName, head + 'while (' + and('p<e', repeatCode) + ') ' + (stepSize ? 'p+=h' : '++p')
					+ '; while (' + and('p>=b', '!(' + tail + ')') + ') ' + (stepSize ? 'p-=h' : '--p') + '; return p>=b');
		} else {
			addFunction(functionName, head + 'while (' + or('p<b', '!(' + tail + ')') + ') { if (' + or('p>=e', '!(' + repeatCode + ')') + ') return false; ' + (stepSize ? 'p+=h' : '++p') + '; }; return true');
		}
		return and(code, functionName + '(' + positionToCode(offset) + ')');
	}
	
	function captureWrap(code, capture, resetCaptureFrom, resetCaptureTo) {
		if (capture === null && resetCaptureFrom === resetCaptureTo) {
			return 'return ' + code;
		} else {
			var declares = '', captures = '', rollbacks = '';
			if (capture !== null) {
				declares += 'r' + capture + '=c' + capture;
				captures = 'c' + capture + '=p';
				rollbacks = 'c' + capture + '=r' + capture;
			}
			for (var i = resetCaptureFrom; i < resetCaptureTo; ++i) {
				var j = i * 2;
				if (capture !== null || i > resetCaptureFrom) {
					declares += ',';
					rollbacks += ',';
					if (i === resetCaptureFrom) {
						captures += ',';
					}
				}
				declares += 'r' + j + '=c' + j;
				captures += 'c' + j + '=';
				rollbacks += 'c' + j + '=r' + j;
				if (i === resetCaptureTo - 1) {
					captures += 'void 0';
				}
			}
			code = captures + ', ' + or(code, '(' + rollbacks + ',false)');
			return 'var ' + declares + '; return ' + code;
		}
	}

	function charClassToCode(ch, rule) {
		return (rule[1] ? '!' : '!!') + '(CC[' + ch + ']&' + (rule[0] | (rule[1] ? EMPTY_CHAR : 0)) + ')';
	}

	function compileTerms(offset, junction) {
		var literalSequence, quantity, code = 'true';
		termLoop: for (;;) {
			if (literalSequence = parseLiteralSequence()) {
				if (quantity = parseQuantifier()) {
					var repeatLiteral = literalSequence.pop();
					return quantify(and(code, literalSequenceToCode(literalSequence, offset))
							, offset + literalSequence.length, literalSequenceToCode(repeatLiteral, 0)
							, compileTerms(0, junction)
							, quantity);
				}
				code = and(code, literalSequenceToCode(literalSequence, offset));
				offset += literalSequence.length;
			} else { // TODO: make subs of these
				var c;
				switch (c = s[p]) {
					case '^': {
						++p;
						var condition = positionToCode(offset) + '===0';
						if (multiline) {
							condition = '(' + or(condition, '!!(CC[s[' + positionToCode(offset - 1) + ']]&' + NEWLINE_CHAR + ')') + ')';
						}
						code = and(code, condition)
						break;
					}
					case '$': {
						++p;
						var condition = positionToCode(offset) + '===l';
						if (multiline) {
							condition = '(' + or(condition, '!!(CC[s[' + positionToCode(offset) + ']]&' + NEWLINE_CHAR + ')') + ')';
						}
						code = and(code, condition);
						break;
					}
					case '[': {
						var negative = false, v0, v1, fromChar, toChar, classCode = 'false';
						if (s[++p] === '^') {
							negative = true;
							++p;
						}
						while (v0 = parseClassAtom()) {
							var b = p;
							if (s[p] === '-' && (++p, v1 = parseClassAtom())) {
								if (typeof v0 === 'string' && typeof v1 === 'string' && v0 <= v1) {
									if (ignoreCase && (v0 > '~' || v1 > '~' || ($regExpCanonicalize(v0) !== v0) !== ($regExpCanonicalize(v1) !== v1))) {
										v0 = escapeCharacter(v0);
										v1 = escapeCharacter(v1);
										classCode = or(classCode, and('$upperToLower[c]>="' + v0 + '"', '$upperToLower[c]<="' + v1 + '"'));
									} else {
										v0 = canonicalizeAndEscape(v0);
										v1 = canonicalizeAndEscape(v1);
									}
									classCode = or(classCode, and('c>="' + v0 + '"', 'c<="' + v1 + '"'));
								} else {
									throw SyntaxError("Invalid character class syntax in regular expression");
								}
							} else if (typeof v0 === 'string') {
								p = b;
								classCode = or(classCode, 'c==="' + canonicalizeAndEscape(v0) + '"');
							} else {
								classCode = or(classCode, charClassToCode('c', v0));
							}
						}
						if (s[p] !== ']') {
							throw SyntaxError("Invalid character class syntax in regular expression");
						}
						++p;
						var functionName = 'k' + (++functionCounter);
						addFunction(functionName, 'var c=s[p]; return ' + (negative ? '!(' + classCode + ')' : classCode));
						if (quantity = parseQuantifier()) {
							return quantify(code, offset, functionName + '(' + positionToCode(0) + ')', compileTerms(0, junction), quantity);
						}
						code = and(code, functionName + '(' + positionToCode(offset) + ')');
						++offset;
						break;
					}
					case '\\': {
						var n;
						++p;
						if ((n = parseNumber(-1)) >= 0) {
							if (n > maxBackReference) {
								maxBackReference = n;
							}
							n = (n - 1) * 2;
							var quantity = parseQuantifier();
							var stepSize = 'c' + (n + 1) + '-c' + n;
				// TODO: $submatch should take two additional optional params: start, end in match-string, thus eliminating need for substring here
							var backMatchCode = 'submatch(s,' + positionToCode(offset) + ',s.substring(c' + n + ',c' + (n + 1) + '))';
							var tail = compileTerms(0, junction);
							var tailName = 't' + (++functionCounter);
							addFunction(tailName, 'return ' + tail);
							if (quantity) {
								return quantify(code, offset, backMatchCode, tailName + '(' + positionToCode(0) + ')', quantity, stepSize);
							}
							return and(code, '(c' + n + '===void 0 ? ' + tailName + '(' + positionToCode(offset) + ') : '
									+ and(backMatchCode, tailName + '(' + positionToCode(offset) + '+' + stepSize + ')') + ')');
						} else if ((c = s[p]) === 'b' || c === 'B') {
							++p;
							code = and(code, (c === 'b' ? '!!((CC[s[' : '!((CC[s[') + positionToCode(offset - 1) + ']]^CC[s[' + positionToCode(offset) + ']])&' + WORD_CHAR + ')');
							break;
						}
						// fall-through
					}
					case '.': {
						var rule;
						if (!(rule = CHAR_CLASS_RULES[c])) {
							throw SyntaxError("Invalid escape in regular expression");
						}
						++p;
						if (quantity = parseQuantifier()) {
							return quantify(code, offset, charClassToCode('s[' + positionToCode(0) + ']', rule), compileTerms(0, junction), quantity);
						}
						code = and(code, charClassToCode('s[' + positionToCode(offset) + ']', rule));
						++offset;
						break;
					}
					case '(': {
						var functionNumber = ++functionCounter;
						var groupName = 'g' + functionNumber;
						var groupCall = groupName + '(' + positionToCode(offset) + ')';
						var junctionName = 'j' + functionNumber;
						var junctionCall = junctionName + '(' + positionToCode(offset) + ')';
						var doCapture = true, lookAhead = false, negativeLookAhead = false;
						++p;
						if (s[p] === '?') {
							switch (s[p + 1]) {
								case '!': negativeLookAhead = true;
								case '=': lookAhead = true;
								case ':': doCapture = false; p += 2;
							}
						}
						var openCapture = null, closeCapture = null;
						if (doCapture) {
							(closeCapture = (openCapture = (captureCounter++) * 2) + 1);
							closureVars += ',c' + openCapture + ',c' + closeCapture;
						}
						var innerCapturesStart = captureCounter; // FIX : these two are only used for negative lookAhead, use for open/close captures too?
						var disjunction = compileDisjunction(0, (lookAhead ? void 0 : junctionName));
						var innerCapturesEnd = captureCounter;
						if ((c = s[p]) !== ')') {
							throw SyntaxError(c ? "Unterminated group in regular expression" : "Invalid regular expression");
						}
						++p;
						var quantity = (lookAhead ? null : parseQuantifier());
						var tail = compileTerms(0, junction);
						var inner = disjunction;
						var extraEndlessCheck = '';
						var looping = (quantity && quantity.max > 1);
						if (looping && (quantity.max < Infinity || quantity.min > 1)) {
							closureVars += ',n' + functionNumber + '=0';
							if (quantity.max < Infinity) {
								inner = and('++n' + functionNumber + '<=' + quantity.max, inner);
							} else {
								inner = '++n' + functionNumber + ', ' + inner;
							}
							if (quantity.min > 1) {
								inner = and(inner, 'n' + functionNumber + '>=' + quantity.min, inner);
								extraEndlessCheck = 'n' + functionNumber + '<' + quantity.min;
							}
							inner = or(inner, '(--n' + functionNumber + ',false)');
						}
						if (lookAhead) {
							var call = junctionName + '(' + positionToCode(0) + ')';
							if (negativeLookAhead) {
								inner = '!(' + inner + ')';
								if (innerCapturesStart < innerCapturesEnd) {
									var captureResets = '';
									for (var i = innerCapturesStart; i < innerCapturesEnd; ++i) {
										captureResets += 'c' + i * 2 + '=';
									}
									call = '(' + captureResets + 'void 0, ' + call + ')';
								}
							}
							inner = and(inner, call);
						}
						if (looping) {
							closureVars += ',p' + functionNumber;
							var endlessCheck = 'p' + functionNumber + '!=p';
							inner = and(extraEndlessCheck ? '(' + or(endlessCheck, extraEndlessCheck) + ')' : endlessCheck, '(p' + functionNumber + '=p, ' + inner + ')');
						}
						addFunction(groupName, captureWrap(inner, openCapture, innerCapturesStart, innerCapturesEnd));
						if (looping) {
							var recursionCall = groupName + '(' + positionToCode(0) + ')';
							var inner = (quantity.greedy ? or(recursionCall, tail) : or(tail, recursionCall));
							addFunction(junctionName, captureWrap(inner, closeCapture));
							var call = (quantity.min === 0 ? junctionCall : groupCall);
							code = and(code, (looping ? '(p' + functionNumber + '=void 0,' + call + ')' : call));
						} else {
							addFunction(junctionName, captureWrap(tail, closeCapture));
							if (quantity && quantity.min === 0) {
								code = and(code, (quantity.max === 0 ? junctionCall
										: '(' + (quantity.greedy ? or(groupCall, junctionCall) : or(junctionCall, groupCall)) + ')'));
							} else {
								code = and(code, groupCall);
							}
						}
						return code;
					}
					default: break termLoop;
				}
			}
		}
		switch (junction) {
			case void 0: return code;
			case '': return and(code, '(q=' + positionToCode(offset) + ',true)');
			default: return and(code, junction + '(' + positionToCode(offset) + ')');
		}
	}

	function compileDisjunction(offset, junction) { // junction = function name, undefined (for none for lookahead) or '' (for end of pattern)
		var code = compileTerms(offset, junction);
		if (s[p] === '|') {
			do {
				++p;
				code = or(code, compileTerms(offset, junction));
			} while (s[p] === '|');
			code = '(' + code + ')';
		}
		return code;
	}

	var disjunction = compileDisjunction(0, ''); 
	if (p < s.length) {
		throw SyntaxError("Invalid regular expression");
	}
	if (maxBackReference > captureCounter) {
		throw SyntaxError("Invalid back reference in regular expression");
	}
	var code = "(function(s, p) {\n\"use strict\";\n"
	if (ignoreCase) {
		code += "\ts=$regExpCanonicalize(s)\n";
	}
	code += "\tvar l=s.length,q"; // FIX : not "use strict"
	code += closureVars + ";\n" + functions + "\tif (" + disjunction + ") return [p,q";
	for (var i = 0; i < captureCounter * 2; ++i) code += ",c" + i;
	code += "];\n})";
	return code;
}

function submatch(s, pos, str) { return (s.substr(pos, str.length) === str); }

function $isNaN(v) { return isNaN(v); }
function $isFinite(v) { return isFinite(v); }
function $floor(v) { return Math.floor(v); }

function $isPrimitive(v) {
	var v, t;
	return (((t = typeof v) !== "object" || v === null) && t !== "function");
}

function $toInteger(v) {
	return ($isNaN(v = +v) ? 0.0 : (!$isFinite(v) ? v : (v < 0 ? -$floor(-v) : $floor(v))));
}

function $toString(o) {
	return '' + ($isPrimitive(o) ? o : $support.toPrimitiveString(o));
}

var REG_EXP_FLAG_TO_PROPERTY = { 'g': 'global', 'i': 'ignoreCase', 'm': 'multiline' };

function MyRegExp(pattern, flags) {
	pattern = (pattern === void 0 ? '' : $toString(pattern));
	flags = (flags === void 0 ? '' : $toString(flags));
	this.global = this.ignoreCase = this.multiline = false;
	for (var i = flags.length - 1; i >= 0; --i) {
		var p;
		if (!(p = REG_EXP_FLAG_TO_PROPERTY[flags[i]]) || this[p]) {
			throw SyntaxError("Invalid regular expression flags");
		}
		this[p] = true; // FIX : { DontDelete, ReadOnly, DontEnum }.
	}
	this.source = pattern; // FIX : { DontDelete, ReadOnly, DontEnum }.
	this.match = eval(compileRegExp(pattern, this.ignoreCase, this.multiline));
	this.lastIndex = 0; // FIX : { DontDelete, DontEnum }.
}

function $execRegExp(re, string) {
	var i, m, l;
	string = $toString(string);
	if ((i = (re.global ? $toInteger(re.lastIndex) : 0)) >= 0) {
		for (l = string.length; i <= l; ++i) {
			if (m = re.match(string, i)) {
				if (re.global) {
					re.lastIndex = m[1];
				}
				return m;
			}
		}
	}
	re.lastIndex = 0;
}

MyRegExp.prototype.exec = function(string) {
	var m, a = null;
	if (m = $execRegExp(this, string)) {
		(a = [ ]).input = string;
		a.index = m[0];
		for (var j = 0; j < m.length; j += 2) {
			a[a.length] = ((m[j] === void 0) ? void 0 : string.substring(m[j], m[j + 1]));
		}
	}
	return a;
};

MyRegExp.prototype.toString = function() {
	return '/' + this.source + '/' + (this.global ? 'g' : '') + (this.ignoreCase ? 'i' : '') + (this.multiline ? 'm' : '');
};

MyRegExp.prototype.test = function(string) { return $execRegExp(this, string) !== void 0; };

/*
print(compileRegExp("\\n\\rab\\nc\\x55\\u1234\\cA\\0\""));
print(compileRegExp("a"));
print(compileRegExp("ade...bfg*"));
print(compileRegExp("a\\n"));
print(compileRegExp("a|bdef.gh|iop.*"));
print(compileRegExp("qwerty(abc|defgh|x)yzzy"));

print(compileRegExp("qwerty(a(bc|pa..pa)poo*|defgh|x)yzzy(a|bc|def)*end$"));
print(compileRegExp("start((abc|de)*end)*end"));
print(compileRegExp("start(?:(?:abc|de)*end)*end"));
print(compileRegExp("start(?:x)+end"));
print(compileRegExp("((?:abc)+?)((?:abc)+)end"));
print(compileRegExp("((?:abc)+)((?:abc)+?)end"));
print(compileRegExp("(?:a)*e"));
print(compileRegExp("a*e"));
print(compileRegExp("s(a)?$"));
print(compileRegExp("s(a)??$"));
print(compileRegExp("s(?:a|)$"));
print(compileRegExp("(a*)*"));
print(compileRegExp("(z)(((?:a)+)?((?:b)+)?(c))*"));
print(compileRegExp("a(b)?c"));
print(compileRegExp("a(b)*c"));
print(compileRegExp("a(b(c)*)*d"));
print(compileRegExp("(abc(def)|(ghi)jkl)*"));

print(compileRegExp("b(a|b*|c)e"));
print(compileRegExp(".*"));
print(compileRegExp("s(a)??(a)?$"));
*/

function myescape(s) {
	var o = '', b = 0, c;
	for (var i = 0; i < s.length; ++i) {
		if ((c = s[i]) < ' ' || c > '\x7E') {
			o += s.substring(b, i) + ("\\u" + ("0000" + c.charCodeAt(0).toString(16)).slice(-4));
			b = i + 1;
		}
	}
	return o + (b > 0 ? s.substring(b, i) : s);
}

function testRE(re, s) {
	var m = re.exec(s);
	var r = '';
	if (!m) {
		r = 'null';
	} else {
		r = '{ ';
		for (var i = 0; i < m.length; ++i) {
			r += (i > 0 ? ', ' : '') + myescape('' + m[i]);
		}
		r += ' } @ ' + m.index + (m.global ? ' (next: ' + re.lastIndex + ')' : '');
	}
	print(myescape(re.toString()) + " on " + myescape(s) + ": " + r + "\n");
}

// MyRegExp = RegExp;

var t = new MyRegExp("abcdef", "g");
var m;
testRE(t, "abcdef");
testRE(t, "abc");
testRE(t, "abcdeabcdefabcde");

var t = new MyRegExp("\\n\\rab\\nc\\x15\\u1234\u0055\\cA\\0\"");
testRE(t, "\n\rab\nc\x15\u1234\u0055\x01\0\"");
testRE(t, "\\n\\rab\\nc\\x15\\u1234\\u0055\\x01\\0\\\"");

var t = new MyRegExp("\\\\");
testRE(t, "\\");

var t = new MyRegExp("\\(");
testRE(t, "(");

var t = new MyRegExp("^abcdef");
testRE(t, "abcdef");
testRE(t, "abcdeabcdefabcde");

var t = new MyRegExp(".*$");
testRE(t, "abc\ndef");
testRE(t, "abc\u2029def");
testRE(t, "abc");

var t = new MyRegExp("^def");
testRE(t, "abc\ndef");

var t = new MyRegExp("^def", "m");
testRE(t, "abc\ndef");

var t = new MyRegExp("abc$", "");
testRE(t, "abc\ndef");

var t = new MyRegExp("abc$", "m");
testRE(t, "abc\ndef");

var t = new MyRegExp("^(a|bc|def)$");
testRE(t, "");
testRE(t, "a");
testRE(t, "bc");
testRE(t, "def");
testRE(t, "bcdef");

var t = new MyRegExp("^(|a|bc|def)$");
testRE(t, "");
testRE(t, "def");
testRE(t, "bcdef");

var t = new MyRegExp("\\w\\w\\w");
testRE(t, "");
testRE(t, "123");
testRE(t, "12a");
testRE(t, "12#");
var t = new MyRegExp("\\W\\W\\W");
testRE(t, "12#");
testRE(t, "#%)");
var t = new MyRegExp("\\w\\w\\w\\W\\s\\s\\s\\S\\S\\S\\d\\d\\d\\d\\D\\D\\D\\D");
testRE(t, "q9_!\n \u2028lm_9124qwer");

var t = new MyRegExp("(z)(((?:a)+)?((?:b)+)?(c))*");
testRE(t, "");
testRE(t, "zaacbbbcac");
t = new MyRegExp("s(a)??(a)?$");
testRE(t, "");
testRE(t, "s");
testRE(t, "sa");
testRE(t, "saa");
testRE(t, "saaa");

t = new MyRegExp("^(a){0}$");
testRE(t, "");
testRE(t, "a");
testRE(t, "aa");

t = new MyRegExp("^(a){1}$");
testRE(t, "");
testRE(t, "a");
testRE(t, "aa");

t = new MyRegExp("(a){3}");
testRE(t, "a");
testRE(t, "aa");
testRE(t, "aaa");
testRE(t, "aaaa");

t = new MyRegExp("(a){3,}");
testRE(t, "aa");
testRE(t, "aaa");
testRE(t, "aaaa");

t = new MyRegExp("(a){13,23}$");
testRE(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

t = new MyRegExp("(a){1,1}?$");
testRE(t, "a");
testRE(t, "aa");

t = new MyRegExp("^(a|bc|def){3,5}$");
testRE(t, "abcdef");
testRE(t, "defbca");
testRE(t, "aaaaa");
testRE(t, "abcdefabcdef");

t = new MyRegExp("(abc)\\1");
testRE(t, "abc");
testRE(t, "abcabc");
testRE(t, "qwertyabcabc");

t = new MyRegExp("^qwerty(abc|def)\\1$");
testRE(t, "qwertyabcabc");
testRE(t, "qwertydefdef");
testRE(t, "qwertyabcdef");
testRE(t, "qwertydefabc");

t = new MyRegExp("^qwerty(abc|defg)\\1?$");
testRE(t, "qwertyabc");
testRE(t, "qwertydefgdefg");
testRE(t, "qwertyabcabcabc");

t = new MyRegExp("^qwerty(abc|defg)\\1*$");
testRE(t, "qwertyabc");
testRE(t, "qwertydefgdefg");
testRE(t, "qwertyabcabcabc");

t = new MyRegExp("^qwerty(abc|defg)\\1+$");
testRE(t, "qwertyabc");
testRE(t, "qwertydefgdefg");
testRE(t, "qwertyabcabcabc");

t = new MyRegExp("^qwerty(abc|defg)\\1+?$");
testRE(t, "qwertyabc");
testRE(t, "qwertydefgdefg");
testRE(t, "qwertyabcabcabcabcabc");

t = new MyRegExp("^qwerty(abc|defg|)\\1*abc$");
testRE(t, "qwertydefg");
testRE(t, "qwertydefgdefg");
testRE(t, "qwertyabc");

t = new MyRegExp("^(ab|def|ghij)?\\1{0}$");
testRE(t, "");
testRE(t, "ghij");
testRE(t, "ababababab");
testRE(t, "abababababab");

t = new MyRegExp("^(ab|def|ghij)?\\1{1}$");
testRE(t, "");
testRE(t, "def");
testRE(t, "defdef");
testRE(t, "ababababab");
testRE(t, "abababababab");

t = new MyRegExp("^(ab|def|ghij)?\\1{5}$");
testRE(t, "");
testRE(t, "ababababab");
testRE(t, "abababababab");

t = new MyRegExp("^(ab|def|ghij)?(\\1){5}$");
testRE(t, "");
testRE(t, "ababababab");
testRE(t, "abababababab");
testRE(t, "defdefdefdefdefdef");
testRE(t, "ghijghijghijghijghijghij");


t = new MyRegExp("^(ab)(abc)(\\1|\\2){5}abc$");
testRE(t, "abcd");
testRE(t, "ababcabababcabcabcabc");

t = new MyRegExp("^(ab|ab..)?(\\1){5}$");
testRE(t, "");
testRE(t, "abababababab");
testRE(t, "abababababababababababab");
testRE(t, "abcdabcdabcdabcdabcdabcd");

t = new MyRegExp("^(a|ab)(bc|c)(\\1\\2){5}$");
testRE(t, "abcabcabcabcabcabc");

t = new MyRegExp("((?:a)*)(\\1){3}$");
testRE(t, "");
testRE(t, "xxxxa");
testRE(t, "xxxxaaaa");
testRE(t, "xxxxaaaaaa");
testRE(t, "xxxxaaaaaaaa");

t = new MyRegExp("((?:a)*)(\\1){3,}$");
testRE(t, "");
testRE(t, "xxxxaaaa");
testRE(t, "xxxxaaaaaaaaaaaaa");

t = new MyRegExp("(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k)(l)\\1\\2\\3\\4\\5\\6\\7\\8\\9\\10\\11\\12");
testRE(t, "abcdefghijklabcdefghijkl");

t = new MyRegExp("^(?=abc)abcdef$");
testRE(t, 'def');
testRE(t, 'abcabcdef');
testRE(t, 'abcdef');

t = new MyRegExp("^(?=abc|ab)abcdef$");
testRE(t, 'abcdef');
testRE(t, 'abcdefabc');
testRE(t, 'abcdefab');

t = new MyRegExp("^(?=nope|ab)abcdef$");
testRE(t, 'abcdef');
testRE(t, 'abcdefabc');
testRE(t, 'abcdefab');

t = new MyRegExp("^(?=(abc|ab))abcdef\\1$");
testRE(t, 'abcdefabc');
testRE(t, 'abcdefab');
t = new MyRegExp("^(?=(ab|abc))abcdef\\1$");
testRE(t, 'abcdefabc');
testRE(t, 'abcdefab');

t = new MyRegExp("(?=((?:a)+))(?:a)*b\\1");
testRE(t, "baaabac");

t = new MyRegExp("^(?!abc)(.)+$");
testRE(t, "abc");
testRE(t, "ab");
testRE(t, "abcd");

t = new MyRegExp("^(?!(abc))(.)+$");
testRE(t, "abab");

t = new MyRegExp("^(?:a)+aa$");
testRE(t, "aaaaabc");

t = new MyRegExp("^a*$");
testRE(t, "");
testRE(t, "a");
testRE(t, "aaaaa");

t = new MyRegExp("^aaa*aa$");
testRE(t, "aaaaaaaaa");

t = new MyRegExp("^(aa)(a*)(aa)$");
testRE(t, "");
testRE(t, "aa");
testRE(t, "aaaa");
testRE(t, "aaaaaaaaa");

t = new MyRegExp("^(.*)\\1$");
testRE(t, "");
testRE(t, "aa");
testRE(t, "aaaa");
testRE(t, "qweqwe");
testRE(t, "qweqweqwe");
testRE(t, "qwertyqwertyqwertyqwerty");

t = new MyRegExp("^start(x+)end$");
testRE(t, "startend");
testRE(t, "startxend");
testRE(t, "startxxxxxend");
testRE(t, "startxxxxxen");

t = new MyRegExp("^start(x?)end$");
testRE(t, "startend");
testRE(t, "startxend");
testRE(t, "startxxxxxend");
testRE(t, "startxxxxxen");

t = new MyRegExp("^start(x{3,5})end$");
testRE(t, "startend");
testRE(t, "startxend");
testRE(t, "startxxend");
testRE(t, "startxxxend");
testRE(t, "startxxxxxend");
testRE(t, "startxxxxxxend");
testRE(t, "startxxxxxen");

t = new MyRegExp("^startx{3,5}?end$");
testRE(t, "startend");
testRE(t, "startxend");
testRE(t, "startxxend");
testRE(t, "startxxxend");
testRE(t, "startxxxxxend");
testRE(t, "startxxxxxxend");
testRE(t, "startxxxxxen");

t = new MyRegExp("^start(x{3,5})(x*)end$");
testRE(t, "startxxxxxxxxxxend");

t = new MyRegExp("^start(x{3,5}?)(x*)end$");
testRE(t, "startxxxxxxxxxxend");

t = new MyRegExp("(?=(a+))a*b\\1");
testRE(t, "baaabac");

t = new MyRegExp("(a+)*q");
testRE(t, "aaq");

t = new MyRegExp("^(?:(a)b|(ab))(?:\\2)+$");
testRE(t, "ababab");

t = new MyRegExp("(a*)b(\\1)+");
testRE(t, "baaaac");

t = new MyRegExp("^[]$"); testRE(t, ""); testRE(t, "x"); testRE(t, "xy");
t = new MyRegExp("^[^]$"); testRE(t, ""); testRE(t, "x"); testRE(t, "xy");
t = new MyRegExp("^[a]$"); testRE(t, "x");
t = new MyRegExp("^[x]$"); testRE(t, "x");
t = new MyRegExp("^[\\\\]$"); testRE(t, "\\");
t = new MyRegExp("^[\\]]$"); testRE(t, "]");
t = new MyRegExp("^[a-z]$"); testRE(t, "x"); testRE(t, "X");
t = new MyRegExp("^[^a-z]$"); testRE(t, "x"); testRE(t, "X");
t = new MyRegExp("^[a-chioggf-z]$"); testRE(t, "x"); testRE(t, "X");
t = new MyRegExp("^[\\[-\\]]$"); testRE(t, "\\"); testRE(t, "["); testRE(t, "]"); testRE(t, "Z"); testRE(t, "^");
t = new MyRegExp("^[\\n]$"); testRE(t, "\\n"); testRE(t, "X");
t = new MyRegExp("^[\\cA]$"); testRE(t, "\u0001"); testRE(t, "X");
t = new MyRegExp("^[\u1234]$"); testRE(t, "\u1234"); testRE(t, "X");
t = new MyRegExp("^[\0-\uFFFF]$"); testRE(t, "\u1234"); testRE(t, "X");
t = new MyRegExp("^[\\b]$"); testRE(t, "\b"); testRE(t, "X");
t = new MyRegExp("^[\\w]$"); testRE(t, "z"); testRE(t, "X"); testRE(t, "1"); testRE(t, "!"); testRE(t, "\n"); testRE(t, " ");
t = new MyRegExp("^[\\W]$"); testRE(t, "z"); testRE(t, "X"); testRE(t, "1"); testRE(t, "!"); testRE(t, "\n"); testRE(t, " ");
t = new MyRegExp("^[\\s]$"); testRE(t, "z"); testRE(t, "X"); testRE(t, "1"); testRE(t, "!"); testRE(t, "\n"); testRE(t, " ");
t = new MyRegExp("^[\\S]$"); testRE(t, "z"); testRE(t, "X"); testRE(t, "1"); testRE(t, "!"); testRE(t, "\n"); testRE(t, " ");
t = new MyRegExp("^[\\d]$"); testRE(t, "z"); testRE(t, "X"); testRE(t, "1"); testRE(t, "!"); testRE(t, "\n"); testRE(t, " ");
t = new MyRegExp("^[\\D]$"); testRE(t, "z"); testRE(t, "X"); testRE(t, "1"); testRE(t, "!"); testRE(t, "\n"); testRE(t, " ");

t = new MyRegExp("^.\\b..$"); testRE(t, "zxc"); testRE(t, " xc"); testRE(t, "xc "); testRE(t, "x  "); testRE(t, "   "); testRE(t, "  x");
t = new MyRegExp("^.\\B..$"); testRE(t, "zxc"); testRE(t, " xc"); testRE(t, "xc "); testRE(t, "x  "); testRE(t, "   "); testRE(t, "  x");
t = new MyRegExp("^\\B$"); testRE(t, ""); testRE(t, "x");
t = new MyRegExp("^(\\b)*$"); testRE(t, ""); testRE(t, "xc");

/*
	Here, I'm interpreting the ES specs differently from the browser engines that I've tried, but similar to (at least
	older versions of) PCRE. The first capture will be empty because the empty string is the last successful match to
	(a*), after which we are not progressing on the input so we break out of the two '*' loops. I believe my
	interpretation is correct, but I am not 100%. See 15.10.2.5 in the ES spec.
*/
t = new MyRegExp("(a*)*q");
testRE(t, "aaq");
t = new MyRegExp("((?:a)*)*q");
testRE(t, "aaq");
t = new MyRegExp("(a*|b*)*q");
testRE(t, "aaq");
testRE(t, "bbq");
t = new MyRegExp("(a*)+q");
testRE(t, "aaq");
t = new MyRegExp("^(\\B)*$");
testRE(t, "");
testRE(t, "xc");

t = new MyRegExp("abcdABCD", "i");
testRE(t, "AbcDabCd");

t = new MyRegExp("\\x66\\x46\\n", "i");
testRE(t, "Ff\n");

t = new MyRegExp("[a\x66]", "i");
testRE(t, "AF\n");
testRE(t, "af\n");

t = new MyRegExp("[a-z]", "i");
testRE(t, "g");
testRE(t, "\u0131");
testRE(t, "\u017F");

t = new MyRegExp("[A-Z]", "i");
testRE(t, "g");
testRE(t, "\u0131");
testRE(t, "\u017F");

t = new MyRegExp("^(?:([E-F])|.)(?:([E-F])|.)(?:([E-F])|.)(?:([E-F])|.)(?:([E-F])|.)(?:([E-F])|.)(?:([E-F])|.)(?:([E-F])|.)(?:([E-F])|.)$", "i");
testRE(t, "DEFGdefg_");
t = new MyRegExp("^(?:([E-f])|.)(?:([E-f])|.)(?:([E-f])|.)(?:([E-f])|.)(?:([E-f])|.)(?:([E-f])|.)(?:([E-f])|.)(?:([E-f])|.)(?:([E-f])|.)$", "i");
testRE(t, "DEFGdefg_");
t = new MyRegExp("^(?:([e-f])|.)(?:([e-f])|.)(?:([e-f])|.)(?:([e-f])|.)(?:([e-f])|.)(?:([e-f])|.)(?:([e-f])|.)(?:([e-f])|.)(?:([e-f])|.)$", "i");
testRE(t, "DEFGdefg_");
t = new MyRegExp("^(?:([G-d])|.)(?:([G-d])|.)(?:([G-d])|.)(?:([G-d])|.)(?:([G-d])|.)(?:([G-d])|.)(?:([G-d])|.)(?:([G-d])|.)(?:([G-d])|.)$", "i");
testRE(t, "DEFGdefg_");
t = new MyRegExp("a|ab");
testRE(t, "abc");
t = new MyRegExp("a[a-z]{2,4}");
testRE(t, "abcdefghi");

t = new MyRegExp("(a*)b\\1+");
testRE(t, "baaaac")

t = new MyRegExp("(.*?)a(?!(a+)b\\2c)\\2(.*)");
testRE(t, "baaabaac");

t = new MyRegExp("(?:(a)|(a))(?!(?:\\2)+)\\2\\2\\2$");
testRE(t, "aaaa");

t = new MyRegExp("(\\1(a)){2}$");
testRE(t, "aa");

t = new MyRegExp("(z)((a+)?(b+)?(c))*");
testRE(t, "zaacbbbcac");

t = new MyRegExp("(z)(?:(a+)?(b+)?(c))*");
testRE(t, "zaacbbbcac");

try { t = new MyRegExp("\\"); } catch (x) { print(x); }
try { t = new MyRegExp("\\q"); } catch (x) { print(x); }
try { t = new MyRegExp("\\1"); } catch (x) { print(x); }
try { t = new MyRegExp("\\2(a)\\1"); } catch (x) { print(x); }
try { t = new MyRegExp("^(?=ab)*abcdef$"); } catch (x) { print(x); }
try { t = new MyRegExp("(?%)"); } catch (x) { print(x); }
try { t = new MyRegExp("("); } catch (x) { print(x); }
try { t = new MyRegExp("x{5,3}"); } catch (x) { print(x); }

try { t = new MyRegExp("["); } catch (x) { print(x); }
try { t = new MyRegExp("[^"); } catch (x) { print(x); }
try { t = new MyRegExp("[\\p]"); } catch (x) { print(x); }
try { t = new MyRegExp("[\\3]"); } catch (x) { print(x); }
try { t = new MyRegExp("[\\01]"); } catch (x) { print(x); }
try { t = new MyRegExp("[\\w-1]"); } catch (x) { print(x); }
try { t = new MyRegExp("[1-\\w]"); } catch (x) { print(x); }
try { t = new MyRegExp("[\\S-\\w]"); } catch (x) { print(x); }
try { t = new MyRegExp("[\\B]"); } catch (x) { print(x); }
try { t = new MyRegExp("[b-a]"); } catch (x) { print(x); }
