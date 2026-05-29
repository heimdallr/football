#pragma once

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

#include "SqlDatabase.h"
#include "Team.h"

namespace HomeCompa::Football
{

class Match final : public QWidget
{
	Q_OBJECT
	NON_COPY_MOVABLE(Match)

signals:
	void MatchTeamInfoChanged(const std::pair<MatchTeamInfo, MatchTeamInfo>& info) const;

public:
	Match(std::shared_ptr<SqlDatabase> db, std::shared_ptr<Team> team1, std::shared_ptr<Team> team2, QWidget* parent = nullptr);
	~Match() override;

	void SetTeams(int idMatch, int idTeam1, int idTeam2);

private:
	class Impl;
	PropagateConstPtr<Impl> m_impl;
};

} // namespace HomeCompa::Football
