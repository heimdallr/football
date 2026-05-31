#include "Factory.h"

#include <Hypodermic/Container.h>

#include "SelectChampDialog.h"

using namespace HomeCompa::Football;

Factory::Factory(Hypodermic::Container& container)
	: m_container { container }
{
}

Factory::~Factory() = default;

std::shared_ptr<SelectChampDialog> Factory::CreateSelectChampDialog() const
{
	return m_container.resolve<SelectChampDialog>();
}
