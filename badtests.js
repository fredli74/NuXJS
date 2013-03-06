( // missing expression

) // just wrong

åäö = 4 // unaccepted identifier

3 = 3 // illegal l-value

/* unterminated

" unterminated

' unterminated

"\

"\x5k" // should be illegal, but isn't right now

"\u53Ak" // should be illegal, but isn't right now

" \q illegal escape "

f(

f(,

f(3,

f(3,5 3)

a.

a.+

case = 3

super = 5

(function( {})

(function(,) {})

(function(a,) {})

(function(a,b c) {})

(function(a,b,c))

a = 0x

a = .

var =5

var x=5,

x: { x: { } }

x: { break y }

x: { continue y }

x: { continue x }

{ break }

{ continue }

try

try {

try { }

try { } catch ( ) { }

switch () { }

switch (3) { x=1 }

(true ? a : b) = 3

for (var x = 3, y = 5 in f; x < 100 && y < 15 ; ++y) { s += y + "," };

for ( a = 100 in "abcdefgh" ) ;

