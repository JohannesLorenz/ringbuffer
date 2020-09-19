# Documentation

This file documents some of the functionality.

There is also doxygen available, see the [README](README.md).

## Basic functionality

The ringbuffer is parted into two halves. While both halves are readable,
only one halve is writable.

## Non copyable types

If your ringbuffer needs to be able to store those, you should implement a
copier class, e.g.

```
using sampleFrame = float[2]; // has no copy ctor

class SampleFrameCopier
{
	const sampleFrame* m_src;
public:
	SampleFrameCopier(const sampleFrame* src) : m_src(src) {}
	void operator()(std::size_t src_offset, std::size_t count, sampleFrame* dest)
	{
		for (std::size_t i = src_offset; i < src_offset + count; i++, dest++)
		{
			(*dest)[0] = m_src[i][0];
			(*dest)[1] = m_src[i][1];
		}
	}
};
```

You many not use `ringbuffer_t<T>::write` then, but you can use
`ringbuffer_t<T>::write_func<F>`. If you subclass `ringbuffer_t<T>`, overwrite
`write`.

