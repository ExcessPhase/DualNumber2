#include <boost/mp11.hpp>
#include <array>
#include <iostream>


	/// meta function to merge two sets
template<typename L, typename R>
struct merge
{	typedef boost::mp11::mp_set_union<L, R> merged;
	typedef boost::mp11::mp_sort<merged, boost::mp11::mp_less> type;
};


	/// the dual number class to be instantiated by passing a SET like e.g.
	/// DualNumber<boost::mp11::mp_list<boost::mp_size_t<0> > >
template<typename SET>
struct DualNumber
{		/// the value
	double m_dValue;
		/// number of independent variables
	static constexpr std::size_t SIZE = boost::mp11::mp_size<SET>::value;
		/// container type for derivatives
	typedef std::array<double, SIZE> ARRAY;
		/// derivative values
	ARRAY m_adDer;
	DualNumber(void) = default;
	DualNumber(const double _d, const bool)
		:m_dValue(_d),
		m_adDer({1.0})
	{	static_assert(SIZE == 1, "size must be 1");
	}
		/// functor to be used for multiplication 
	template<typename SET1>
	struct multiplication
	{	typedef typename merge<SET, SET1>::type TYPE;
		DualNumber<TYPE>&m_rResult;
		const DualNumber<SET>&m_rL;
		const DualNumber<SET1>&m_rR;
		multiplication(
			DualNumber<TYPE>&_rResult,
			const DualNumber<SET>&_rL,
			const DualNumber<SET1>&_rR
		)
			:m_rResult(_rResult),
			m_rL(_rL),
			m_rR(_rR)
		{
		}
			/// derivative is contained in both arguments
		template<typename TPOS, typename LPOS, typename RPOS>
		void operator()(const TPOS&, const LPOS&, const RPOS&) const
		{	m_rResult.m_adDer[TPOS::value] =
				m_rL.m_adDer[LPOS::value]*m_rR.m_dValue
					+ m_rR.m_adDer[RPOS::value]*m_rL.m_dValue;
		}
			/// derivative is not contained in the left argument
		template<typename TPOS, typename RPOS>
		void operator()(
			const TPOS&,
			const boost::mp11::mp_size_t<DualNumber<SET>::SIZE>&,
			const RPOS&
		) const
		{	m_rResult.m_adDer[TPOS::value] =
				m_rR.m_adDer[RPOS::value]*m_rL.m_dValue;
		}
			/// derivative is not contained in the right argument
		template<typename TPOS, typename LPOS>
		void operator()(
			const TPOS&,
			const LPOS&,
			const boost::mp11::mp_size_t<DualNumber<SET1>::SIZE>&
		) const
		{	m_rResult.m_adDer[TPOS::value] =
				m_rL.m_adDer[LPOS::value]*m_rR.m_dValue;
		}
			/// entry point called for a particular independent variable in the TARGET
		template<typename INDEP>
		void operator()(const INDEP&) const
		{	(*this)(
				boost::mp11::mp_find<TYPE, INDEP>(),
				boost::mp11::mp_find<SET, INDEP>(),
				boost::mp11::mp_find<SET1, INDEP>()
			);
		}
	};
		/// multiplication operator for different DualNumber types
	template<typename SET1>
	auto operator*(
		const DualNumber<SET1>&_r
	) const
	{	typedef typename merge<SET, SET1>::type TYPE;
		DualNumber<TYPE> s;
		s.m_dValue = m_dValue*_r.m_dValue;
		boost::mp11::mp_for_each<TYPE>(multiplication<SET1>(s, *this, _r));
		return s;
	}
	friend std::ostream &operator<<(std::ostream &_rS, const DualNumber&_r)
	{	_rS << "value=" << _r.m_dValue << "\n";
		for (const auto d : _r.m_adDer)
			_rS << "der=" << d << "\n";
		return _rS;
	}
};

int main()
{
	const DualNumber<boost::mp11::mp_list<boost::mp11::mp_size_t<0> > > s0(1.1, true);
	const DualNumber<boost::mp11::mp_list<boost::mp11::mp_size_t<1> > > s1(1.2, true);
	std::cout << s0*s1 << std::endl;
}
