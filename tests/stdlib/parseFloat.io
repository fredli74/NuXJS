> print(parseFloat(''))
< NaN
-
> print(parseFloat('    '))
< NaN
-
> print(parseFloat('    1234'))
< 1234
-
> print(parseFloat('    - 1234'))
< NaN
-
> print(parseFloat('    -1234.1234'))
< -1234.1234
-
> print(parseFloat('    -1234.1234e+3'))
< -1234123.4
-
> print(parseFloat('    -1234.1234e+3kokobello'))
< -1234123.4
-
> print(parseFloat({ toString: function() { return "  \n  \t 12345.66789" } }))
< 12345.66789
-
> print(parseFloat('+Infinity'))
< Infinity
-
> print(parseFloat('-Infinity'))
< -Infinity
-
> print(parseFloat('+Infinitymessimal'))
< Infinity
-
> print(parseFloat('-0'))
< 0
-
