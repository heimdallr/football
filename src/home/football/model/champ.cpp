#include "champ.h"

using namespace HomeCompa::Football;

namespace
{

class Model final : public QAbstractTableModel
{
private: // QAbstractTableModel
	int columnCount(const QModelIndex&) const override
	{
		return 2;
	}

	int rowCount(const QModelIndex&) const override
	{
		return 0;
	}

	QVariant data(const QModelIndex& /*index*/, int /*role*/) const override
	{
		return {};
	}
};

}

ModelChamp::ModelChamp(QObject* parent)
	: QIdentityProxyModel(parent)
	, m_sourceModel { std::unique_ptr<QAbstractItemModel> { std::make_unique<Model>() } }
{
	QIdentityProxyModel::setSourceModel(m_sourceModel.get());
}

ModelChamp::~ModelChamp() = default;
