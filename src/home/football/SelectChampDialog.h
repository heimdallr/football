#pragma once

#include <QDialog>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

#include "settings/ISettings.h"
#include "utilgui/ItemViewToolTipper.h"
#include "utilgui/ScrollBarController.h"

#include "SqlDatabase.h"

namespace HomeCompa::Football
{

class SelectChampDialog final : public QDialog
{
	NON_COPY_MOVABLE(SelectChampDialog)

public:
	SelectChampDialog(
		const std::shared_ptr<SqlDatabase>&        db,
		std::shared_ptr<ISettings>                 settings,
		std::shared_ptr<Util::ItemViewToolTipper>  itemViewToolTipper,
		std::shared_ptr<Util::ScrollBarController> scrollBarController,
		QWidget*                                   parent = nullptr
	);
	~SelectChampDialog() override;

private:
	class Impl;
	PropagateConstPtr<Impl> m_impl;
};

}
