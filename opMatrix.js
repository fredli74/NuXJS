(function(){
	print("*** operation matrix ***");

	var valueStrings = [
		'undefined', 'null', 'false', 'true', '0', '1', '-128', '1234.5678', '-1.0e+7', 'NaN', 'Infinity', '""', '"str"'
		, '"-128"', '"1.0"', '"  +1234  "', '"string"', '"5tring"'
		, '({valueOf:function(){return "-128"}})','({valueOf:function(){return null}})'
	];

	var values = [ ];
	for (var i = 0; i < valueStrings.length; ++i) {
		values[i] = eval(valueStrings[i]);
	}

	var tests = [
		"==", "!=", "===", "!==", "<", "<=", ">", ">=", "+", "-", "*", "/", "%", "&", "^", "|", "<<", ">>", ">>>"
	];

	for (var t = 0; t < tests.length; ++t) {
		var test = tests[t];
		print(test);
		for (var i = 0; i < values.length; ++i) {
			var a = values[i];
			var results = "";
			for (var j = 0; j < values.length; ++j) {
				var b = values[j];
				var r;
				try {
					r = eval('a' + test + 'b');
					if (typeof r === 'number') {
						r = Math.floor(r * 100000 + 0.5) / 100000;
					}
				}
				catch (e) {
					r = e;
				}
				if (results !== "") {
					results += " ";
				}
				results += r;
			}
			results = valueStrings[i] + ": " + results;
			print(results);
		}
		print("");
	}
})();