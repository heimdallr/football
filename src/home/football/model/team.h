#pragma once

#include <QSortFilterProxyModel>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

namespace HomeCompa::Football
{

class SqlDatabase;

class ModelTeam final : public QSortFilterProxyModel
{
	NON_COPY_MOVABLE(ModelTeam)

public:
	struct Role
	{
		enum
		{
			TeamId = Qt::UserRole + 1,
			MatchId,
			SubstituteMinute,
			ChampId,
			Number,
			PlayerCount,
			SubstituteCount,
			SourceModel,
			CardColor,
		};
	};

	static std::unique_ptr<QAbstractItemModel> Create(std::shared_ptr<SqlDatabase> db);
	static std::unique_ptr<QAbstractItemModel> Create(QAbstractItemModel* sourceModel);

public:
	explicit ModelTeam(std::shared_ptr<SqlDatabase> db, QObject* parent = nullptr);
	explicit ModelTeam(QAbstractItemModel* sourceModel, QObject* parent = nullptr);
	~ModelTeam() override;

private: // QSortFilterProxyModel
	bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
	const bool m_isSubstitutes { false };

	PropagateConstPtr<QAbstractItemModel> m_sourceModel;
};

} // namespace HomeCompa::Football
