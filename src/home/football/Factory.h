#pragma once

#include <memory>

#include "fnd/NonCopyMovable.h"

namespace Hypodermic
{

class Container;

}

namespace HomeCompa::Football
{

class SelectChampDialog;

class IFactory // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	virtual ~IFactory() = default;

	virtual std::shared_ptr<SelectChampDialog> CreateSelectChampDialog() const = 0;
};

class Factory final : public IFactory
{
	NON_COPY_MOVABLE(Factory)

public:
	explicit Factory(Hypodermic::Container& container);
	~Factory() override;

private: // IFactory
	std::shared_ptr<SelectChampDialog> CreateSelectChampDialog() const override;

private:
	Hypodermic::Container& m_container;
};

}
