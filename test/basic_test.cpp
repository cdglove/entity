#include "entity/dense_component_pool.h"
#include "entity/sparse_component_pool.h"
#include "entity/algorithm/for_each.h"
#include "entity/algorithm/for_all.h"

#include <iostream>

int main()
{
	entity::entity_pool ep(1024 * 1024);
	entity::entity e = ep.create();

	entity::sparse_component_pool<float> cp(ep);
	float& f = *cp.create(e);
	f = 10.f;

	entity::dense_component_pool<int> cp2(ep);
	int* i = cp2.create(e);
	*i = 2;

	entity::for_each(ep, cp, [](float f)
	{
		std::cout << f;
	});

	entity::for_each(ep, cp, cp2, [](float f, int i)
	{
		std::cout << f * i;
	});

	entity::for_all(ep, cp, [](float* f)
	{
		std::cout << *f;
	});

	entity::for_all(ep, cp, cp2, [](float* f, int* i)
	{
		std::cout << *f * *i;
	});	
	
	cp2.destroy(e);
	cp.destroy(e);
	ep.destroy(e);
	return 0;
}