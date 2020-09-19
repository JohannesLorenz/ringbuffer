# 0 Greetings

Welcome! Thank you for considering the README file.

**Contents**

  1. What is ringbuffer?
  2. Why using ringbuffer?
  3. License
  4. Documentation
  5. Installation & Debugging
  6. Examples
  7. Thanks to
  8. Contact

# 1 What is ringbuffer?

`ringbuffer' is a simple library containing a
[ringbuffer](https://en.wikipedia.org/wiki/Circular_buffer).

The ringbuffer is lock-free (using atomics only), and allows multiple readers,
but only one writer.

The ringbuffer is real-time safe, i.e. guarantees that the functions return
"fast enough" (except to initialization and cleanup routines).

Parts of the code have their origin in
[JACK's ringbuffer](https://github.com/jackaudio/jack2/blob/develop/common/ringbuffer.c).

# 2 Why using ringbuffer?

As mentioned, the advantages are:

 * minimality
 * no dependencies
 * lock-freeness while still supporting multiple readers
 * real-time safety
 * safety (tests are included)

# 3 License

Please see the [LICENSE](LICENSE.txt) file.

# 4 Documentation

See the [DOCUMENTATION](DOCUMENTATION.md) for official docs.

Type make doc for doxygen.

# 5 Installation & Debugging

Please see the [INSTALL](INSTALL.md) file.

# 6 Examples

See tests.

# 7 Thanks to

In alphabetical order, I thank:
  * Martin Pavelek (@he29-net)
  * The JACK team for their ringbuffer

# 8 Contact

Feel free to give feedback on
[github](https://github.com/JohannesLorenz/ringbuffer).


