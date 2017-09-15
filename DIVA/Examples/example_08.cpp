/* example_08.cpp */
template<typename T>
class EnumerateRange
{
public:

	class Iterator
	{
	public:
		Iterator(const T &value)
			: m_value(value) // call the constructor by copy of T
			// the constructor must not be the trivial
		{
		}

		T operator*(void) const
		{
			return m_value;
		}

		void operator++(void)
		{
			m_value.operator++();
		}

		bool operator!=(const Iterator &rhs) const
		{
			return m_value != rhs.m_value;
		}

	protected:
		T m_value;
	};

	EnumerateRange(const T &end)
		: m_begin(0)
		, m_end(end)
	{
	}
	EnumerateRange(const T &begin, const T &end)
		: m_begin(begin)
		, m_end(end)
	{
	}

	Iterator begin()
	{
		return Iterator(m_begin);
	}

	Iterator end()
	{
		// must call the constructor of Iterator
		return Iterator(m_end);
	}

	T m_begin;
	T m_end;
};

template<typename T>
EnumerateRange<T> Range(const T &end)
{
	return EnumerateRange<T>(end);
}

template<typename T>
EnumerateRange<T> Range(const T &begin, const T &end)
{
	return EnumerateRange<T>(begin, end);
}

struct Elem
{
	Elem(int value) : m_value(10) { }

	// must not be trivial => cause an "optimisation" that cause the 0 line record bug
	Elem(const Elem &elem) { m_value = elem.m_value; if (m_value == 0) m_value += 1; }

	int value() { return m_value; }

	bool operator==(const Elem &rhs) const { return m_value == rhs.m_value; }
	bool operator!=(const Elem &rhs) const { return !this->operator==(rhs); }
	Elem operator++() { return Elem(m_value + 1); }

	int m_value;
};

int main()
{
	EnumerateRange<Elem> range(10);

	for (auto i : range)
	{
	}
	return 1;
}
