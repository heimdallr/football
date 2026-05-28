#pragma once

#include <QSortFilterProxyModel>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

class QSqlDatabase;

namespace HomeCompa::Football
{

class ModelTeam final : public QSortFilterProxyModel
{
	NON_COPY_MOVABLE(ModelTeam)

public:
	struct Role
	{
		enum
		{
			TeamId = Qt::UserRole + 1,
		};
	};

public:
	explicit ModelTeam(std::shared_ptr<QSqlDatabase> db, QObject* parent = nullptr);
	explicit ModelTeam(QAbstractItemModel* sourceModel, QObject* parent = nullptr);
	~ModelTeam() override;

private: // QSortFilterProxyModel
	bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
	const bool m_isSubstitutes { false };

	PropagateConstPtr<QAbstractItemModel> m_sourceModel;
};

}
