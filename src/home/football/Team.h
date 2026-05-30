#pragma once

#include <QWidget>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

#include "SqlDatabase.h"
#include "utilgui/ItemViewToolTipper.h"
#include "utilgui/ScrollBarController.h"

namespace HomeCompa
{

class ISettings;

}

namespace HomeCompa::Football
{

struct MatchTeamInfo
{
	QString name;
	int     goalCount;
	int     penaltyCount;
};

class Team final : public QWidget
{
	Q_OBJECT
	NON_COPY_MOVABLE(Team)

public:
	Team(
		std::shared_ptr<ISettings>                 settings,
		std::shared_ptr<SqlDatabase>               db,
		std::shared_ptr<Util::ItemViewToolTipper>  itemViewToolTipperPlayers,
		std::shared_ptr<Util::ScrollBarController> scrollBarControllerPlayers,
		std::shared_ptr<Util::ItemViewToolTipper>  itemViewToolTipperSubstitutes,
		std::shared_ptr<Util::ScrollBarController> scrollBarControllerSubstitutes,
		QWidget*                                   parent = nullptr
	);
	~Team() override;

	MatchTeamInfo SetTeam(int idTeam);
	MatchTeamInfo GetInfo() const;

	void AddPlayer();
	void RemovePlayer();

private:
	class Impl;
	PropagateConstPtr<Impl> m_impl;
};

} // namespace HomeCompa::Football
