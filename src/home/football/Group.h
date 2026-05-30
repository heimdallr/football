#pragma once

#include <QWidget>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

#include "model/group.h"
#include "utilgui/ItemViewToolTipper.h"
#include "utilgui/ScrollBarController.h"

namespace HomeCompa::Football
{

class Group final : public QWidget
{
	NON_COPY_MOVABLE(Group)

public:
	Group(std::shared_ptr<ModelGroup> model, std::shared_ptr<Util::ItemViewToolTipper> itemViewToolTipper, std::shared_ptr<Util::ScrollBarController> scrollBarController, QWidget* parent = nullptr);
	~Group() override;

public:
	void Init(int idChamp);

private:
	class Impl;
	PropagateConstPtr<Impl> m_impl;
};

}
