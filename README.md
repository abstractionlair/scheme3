This is a fraction of a scheme(ish) interpreter.
It can do simple things.
There is a bug in the closures handling so that only one argument works.
Since writing it I have realized that this design will never be able to support continuations because it relies on C's stack rather than explicitly managing its own.
