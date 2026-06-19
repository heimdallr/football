#pragma once

#include <QIdentityProxyModel>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

#include "SqlDatabase.h"

namespace HomeCompa::Football
{

class ModelGroup final : public QIdentityProxyModel
{
	NON_COPY_MOVABLE(ModelGroup)

public:
	struct Role
	{
		enum
		{
			ChampId = Qt::UserRole + 1,
			GroupSize,
			GroupCount,
			Result,
		};
	};

public:
	explicit ModelGroup(std::shared_ptr<SqlDatabase> db, QObject* parent = nullptr);
	ModelGroup(std::shared_ptr<SqlDatabase> db, QWidget* parent);
	~ModelGroup() override;

private:
	PropagateConstPtr<QAbstractItemModel> m_sourceModel;
};

}
