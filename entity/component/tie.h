//! \file entity/component/tie.h
//
// Ties component poos together to be used with algorithms
// iterators etc.  
// Replaces the need to use variadics everywhere and
// possibly makes client code more readable by explicitly
// tieing component pools together.
// 

#include <boost/fusion/container/vector.hpp>

// ----------------------------------------------------------------------------
//
namespace entity
{
	// ------------------------------------------------------------------------
	//
	template<typename Pool>
	boost::fusion::vector<Pool&> tie(Pool& pool)
	{
		return boost::fusion::vector<Pool&>(pool);
	}

	template<typename Pool1, typename Pool2>
	boost::fusion::vector<Pool1&, Pool2&> tie(Pool1& pool_1, Pool2& pool_2)
	{
		return boost::fusion::vector<Pool1&, Pool2&>(pool_1, pool_2);
	}

	template<typename Pool1, typename Pool2, typename Pool3>
	boost::fusion::vector<Pool1&, Pool2&, Pool3&> tie(Pool1& pool_1, Pool2& pool_2, Pool3& pool_3)
	{
		return boost::fusion::vector<Pool1&, Pool2&, Pool3&>(pool_1, pool_2, pool_3);
	}
}