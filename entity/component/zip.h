//! \file entity/component/zip.h
//
// Creates a tuple of component views such that we
// can iterate over them as one iterator.
// 
#pragma once
#ifndef _ENTITY_COMPONENT_ZIP_H_INCLUDED_
#define _ENTITY_COMPONENT_ZIP_H_INCLUDED_

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
	zipped_component_windows<ComponentPoolTuple> zip(ComponentPoolTuple&& pools)
	{
		return zipped_component_windows<
			ComponentPoolTuple
		>(std::forward<ComponentPoolTuple>(pools));
	}
}

#endif // _ENTITY_COMPONENT_ZIP_H_INCLUDED_