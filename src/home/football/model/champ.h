#pragma once

#include <QIdentityProxyModel>

#include "fnd/NonCopyMovable.h"
#include "fnd/memory.h"

namespace HomeCompa::Football
{

class ModelChamp final : public QIdentityProxyModel
{
	NON_COPY_MOVABLE(ModelChamp)

public:
	ModelChamp(QObject* parent = nullptr);
	~ModelChamp() override;

private:
	PropagateConstPtr<QAbstractItemModel> m_sourceModel;
};

}
