#pragma once

#include <QIdentityProxyModel>
#include <QSqlDatabase>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

#include "settings/ISettings.h"

namespace HomeCompa::Football
{

class ModelChamp final : public QIdentityProxyModel
{
	NON_COPY_MOVABLE(ModelChamp)

public:
	struct Role
	{
		enum
		{
			SwitchMatchEndFlag = Qt::UserRole + 1,
			TeamIds,
		};
	};

public:
	ModelChamp(std::shared_ptr<ISettings> settings, std::shared_ptr<QSqlDatabase> db, QObject* parent = nullptr);
	~ModelChamp() override;

private:
	PropagateConstPtr<QAbstractItemModel> m_sourceModel;
};

}
