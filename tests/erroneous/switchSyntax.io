> switch()
! !!!! Line: 1
! !!!! SyntaxError: Missing / invalid expression
-
> switch (1)
! !!!! Line: 1
! !!!! SyntaxError: Expected '{'
-
> switch (1) {
! !!!! Line: 1
! !!!! SyntaxError: Expected '}'
-
> switch (1) { }
-
> switch (1) { break }
! !!!! Line: 1
! !!!! SyntaxError: Expected 'case', 'default' or '}'
-
> switch (1) { case }
! !!!! Line: 1
! !!!! SyntaxError: Missing / invalid expression
-
> switch (1) { case: }
! !!!! Line: 1
! !!!! SyntaxError: Missing / invalid expression
-
> switch (1) { case 1: }
-
> switch (1) { case 1: default: }
-
> switch (1) { case 1: break 1; }
! !!!! Line: 1
! !!!! SyntaxError: Syntax error
-
> switch (1) { case1: }
-
> switch (1) case 1
! !!!! Line: 1
! !!!! SyntaxError: Expected '{'
-
> switch (1) { case a }
! !!!! Line: 1
! !!!! SyntaxError: Expected ':'
-
> switch (1) { case a: }
! !!!! ReferenceError: a is not defined
-
