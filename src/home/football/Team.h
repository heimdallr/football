#pragma once

#include <QWidget>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

class QSqlDatabase;

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
	explicit Team(QWidget* parent = nullptr);
	~Team() override;

	void Setup(std::shared_ptr<QSqlDatabase> db);
	MatchTeamInfo SetTeam(int idTeam);

private:
	class Impl;
	PropagateConstPtr<Impl> m_impl;
};

}
