#include "ui_SelectChampDialog.h"

#include "SelectChampDialog.h"

#include "model/reader.h"
#include "utilgui/GeometryRestorable.h"

#include "SettingsConstant.h"

using namespace HomeCompa::Football;

namespace
{

struct Role
{
	enum
	{
		Id = Qt::UserRole + 1,
	};
};

struct Item
{
	int     id;
	int     year;
	QString title;
};

using Items = std::vector<Item>;

class Model final : public QAbstractTableModel
{
public:
	explicit Model(const SqlDatabase& db)
	{
		auto query = db.CreateQuery("select id, year_, description from get_champ");
		if (query.exec())
			while (query.next())
				m_items.emplace_back(ReadItem<Item>(query));
	}

private: // QAbstractItemModel
	int columnCount(const QModelIndex&) const override
	{
		return 2;
	}

	int rowCount(const QModelIndex& parent) const override
	{
		return parent.isValid() ? 0 : static_cast<int>(m_items.size());
	}

	QVariant data(const QModelIndex& index, const int role) const override
	{
		assert(index.isValid() && index.row() < rowCount({}));
		const auto& item = m_items[index.row()];
		switch (role)
		{
			case Qt::DisplayRole:
			case Qt::ToolTipRole:
				return index.column() == 0 ? QVariant { item.year } : index.column() == 1 ? QVariant { item.title } : QVariant {};

			case Role::Id:
				return item.id;

			default:
				break;
		}
		return {};
	}

private:
	Items m_items;
};

} // namespace

class SelectChampDialog::Impl final
	: Util::GeometryRestorable
	, Util::GeometryRestorableObserver
{
	NON_COPY_MOVABLE(Impl)

public:
	Impl(
		QDialog&                                   self,
		const SqlDatabase&                         db,
		std::shared_ptr<ISettings>                 settings,
		std::shared_ptr<Util::ItemViewToolTipper>  itemViewToolTipper,
		std::shared_ptr<Util::ScrollBarController> scrollBarController
	)
		: GeometryRestorable(*this, settings, "SelectChampDialog")
		, GeometryRestorableObserver(self)
		, m_settings { std::move(settings) }
		, m_itemViewToolTipper { std::move(itemViewToolTipper) }
		, m_scrollBarController { std::move(scrollBarController) }
		, m_model { std::unique_ptr<QAbstractItemModel> { std::make_unique<Model>(db) } }
	{
		m_ui.setupUi(&self);
		self.addActions({ m_ui.actionSelect, m_ui.actionDiscard });

		m_ui.view->setModel(m_model.get());
		m_ui.view->viewport()->installEventFilter(m_itemViewToolTipper.get());
		m_ui.view->viewport()->installEventFilter(m_scrollBarController.get());
		m_scrollBarController->SetScrollArea(m_ui.view);

		if (const auto match = m_model->match(m_model->index(0, 0), Role::Id, m_settings->Get(Constant::CHAMP_ID_KEY, -1), 1, Qt::MatchExactly); !match.isEmpty())
			m_ui.view->setCurrentIndex(match.front());

		LoadGeometry();

		connect(&self, &QDialog::accepted, [this] {
			if (const auto index = m_ui.view->currentIndex(); index.isValid())
				m_settings->Set(Constant::CHAMP_ID_KEY, index.data(Role::Id).toInt());
		});
	}

	~Impl() override
	{
		SaveGeometry();
	}

private:
	std::shared_ptr<ISettings> m_settings;

	PropagateConstPtr<Util::ItemViewToolTipper, std::shared_ptr>  m_itemViewToolTipper;
	PropagateConstPtr<Util::ScrollBarController, std::shared_ptr> m_scrollBarController;

	PropagateConstPtr<QAbstractItemModel> m_model;

	Ui::SelectChampDialog m_ui {};
};

SelectChampDialog::SelectChampDialog(
	const std::shared_ptr<SqlDatabase>&        db,
	std::shared_ptr<ISettings>                 settings,
	std::shared_ptr<Util::ItemViewToolTipper>  itemViewToolTipper,
	std::shared_ptr<Util::ScrollBarController> scrollBarController,
	QWidget*                                   parent
)
	: QDialog(parent)
	, m_impl(*this, *db, std::move(settings), std::move(itemViewToolTipper), std::move(scrollBarController))
{
}

SelectChampDialog::~SelectChampDialog() = default;
