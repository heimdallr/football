#pragma once

#include <QWidget>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

#include "model/group.h"

namespace HomeCompa::Football
{

class Group final : public QWidget
{
	NON_COPY_MOVABLE(Group)

public:
	explicit Group(std::shared_ptr<ModelGroup> model, QWidget* parent = nullptr);
	~Group() override;

public:
	void Init(int idChamp);

private:
	class Impl;
	PropagateConstPtr<Impl> m_impl;
};

}
