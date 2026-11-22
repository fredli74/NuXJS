// An if-block over 2^23 code words (x++ is 7) is a RangeError; unchecked, release wrapped the jump and crashed.
> var body = "x++;x++;x++;x++;x++;"; for (var i = 0; i < 18; ++i) body += body;
> try { new Function("if (x) {" + body + "}"); } catch (e) { print(e.name + ": " + e.message); }
< RangeError: Internal compiler limitations reached. Reduce code complexity.
-
// A case test and its body each under 2^23 words, their jump back over 2^23: only the switch fix-up catches it.
> var s = "x++;x++;x++;x++;x++;", body = s, test = s.replace(/;/g, ",");
> for (var i = 0; i < 17; ++i) { body += body; test += test; }
> var source = "switch (x) { case (" + test + "1): " + body + " }";
> try { new Function(source); } catch (e) { print(e.name + ": " + e.message); }
< RangeError: Internal compiler limitations reached. Reduce code complexity.
