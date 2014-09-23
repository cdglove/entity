//! \file entity/component_pool_destruction_queue.h
//
// Represents a way to queue component destruction in order to 
// reduce creation complexity from O(m*nlog(n)) to O(m+n)
// 

#include <vector>
#include <utility>

// ----------------------------------------------------------------------------
//
namespace entity
{
	template<typename ComponentPool>
	class component_pool_destruction_queue
	{
	public:

		typedef typename ComponentPool::type type;

		component_pool_destruction_queue(ComponentPool& p)
			: m_Pool(p)
		{}

		~component_pool_destruction_queue()
		{
			flush();
		}

		template<typename... Args>
		void push(entity e)
		{
			m_Destroyed.push_back(e);
		}

		void flush()
		{
			std::sort(m_Destroyed.begin(), m_Destroyed.end(), std::greater<entity>());
			m_Pool.destroy_range(m_Destroyed.begin(), m_Destroyed.end());
			clear();
		}

		void clear()
		{
			m_Destroyed.clear();
		}

	private: 

		// No copying.
		component_pool_destruction_queue(component_pool_destruction_queue const&);
		component_pool_destruction_queue operator=(component_pool_destruction_queue);

		std::vector<entity> m_Destroyed;
		ComponentPool& m_Pool;
	};
}