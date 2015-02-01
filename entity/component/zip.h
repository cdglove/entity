// ****************************************************************************
// entity/component/zip.h
//
// Creates a tuple of component views such that we
// can iterate over them as one iterator.
// 
// Copyright Chris Glover 2014
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
# pragma once
#ifndef _ENTITY_COMPONENT_ZIP_H_INCLUDED_
#define _ENTITY_COMPONENT_ZIP_H_INCLUDED_

#include "entity/config.h"
#include <boost/mpl/transform.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/algorithm/iteration/fold.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/fusion/algorithm/query/count.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/fusion/include/at.hpp>
#include "entity/functional/window.h"

// ----------------------------------------------------------------------------
//
namespace entity 
{
	namespace detail
	{
		template<typename ComponentPool>
		struct component_window_type
		{
			typedef typename 
			std::decay<
				ComponentPool
			>::type::window type;
		};

		template<typename ComponentPoolTuple>
		struct component_windows_type
		{
			typedef typename
			boost::mpl::transform<
				ComponentPoolTuple, 
				component_window_type<boost::mpl::_1>
			>::type type;
		};

		struct sized_increment_window
		{
			sized_increment_window(entity target)
				: target_(target)
			{}

			template<typename ComponentPoolWindow>
			std::size_t operator()(std::size_t result, ComponentPoolWindow& view) const
			{
				return view.increment(target_) ? result + 1 : result;
			}

			entity target_;
		};

		struct sized_advance_window
		{
			sized_advance_window(entity target)
				: target_(target)
			{}

			template<typename ComponentPoolWindow>
			std::size_t operator()(std::size_t result, ComponentPoolWindow& view) const
			{
				return view.advance(target_) ? result + 1 : result;
			}

			entity target_;
		};
	}

	// ------------------------------------------------------------------------
	//
	template<typename ComponentPoolTuple>
	class zipped_component_windows
	{
	public:

		typedef typename 
		boost::fusion::result_of::as_vector<
			typename detail::component_windows_type<
			ComponentPoolTuple
			>::type
		>::type windows_type;

		zipped_component_windows(ComponentPoolTuple&& pools)
		{
			windows_ = boost::fusion::as_vector(
				boost::fusion::transform(
					pools,
					functional::get_window()
				)
			);
		}

		std::size_t count()
		{
			return boost::fusion::count(windows_);
		}

		std::size_t increment(entity e)
		{
			return boost::fusion::fold(
				windows_,
				0,
				detail::sized_increment_window(e)
			);
		}

		std::size_t advance(entity e)
		{
			return boost::fusion::fold(
				windows_,
				0,
				detail::sized_advance_window(e)
			);
		}

		windows_type get() const
		{
			return windows_;
		}

	private:

		windows_type windows_;
	};

	// ------------------------------------------------------------------------
	//
	template<typename T>
	struct is_zipped_component_tuple
	{
		static const bool value = false;
	};

	template<typename ComponentTuple>
	struct is_zipped_component_tuple<zipped_component_windows<ComponentTuple>>
	{
		static const bool value = true;
	};

	// ------------------------------------------------------------------------
	//
	template<std::size_t idx, typename ZippedComponentWindows>
	typename std::enable_if<
		is_zipped_component_tuple<ZippedComponentWindows>::value,
		typename boost::fusion::result_of::value_at_c<
			typename ZippedComponentWindows::windows_type, idx
		>::type::value_type&
	>::type get(ZippedComponentWindows const& windows)
	{
		return boost::fusion::at_c<idx>(windows.get()).get();
	}

	template<typename ComponentPoolTuple>
	zipped_component_windows<ComponentPoolTuple> zip_from_tuple(ComponentPoolTuple pools)
	{
		return zipped_component_windows<
			ComponentPoolTuple
		>(std::move(pools));
	}

	// ------------------------------------------------------------------------
	//
#if ENTITY_SUPPORT_VARIADICS
	template<typename... Pools>
	auto zip(Pools&... pools) -> decltype(zip_from_tuple(tie(pools...)))
	{
		return zip_from_tuple(tie(pools...));
	}
#else
	template<typename Pool>
	auto zip(Pool& pool) -> decltype(zip_from_tuple(tie(pool)))
	{
		return zip_from_tuple(tie(pool));
	}

	template<typename Pool1, typename Pool2>
	auto zip(Pool1& pool_1, Pool2& pool_2) -> decltype(zip_from_tuple(tie(pool_1, pool_2)))
	{
		return zip_from_tuple(tie(pool_1, pool_2));
	}

	template<typename Pool1, typename Pool2, typename Pool3>
	auto zip(Pool1& pool_1, Pool2& pool_2, Pool3& pool_3) -> decltype(zip_from_tuple(tie(pool_1, pool_2, pool_3)))
	{
		return zip_from_tuple(tie(pool_1, pool_2, pool_3));
	}
#endif
}

#endif // _ENTITY_COMPONENT_ZIP_H_INCLUDED_