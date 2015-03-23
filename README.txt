0 Greetings
:::::::::::

Welcome! Thank you for considering the README file.

CONTENTS

1 What is ringbuffer?
2 Why using ringbuffer?
3 License
4 Documentation
5 Installation & Debugging
6 Examples
7 Thanks to
8 Links
9 Contact

1 What is ringbuffer?
::::::::::::::::::

`ringbuffer' is a simple library containing a ringbuffer (definition see wikipedia todo).

The ringbuffer is lock-free (using atomics only), and allows multiple readers,
but only one writer.

The ringbuffer is real-time safe, i.e. guarantees that the functions return
"fast enough" (except to initialization and cleanup routines).

2 Why using ringbuffer?
::::::::::::::::::::

As mentioned, the advantages are:

 * a leightweight library, few dependencies
 * lock-freeness while still supporting multiple readers
 * real-time safety
 * safety (tests are included)

3 License
:::::::::

Please see the LICENSE.txt file.

4 Documentation
:::::::::::::::

Type make doc for doxygen. TODO: more stuff here...

5 Installation & Debugging
::::::::::::::::::::::::::

Please see the INSTALL.txt file.

6 Examples
::::::::::

7 Thanks to
:::::::::::

In alphabetical order, I thank:

  * Mark McCurry (fundamental) [1] from zynaddsubfx [2]

8 Links
:::::::

[1] http://fundamental-code.com/
[2] http://zynaddsubfx.sourceforge.net/

9 Contact
:::::::::

Feel free to give feedback. My e-mail address is shown if you execute this in
a shell:

  printf "\x6a\x6f\x68\x61\x6e\x6e\x65\x73\x40\x6c\x6f
  \x72\x65\x6e\x7a\x2d\x68\x6f\x2e\x6d\x65\x0a"

