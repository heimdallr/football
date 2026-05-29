#pragma once

#include <QWidget>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

namespace HomeCompa::Football
{

class SqlDatabase;

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
	explicit Team(QWidget* parent = nullptr);
	~Team() override;

	void          Setup(std::shared_ptr<SqlDatabase> db);
	MatchTeamInfo SetTeam(int idTeam);

	void AddPlayer();
	void RemovePlayer();

private:
	class Impl;
	PropagateConstPtr<Impl> m_impl;
};

} // namespace HomeCompa::Football
