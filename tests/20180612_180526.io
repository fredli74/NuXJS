> print("apan bapan banan".replace("bapan",function() { arguments.join = Array.prototype.join; print(arguments.join()); return "hola" }))
< bapan,5,apan bapan banan
< apan hola banan
-
> print("apan bapan banan".replace("banan",function() { arguments.join = Array.prototype.join; print(arguments.join()); return "hola" }))
< banan,11,apan bapan banan
< apan bapan hola
-
> print("apan bapan banan".replace("xx",function() { arguments.join = Array.prototype.join; print(arguments.join()); return "hola" }))
< apan bapan banan
-
> print("apan bapan banan".replace("",function() { arguments.join = Array.prototype.join; print(arguments.join()); return "hola" }))
< ,0,apan bapan banan
< holaapan bapan banan
-
> print("apan bapan banan".replace("apan bapan banan",function() { arguments.join = Array.prototype.join; print(arguments.join()); return "hola" }))
< apan bapan banan,0,apan bapan banan
< hola
-
> print("apan bapan banan".replace("apan bapan banana",function() { arguments.join = Array.prototype.join; print(arguments.join()); return "hola" }))
< apan bapan banan
-
> print("apan bapan banan".replace(/ba(.)(.)n/,function() { arguments.join = Array.prototype.join; print(arguments.join()); return "hola" }))
< bapan,p,a,5,apan bapan banan
< apan hola banan
-
> print("apan bapan banan".replace(RegExp(""),function() { arguments.join = Array.prototype.join; print(arguments.join()); return "hola" }))
< ,0,apan bapan banan
< holaapan bapan banan
-
> print("apan bapan banan".replace(RegExp("z"),function() { arguments.join = Array.prototype.join; print(arguments.join()); return "hola" }))
< apan bapan banan
-
