> print(eval("try { 23 } catch (x) { 57 }"))
< 23
-
> print(eval("try { 23; throw void 0 } catch (x) { 57 }"))
< 57
-
> print(eval("try { 23 } finally { 57 }"))
< 23
-
> print(eval("try { 23; throw void 0 } catch (x) { 57 } finally { 97 }"))
< 57
-
> print(eval("dis: { try { 23; break dis } finally { 97 } }"))
< 23
-
> print(eval("dis: { try { 23; throw void 0 } catch (x) { 57; break dis } finally { 97 } }"))
< 57
-
> print(eval("dis: { try { 23; throw void 0 } catch (x) { 57; break dis } finally { 97; break dis } }"))
< 97
-
