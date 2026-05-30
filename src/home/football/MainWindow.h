#pragma once

#include <QMainWindow>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

#include "model/champ.h"
#include "settings/ISettings.h"
#include "utilgui/ItemViewToolTipper.h"
#include "utilgui/ScrollBarController.h"

#include "Group.h"
#include "Match.h"
#include "SqlDatabase.h"

namespace HomeCompa::Football
{

class MainWindow final : public QMainWindow
{
	Q_OBJECT
	NON_COPY_MOVABLE(MainWindow)

public:
	MainWindow(
		std::shared_ptr<ISettings>                 settings,
		std::shared_ptr<SqlDatabase>               db,
		std::shared_ptr<ModelChamp>                modelChamp,
		std::shared_ptr<Match>                     match,
		std::shared_ptr<Group>                     group,
		std::shared_ptr<Util::ItemViewToolTipper>  itemViewToolTipper,
		std::shared_ptr<Util::ScrollBarController> scrollBarController,
		QWidget*                                   parent = nullptr
	);
	~MainWindow() override;

private:
	class Impl;
	PropagateConstPtr<Impl> m_impl;
};

}
