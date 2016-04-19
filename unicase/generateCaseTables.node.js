/***

	type SpecialCasing-2.1.8.txt UnicodeData-2.1.8.txt | node generateCaseTables.node.js > tables.js

***/

const readline = require('readline');

const rl = readline.createInterface({
  input: process.stdin
});

var lowerToUpper = {};
var upperToLower = {};
var names = {};
var skipped = 0;

function exportTable(name, table) {
	var l = [];
	for (var c in table) 
		if (c.length === 4) l.push(c);
		else skipped++;

	l.sort();
	console.log("var " + name + " = {");
	for (var i in l) {
		var f = decode("\\u" + l[i].toLowerCase());
		var t = decode("\\u" + table[l[i]].toLowerCase());
		console.log("\t\"" + f + "\":\"" + t + "\"," + 
			"                         ".substr(f.length + t.length) + " // " + names[l[i]]);
	}
	console.log("};\n");
};

rl.on('line', (line) => {
	// code;name;<ignore*10>;uppercase;lowercase;<ignore>
	var m = line.match(/^([0-9a-z]{4,6});([^;]*);(?:[^;]*;){10}([0-9a-z]{4,6})?;([0-9a-z]{4,6})?;[^;]*$/i);
	if (m && m.length === 5) {
		if (names[m[1]] === undefined) names[m[1]] = m[2];
		if (m[3] && lowerToUpper[m[1]] === undefined) lowerToUpper[m[1]] = m[3];
		if (m[4] && upperToLower[m[1]] === undefined) upperToLower[m[1]] = m[4];
	} else if (line.match(/^(#.*|\S*)$/)) {
		// comment
	} else {
		m = line.match(/^([0-9A-Z]{4});\s*([ 0-9A-Z]+);[ 0-9A-Z ]+;\s*([ 0-9A-Z ]+); # (.*)$/i);
		if (m && m.length === 5) {
			names[m[1]] = m[4] + " (SpecialCase)";
			if (m[1] !== m[2]) upperToLower[m[1]] = m[2].replace(/ /g,"\\u");
			if (m[1] !== m[3]) lowerToUpper[m[1]] = m[3].replace(/ /g,"\\u");
		} else console.warn("Skipping line: " + line);
	}
}).on('close', () => {
	var a, bidirectional = {};
	for (a in upperToLower) {
		if (upperToLower[a] && lowerToUpper[upperToLower[a]] === a) {
			bidirectional[a] = upperToLower[a];
			delete lowerToUpper[upperToLower[a]];
			delete upperToLower[a];
		}
	}
	for (a in lowerToUpper) {
		if (lowerToUpper[a] && upperToLower[lowerToUpper[a]] === a) {
			bidirectional[upperToLower[a]] = a;
			delete upperToLower[lowerToUpper[a]];
			delete lowerToUpper[a];
		}
	}
	exportTable("lowerToUpper", lowerToUpper);
	exportTable("upperToLower", upperToLower);
	exportTable("bidirectional", bidirectional);
	if (skipped) console.warn("Skipped " + skipped + " characters (outside range \\u0000 - \\uffff)");
	process.exit(0);
});

// output optimization of printable ASCII characters
function decode(s) {
	return s
		.replace(/\\u0041/gi, "A")
		.replace(/\\u0042/gi, 'B')
		.replace(/\\u0043/gi, "C")
		.replace(/\\u0044/gi, "D")
		.replace(/\\u0045/gi, "E")
		.replace(/\\u0046/gi, "F")
		.replace(/\\u0047/gi, "G")
		.replace(/\\u0048/gi, "H")
		.replace(/\\u0049/gi, "I")
		.replace(/\\u004A/gi, "J")
		.replace(/\\u004B/gi, "K")
		.replace(/\\u004C/gi, "L")
		.replace(/\\u004D/gi, "M")
		.replace(/\\u004E/gi, "N")
		.replace(/\\u004F/gi, "O")
		.replace(/\\u0050/gi, "P")
		.replace(/\\u0051/gi, "Q")
		.replace(/\\u0052/gi, "R")
		.replace(/\\u0053/gi, "S")
		.replace(/\\u0054/gi, "T")
		.replace(/\\u0055/gi, "U")
		.replace(/\\u0056/gi, "V")
		.replace(/\\u0057/gi, "W")
		.replace(/\\u0058/gi, "X")
		.replace(/\\u0059/gi, "Y")
		.replace(/\\u005A/gi, "Z")
		.replace(/\\u0061/gi, "a")
		.replace(/\\u0062/gi, 'b')
		.replace(/\\u0063/gi, "c")
		.replace(/\\u0064/gi, "d")
		.replace(/\\u0065/gi, "e")
		.replace(/\\u0066/gi, "f")
		.replace(/\\u0067/gi, "g")
		.replace(/\\u0068/gi, "h")
		.replace(/\\u0069/gi, "i")
		.replace(/\\u006A/gi, "j")
		.replace(/\\u006B/gi, "k")
		.replace(/\\u006C/gi, "l")
		.replace(/\\u006D/gi, "m")
		.replace(/\\u006E/gi, "n")
		.replace(/\\u006F/gi, "o")
		.replace(/\\u0070/gi, "p")
		.replace(/\\u0071/gi, "q")
		.replace(/\\u0072/gi, "r")
		.replace(/\\u0073/gi, "s")
		.replace(/\\u0074/gi, "t")
		.replace(/\\u0075/gi, "u")
		.replace(/\\u0076/gi, "v")
		.replace(/\\u0077/gi, "w")
		.replace(/\\u0078/gi, "x")
		.replace(/\\u0079/gi, "y")
		.replace(/\\u007A/gi, "z");
}