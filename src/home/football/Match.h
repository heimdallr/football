#pragma once

#include <QWidget>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

class QSqlDatabase;

namespace HomeCompa::Football
{

class Match final : public QWidget
{
	Q_OBJECT
	NON_COPY_MOVABLE(Match)

public:
	explicit Match(QWidget* parent = nullptr);
	~Match() override;

	void Setup(std::shared_ptr<QSqlDatabase> db) const;
	void SetTeams(int idTeam1, int idTeam2) const;

private:
	class Impl;
	PropagateConstPtr<Impl> m_impl;
};

}
