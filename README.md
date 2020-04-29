### Peacock Framework v0.1 Beta

## Definition:
This framework is an interactive terminal I've written using PDCurses and a lot of patience in C++ for my lab assistant Joshu.
With this, we now have a proper framework to call our utilities from.

P.S. This whole thing was made using MSVC, trying to compile it in GCC, Clang or other compilers won't guarantee you a fun time.

## Known bugs:
* Curses doesn't do well with maximizing as far as I can tell. In my case, it's been known to cause weird behavior, like the cursor showing up where it shouldn't or other windows resizing themselves suddenly. Try to avoid maximizing.
  
## Utilities:

# The Matrixinator v1.1 (Beta)
The Matrixinator is a small utility I made for my lab assistant Joshu.

He once asked me about an application that could help him predict 8 values,
named BS22, BS15, BS3, BS8, BS27, BS84, BS18 and BS278. These would be
calculated off a similarity matrix generated from a similarity dendrogram,
loaded from a metadata file in .csv format and a dendrogram export in .xml format.
Or, so it was. Now it no longer generates a similarity matrix, for there's no need.

So, being the kind soul I am, I offered my help and thus developed this utility
in C++ for him to use in his lab. It doesn't seem like he understands the language
this program is written in... so he's attempting to recreate it in Python.

Future plans include:
- [x] Improve execution time (52 mins is too long!);
- [x] Provide basic initialization file support (typing directories is annoying);
- [x] ~~Migrate to CMake and enable use of Boost libraries;~~ (no longer needed)
- [x] An automatic way of finding .xml and .csv files for use (through `<filesystem>` from C++ 17 standard library);
- [ ] ~~Made it a sensitive program that does not like to be called names or told off. It'll turn highly offensive so don't do it!;~~ This version has been disciplined and will behave politely. For now.
- [x] Include a proper menu ~~and add command line flags (like -detailed);~~ Peacock does that nicely, and no need for command line arguments anymore.
- [ ] Getting started on the Python version of this, kindly nicknamed "The Pythrixinator";
- [x] ~~A prettier UI in a far, remote future.~~ Peacock is damn beautiful already, and much more user-friendly. Can be improved in a much farther future.

> Leo, 29-Apr-2020