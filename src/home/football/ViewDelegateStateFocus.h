#pragma once

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

class QAbstractItemDelegate;

namespace HomeCompa::Football
{

class ViewDelegateStateFocus
{
	NON_COPY_MOVABLE(ViewDelegateStateFocus)

public:
	ViewDelegateStateFocus();
	~ViewDelegateStateFocus();

public:
	QAbstractItemDelegate* GetDelegate() noexcept;

private:
	class Impl;
	PropagateConstPtr<Impl> m_impl;
};

}
