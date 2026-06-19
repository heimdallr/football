#include "ui_Group.h"

#include "Group.h"

#include <QMenu>
#include <QTimer>

using namespace HomeCompa::Football;

class Group::Impl final : public QObject
{
public:
	Impl(Group& self, std::shared_ptr<ModelGroup> model, std::shared_ptr<Util::ItemViewToolTipper> itemViewToolTipper, std::shared_ptr<Util::ScrollBarController> scrollBarController)
		: m_self { self }
		, m_model { std::move(model) }
		, m_itemViewToolTipper { std::move(itemViewToolTipper) }
		, m_scrollBarController { std::move(scrollBarController) }
	{
		m_ui.setupUi(&m_self);
		m_ui.view->setModel(m_model.get());
		m_itemViewToolTipper->SetScrollArea(m_ui.view);
		m_scrollBarController->SetScrollArea(m_ui.view);

		connect(m_ui.view, &QWidget::customContextMenuRequested, this, &Impl::OnContextMenuRequested);
		connect(m_ui.view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &Impl::OnSelectionChanged);
		connect(m_model.get(), &QAbstractItemModel::modelAboutToBeReset, [this] {
			m_currentRow = m_ui.view->currentIndex().row();
		});
		connect(m_model.get(), &QAbstractItemModel::modelReset, [this] {
			m_ui.view->setCurrentIndex(m_model->index(m_currentRow, 0));
		});
	}

	void Init(const int idChamp)
	{
		m_model->setData({}, idChamp, ModelGroup::Role::ChampId);
		QTimer::singleShot(0, [this] {
			InitImpl();
		});
	}

private:
	void InitImpl()
	{
		const auto rowCount = m_model->rowCount();
		if (rowCount == 0)
			return;

		const auto groupSize   = m_model->data({}, ModelGroup::Role::GroupSize).toInt();
		auto*      header      = m_ui.view->horizontalHeader();
		const auto columnCount = header->count();

		const auto s          = m_ui.view->rowHeight(0);
		const auto scoreWidth = 3 * s;

		for (auto i = 0, sz = columnCount; i < sz; ++i)
		{
			header->resizeSection(i, s);
			header->setSectionResizeMode(i, QHeaderView::Fixed);
		}
		for (int i = 0; i < groupSize; ++i)
		{
			header->resizeSection(5 + i, scoreWidth);
			header->setSectionResizeMode(5 + i, QHeaderView::Fixed);
		}
		header->setSectionResizeMode(3, QHeaderView::Stretch);
		header->resizeSection(2, 3 * s / 2);
		header->resizeSection(8 + groupSize, scoreWidth);

		const auto groupCount = m_model->data({}, ModelGroup::Role::GroupCount).toInt();
		for (int i = 0; i < groupCount; ++i)
		{
			m_ui.view->setSpan(i * (groupSize + 1), 0, groupSize, 1);
			m_ui.view->setSpan(i * (groupSize + 1) + groupSize, 0, 1, columnCount);
		}

		InitActions();
	}

	void InitActions()
	{
		if (!m_actions.empty())
			return;

		const auto initAction = [this](QAction* action, const int result) {
			connect(action, &QAction::triggered, [this, result] {
				m_model->setData(m_ui.view->currentIndex(), result, ModelGroup::Role::Result);
			});
			action->setShortcut(QKeySequence(Qt::ALT | Qt::Key_0 + result));
			m_self.addAction(action);
		};

		initAction(m_actions.emplace_back(new QAction(tr("Clear result"), this)), 0);
		for (int i = 0, sz = m_model->data({}, ModelGroup::Role::GroupSize).toInt(); i < sz; ++i)
			initAction(m_actions.emplace_back(new QAction(tr("Set #%1").arg(i + 1), this)), i + 1);
	}

	void OnContextMenuRequested() const
	{
		QMenu menu;
		menu.setFont(m_self.font());
		for (auto* action : m_actions)
			menu.addAction(action);

		menu.exec(QCursor::pos());
	}

	void OnSelectionChanged()
	{
		assert(!m_actions.empty());
		m_actions.front()->setEnabled(m_model->data(m_ui.view->currentIndex(), ModelGroup::Role::Result).toInt() != 0);
	}

private:
	Group& m_self;

	PropagateConstPtr<ModelGroup, std::shared_ptr>                m_model;
	PropagateConstPtr<Util::ItemViewToolTipper, std::shared_ptr>  m_itemViewToolTipper;
	PropagateConstPtr<Util::ScrollBarController, std::shared_ptr> m_scrollBarController;

	std::vector<QAction*> m_actions;
	int                   m_currentRow;

	Ui::Group m_ui {};
};

Group::Group(std::shared_ptr<ModelGroup> model, std::shared_ptr<Util::ItemViewToolTipper> itemViewToolTipper, std::shared_ptr<Util::ScrollBarController> scrollBarController, QWidget* parent)
	: QWidget(parent)
	, m_impl(*this, std::move(model), std::move(itemViewToolTipper), std::move(scrollBarController))
{
}

Group::~Group() = default;

void Group::Init(const int idChamp)
{
	m_impl->Init(idChamp);
}
