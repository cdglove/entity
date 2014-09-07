#include "Entity/DenseComponentPool.h"
#include "Entity/SparseComponentPool.h"
#include "Entity/ForEach.h"
#include "Entity/ForAll.h"

#include <iostream>

int main()
{
	Entity::EntityPool ep(1024 * 1024);
	Entity::Entity e = ep.create();

	Entity::SparseComponentPool<float> cp(ep);
	float& f = *cp.create(e);
	f = 10.f;

	Entity::DenseComponentPool<int> cp2(ep);
	int* i = cp2.create(e);
	*i = 2;

	Entity::for_each(ep, cp, [](float f)
	{
		std::cout << f;
	});

	Entity::for_each(ep, cp, cp2, [](float f, int i)
	{
		std::cout << f * i;
	});

	Entity::for_all(ep, cp, [](float* f)
	{
		std::cout << *f;
	});

	Entity::for_all(ep, cp, cp2, [](float* f, int* i)
	{
		std::cout << *f * *i;
	});	
	
	cp2.destroy(e);
	cp.destroy(e);
	ep.destroy(e);
	return 0;
}