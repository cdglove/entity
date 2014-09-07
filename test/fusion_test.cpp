#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector.hpp>

template<typename T>
struct S
{
	typedef T type;
};

template<typename T>
struct extract_ptr_type
{
	typedef typename T::type* type;
};

template<typename List>
struct Q
{
	typedef typename
	boost::mpl::transform<
		List,
		extract_ptr_type<boost::mpl::_1>
	>::type type;
};

int main()
{
	//boost::fusion::vector<long> v;
	typedef 
	boost::mpl::push_front<
		boost::mpl::transform<
			boost::mpl::vector<S<int>, S<int>, S<float>>::type,
			extract_ptr_type<boost::mpl::_1>
		>::type,
		long
	>::type t;
	// typedef
	// boost::fusion::push_front<		
	// 	boost::fusion::result_of::as_vector<
	// 		boost::mpl::vector<S<int>, S<int>, S<float>>
	// 	>::type,
	// 	long
	// >::type t;
	
	boost::fusion::result_of::as_vector<t>::type q;
}