In cases of the algorithm finding a letter that's already used in that particular word.

E.g.:

Dictionary word: ABBA
Board:

ABx
xBx

This would actually match since:

Word:         ABBA
Letter Index: 1234

1Bx
xBx

A2x
xBx

ABx
x3x

and then go TOP-LEFT for an A again:

4Bx
xBx

This is obviously not allowed, but needs to be checked for.
