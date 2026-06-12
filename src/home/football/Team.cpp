#include "ui_Team.h"

#include "Team.h"

#include <QInputDialog>
#include <QMenu>
#include <QSqlDriver>
#include <QTimer>

#include "model/reader.h"
#include "model/team.h"
#include "platformgui/PlatformGuiUtil.h"
#include "utilgui/MultiHeaderView.h"

#include "SqlDatabase.h"

using namespace HomeCompa::Football;
using namespace HomeCompa;

namespace
{

constexpr auto INPUT_DIALOG_GEOMETRY_KEY = "ui/MinuteInputDialog/geometry";

constexpr auto CONTEXT = "Team";

constexpr auto NUMBER  = QT_TRANSLATE_NOOP("Team", "#");
constexpr auto PLAYERS = QT_TRANSLATE_NOOP("Team", "Players");
constexpr auto NAME    = QT_TRANSLATE_NOOP("Team", "Name");
constexpr auto TYPE    = QT_TRANSLATE_NOOP("Team", "Type");
constexpr auto GOAL    = QT_TRANSLATE_NOOP("Team", "Goal");
constexpr auto COUNT   = QT_TRANSLATE_NOOP("Team", "Count");
constexpr auto MINUTE  = QT_TRANSLATE_NOOP("Team", "Minute");

constexpr auto GOAL_MENU        = QT_TRANSLATE_NOOP("Team", "GOAAAAAL!!");
constexpr auto SUBSTITUTE       = QT_TRANSLATE_NOOP("Team", "Substitute");
constexpr auto CARD             = QT_TRANSLATE_NOOP("Team", "Card");
constexpr auto ADD_PLAYER       = QT_TRANSLATE_NOOP("Team", "Add to the starting lineup");
constexpr auto REMOVE_PLAYER    = QT_TRANSLATE_NOOP("Team", "Remove from the list");
constexpr auto PLAYER_COUNT     = QT_TRANSLATE_NOOP("Team", "Players: %1");
constexpr auto SUBSTITUTE_COUNT = QT_TRANSLATE_NOOP("Team", "Substitutes: %1");
constexpr auto GET_MINUTE_TITLE = QT_TRANSLATE_NOOP("Team", "Enter the minute of the match");
constexpr auto GET_MINUTE_LABEL = QT_TRANSLATE_NOOP("Team", "Minute of the match");

class ViewResizer final : public QObject
{
public:
	explicit ViewResizer(QTableView& view)
		: QObject(&view)
		, m_view { view }
	{
	}

private: // QObject
	bool eventFilter(QObject*, QEvent* event) override
	{
		if (event->type() == QEvent::Resize)
			QTimer::singleShot(0, [this] {
				SetColumnsWidth();
			});

		return false;
	}

private:
	void SetColumnsWidth() const
	{
		const auto s = m_view.rowHeight(0);
		auto&      h = *m_view.horizontalHeader();
		h.resizeSection(0, s);
		h.resizeSection(3, 3 * s);
		h.resizeSection(4, 3 * s);

		const auto width = m_view.viewport()->width() - (h.sectionSize(0) + h.sectionSize(3) + h.sectionSize(4));
		h.resizeSection(1, 3 * width / 5);
		h.resizeSection(2, width - h.sectionSize(1));
	}

private:
	QTableView& m_view;
};

struct DictionaryItem
{
	int     id;
	QString name;
};

using Dictionary = std::vector<DictionaryItem>;

Dictionary SelectDictionary(const SqlDatabase& db, const QString& source)
{
	auto query = db.CreateQuery(QString("select id, name from get_%1_type").arg(source));
	query.exec();

	Dictionary dictionary;
	while (query.next())
		dictionary.emplace_back(ReadItem<DictionaryItem>(query));

	return dictionary;
}

template <typename F>
void AddContextSubMenu(QMenu& menu, const QString& title, const Dictionary& dictionary, const F& f)
{
	auto* subMenu = menu.addMenu(title);
	subMenu->setFont(menu.font());
	for (const auto& [id, name] : dictionary)
		subMenu->addAction(name, [id, f] {
			f(id);
		});
}

QString Tr(const char* str)
{
	return QCoreApplication::translate(CONTEXT, str);
}

void SetupView(QAbstractItemModel& model, QTableView& view, Util::ItemViewToolTipper& toolTipper, Util::ScrollBarController& scrollBarController)
{
	view.setModel(&model);

	auto* header = new Util::MultiHeaderView(Qt::Horizontal, 2, 5);

	header->setSpan(0, 0, 2, 0);
	header->setSpan(0, 1, 1, 2);
	header->setSpan(0, 3, 1, 2);

	header->setCellLabel(0, 0, Tr(NUMBER));
	header->setCellLabel(0, 1, Tr(PLAYERS));
	header->setCellLabel(1, 1, Tr(NAME));
	header->setCellLabel(1, 2, Tr(TYPE));
	header->setCellLabel(0, 3, Tr(GOAL));
	header->setCellLabel(1, 3, Tr(COUNT));
	header->setCellLabel(1, 4, Tr(MINUTE));

	view.setHorizontalHeader(header);
	for (int i = 0, sz = header->count(); i < sz; ++i)
		header->setSectionResizeMode(i, QHeaderView::Fixed);

	view.viewport()->installEventFilter(new ViewResizer(view));
	toolTipper.SetShowForceColumns({ 0 });
	toolTipper.SetScrollArea(&view);
	scrollBarController.SetScrollArea(&view);
}

std::pair<QVariant, QVariant> GetMinute(ISettings& settings, QWidget& parent, const int defaultMinute = 0)
{
	if (defaultMinute)
		return std::make_pair(QVariant { defaultMinute }, QVariant {});

	QInputDialog inputDialog(&parent);
	inputDialog.setFont(parent.font());
	inputDialog.setWindowTitle(Tr(GET_MINUTE_TITLE));
	inputDialog.setLabelText(Tr(GET_MINUTE_LABEL));
	inputDialog.setInputMode(QInputDialog::TextInput);

	QRegularExpression rx(R"(([0-9]{1,3})(\+[0-9]{1,2})?)");

	auto* lineEdit = inputDialog.findChild<QLineEdit*>();
	assert(lineEdit);
	lineEdit->setValidator(new QRegularExpressionValidator(rx, &inputDialog));

	QObject::connect(&inputDialog, &QDialog::finished, &inputDialog, [&] {
		settings.Set(INPUT_DIALOG_GEOMETRY_KEY, inputDialog.geometry());
	});
	QTimer::singleShot(0, [&] {
		if (auto geometry = settings.Get(INPUT_DIALOG_GEOMETRY_KEY); geometry.isValid())
			Platform::SetGeometry(inputDialog, geometry.toRect());
	});

	if (inputDialog.exec() != QDialog::Accepted)
		return {};

	const auto match = rx.match(inputDialog.textValue());
	if (!match.hasMatch())
		return {};

	const auto minute = match.captured(1);
	if (minute.isEmpty())
		return {};

	const auto additional = match.captured(2);

	return std::make_pair(QVariant { minute }, additional.isEmpty() ? QVariant {} : QVariant { additional.mid(1) });
}

} // namespace

class Team::Impl
{
public:
	explicit Impl(
		Team&                                      self,
		std::shared_ptr<ISettings>                 settings,
		std::shared_ptr<SqlDatabase>               db,
		std::shared_ptr<ViewDelegateStateFocus>    viewDelegateStateFocus,
		std::shared_ptr<Util::ItemViewToolTipper>  itemViewToolTipperPlayers,
		std::shared_ptr<Util::ScrollBarController> scrollBarControllerPlayers,
		std::shared_ptr<Util::ItemViewToolTipper>  itemViewToolTipperSubstitutes,
		std::shared_ptr<Util::ScrollBarController> scrollBarControllerSubstitutes
	)
		: m_self { self }
		, m_settings { std::move(settings) }
		, m_db { std::move(db) }
		, m_viewDelegateStateFocus { std::move(viewDelegateStateFocus) }
		, m_itemViewToolTipperPlayers { std::move(itemViewToolTipperPlayers) }
		, m_scrollBarControllerPlayers { std::move(scrollBarControllerPlayers) }
		, m_itemViewToolTipperSubstitutes { std::move(itemViewToolTipperSubstitutes) }
		, m_scrollBarControllerSubstitutes { std::move(scrollBarControllerSubstitutes) }
		, m_modelPlayers { ModelTeam::Create(m_db) }
		, m_modelSubstitutes { ModelTeam::Create(m_modelPlayers->data({}, ModelTeam::Role::SourceModel).value<QAbstractItemModel*>()) }
	{
		m_ui.setupUi(&m_self);

		m_ui.viewPlayers->setItemDelegate(m_viewDelegateStateFocus->GetDelegate());
		m_ui.viewSubstitutes->setItemDelegate(m_viewDelegateStateFocus->GetDelegate());

		SetupView(*m_modelPlayers, *m_ui.viewPlayers, *m_itemViewToolTipperPlayers, *m_scrollBarControllerPlayers);
		SetupView(*m_modelSubstitutes, *m_ui.viewSubstitutes, *m_itemViewToolTipperSubstitutes, *m_scrollBarControllerSubstitutes);

		connect(m_ui.viewPlayers, &QWidget::customContextMenuRequested, [this] {
			OnPlayersContextMenuRequested();
		});
		connect(m_ui.viewSubstitutes, &QWidget::customContextMenuRequested, [this] {
			OnSubstitutesContextMenuRequested();
		});
	}

	void SetMode(const Mode mode) const
	{
		if (mode == Mode::Left)
			return;

		m_ui.topLayout->removeWidget(m_ui.goals);
		m_ui.topLayout->insertWidget(0, m_ui.goals);
	}

	MatchTeamInfo SetTeam(const int idTeam)
	{
		if (!m_currentTeamId || *m_currentTeamId != idTeam)
		{
			m_currentTeamId = idTeam;
			UpdatePlayersInfo();
			m_subscription = m_db->Subscribe(QString("match_player_%1").arg(idTeam), [this] {
				UpdatePlayersInfo();
			});
		}

		auto result = GetTeamInfo();
		m_ui.teamName->setText(result.name);

		return result;
	}

	MatchTeamInfo GetTeamInfo() const
	{
		assert(m_currentTeamId);
		auto query = m_db->CreateQuery("select NAME, GOAL_COUNT, PENALTY_COUNT from GET_MATCH_COUNTRY_INFO(?)");
		query.bindValue(0, *m_currentTeamId);
		query.exec();
		query.next();

		auto result = ReadItem<MatchTeamInfo>(query);

		m_ui.goals->setText(QString::number(result.goalCount));

		return result;
	}

	void OnAddPlayerTriggered()
	{
		const auto currentIndex = m_ui.viewSubstitutes->currentIndex();
		if (!currentIndex.isValid() || !currentIndex.data(ModelTeam::Role::Number).isValid() || m_modelSubstitutes->data({}, ModelTeam::Role::PlayerCount).toULongLong() >= 11)
			return;

		assert(m_currentTeamId);
		const auto transaction = m_db->StartTransaction();
		auto       query       = m_db->CreateQuery("select id from add_match_player(?, ?)");
		query.bindValue(0, *m_currentTeamId);
		query.bindValue(1, currentIndex.data(ModelTeam::Role::ChampId));
		query.exec();
		query.next();
	}

	void OnRemovePlayerTriggered()
	{
		if (!m_ui.viewPlayers->currentIndex().isValid())
			return;

		const auto transaction = m_db->StartTransaction();
		auto       query       = m_db->CreateQuery("execute procedure del_match_player(?)");
		query.bindValue(0, m_ui.viewPlayers->currentIndex().data(ModelTeam::Role::MatchId));
		query.exec();
	}

private:
	void UpdatePlayersInfo()
	{
		m_modelPlayers->setData({}, *m_currentTeamId, ModelTeam::Role::TeamId);

		if (const auto count = m_modelPlayers->data({}, ModelTeam::Role::PlayerCount).toULongLong(); count > 0 && count < 11)
			m_ui.playerCount->setText(Tr(PLAYER_COUNT).arg(count));
		else
			m_ui.playerCount->setText({});

		if (const auto count = m_modelPlayers->data({}, ModelTeam::Role::SubstituteCount).toULongLong(); count > 0)
			m_ui.substituteCount->setText(Tr(SUBSTITUTE_COUNT).arg(count));
		else
			m_ui.substituteCount->setText({});
	}

	void OnPlayersContextMenuRequested()
	{
		const auto currentIndex = m_ui.viewPlayers->currentIndex();
		if (!currentIndex.isValid())
			return;

		UpdateDictionaries();
		QMenu menu;
		menu.setFont(m_self.font());

		AddContextSubMenu(menu, Tr(GOAL_MENU), m_goals, [this](const int id) {
			OnGoalTriggered(id);
		});

		if (auto* action = menu.addAction(Tr(SUBSTITUTE), [this] {
				OnSubstituteTriggered();
			}))
		{
			if (const auto substituteIndex = m_ui.viewSubstitutes->currentIndex(); !substituteIndex.isValid() || !substituteIndex.data(ModelTeam::Role::Number).isValid())
				action->setEnabled(false);
		}

		AddContextSubMenu(menu, Tr(CARD), m_cards, [this](const int id) {
			OnCardTriggered(*m_ui.viewPlayers, id);
		});
		menu.addSeparator();
		menu.addAction(
				Tr(REMOVE_PLAYER),
				[this] {
					OnRemovePlayerTriggered();
				}
		)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Delete));
		menu.exec(QCursor::pos());
	}

	void OnSubstitutesContextMenuRequested()
	{
		const auto currentIndex = m_ui.viewSubstitutes->currentIndex();
		if (!currentIndex.isValid())
			return;

		const auto hasNumber = currentIndex.data(ModelTeam::Role::Number).isValid();

		UpdateDictionaries();
		QMenu menu;
		menu.setFont(m_self.font());

		if (auto* action = menu.addAction(Tr(ADD_PLAYER), [this] {
				OnAddPlayerTriggered();
			}))
		{
			if (hasNumber && m_modelSubstitutes->data({}, ModelTeam::Role::PlayerCount).toULongLong() < 11)
				action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
			else
				action->setEnabled(false);
		}

		AddContextSubMenu(menu, Tr(CARD), m_cards, [this](const int id) {
			OnCardTriggered(*m_ui.viewSubstitutes, id);
		});

		if (auto* action = menu.addAction(
				Tr(SUBSTITUTE),
				[this] {
					OnSubstituteTriggered();
				}
			);
		    !hasNumber || !m_ui.viewPlayers->currentIndex().isValid())
			action->setEnabled(false);

		menu.exec(QCursor::pos());
	}

	void OnGoalTriggered(const int id)
	{
		const auto index = m_ui.viewPlayers->currentIndex();
		assert(index.isValid());

		const auto [minute, additional] = GetMinute(*m_settings, m_self, id % 1000);
		if (!minute.isValid())
			return;

		const auto transaction = m_db->StartTransaction();
		auto       query       = m_db->CreateQuery("select id from add_goal(?, ?, ?, ?)");
		query.bindValue(0, index.data(ModelTeam::Role::MatchId));
		query.bindValue(1, id / 1000);
		query.bindValue(2, minute);
		query.bindValue(3, additional);
		query.exec();
		query.next();
	}

	void OnCardTriggered(const QAbstractItemView& view, const int id)
	{
		const auto index = view.currentIndex();
		assert(index.isValid() && m_currentTeamId);

		const auto [minute, additional] = GetMinute(*m_settings, m_self);
		if (!minute.isValid())
			return;

		const auto transaction = m_db->StartTransaction();
		auto       query       = m_db->CreateQuery("select id from add_card(?, ?, ?, ?, ?)");
		query.bindValue(0, *m_currentTeamId);
		query.bindValue(1, index.data(ModelTeam::Role::ChampId));
		query.bindValue(2, id);
		query.bindValue(3, minute);
		query.bindValue(4, additional);
		query.exec();
		query.next();
	}

	void OnSubstituteTriggered()
	{
		const auto playerIndex = m_ui.viewPlayers->currentIndex(), substituteIndex = m_ui.viewSubstitutes->currentIndex();
		assert(playerIndex.isValid() && substituteIndex.isValid());

		const auto [minute, additional] = GetMinute(*m_settings, m_self);
		if (!minute.isValid())
			return;

		const auto transaction = m_db->StartTransaction();
		auto       query       = m_db->CreateQuery("select id from add_substitute(?, ?, ?, ?)");
		query.bindValue(0, playerIndex.data(ModelTeam::Role::MatchId));
		query.bindValue(1, substituteIndex.data(ModelTeam::Role::ChampId));
		query.bindValue(2, minute);
		query.bindValue(3, additional);
		query.exec();
		query.next();
	}

	void UpdateDictionaries()
	{
		if (!m_cards.empty())
			return;

		m_cards = SelectDictionary(*m_db, "CARD");
		m_goals = SelectDictionary(*m_db, "GOAL");
	}

private:
	Team& m_self;

	PropagateConstPtr<ISettings, std::shared_ptr>                 m_settings;
	PropagateConstPtr<SqlDatabase, std::shared_ptr>               m_db;
	PropagateConstPtr<ViewDelegateStateFocus, std::shared_ptr>    m_viewDelegateStateFocus;
	PropagateConstPtr<Util::ItemViewToolTipper, std::shared_ptr>  m_itemViewToolTipperPlayers;
	PropagateConstPtr<Util::ScrollBarController, std::shared_ptr> m_scrollBarControllerPlayers;
	PropagateConstPtr<Util::ItemViewToolTipper, std::shared_ptr>  m_itemViewToolTipperSubstitutes;
	PropagateConstPtr<Util::ScrollBarController, std::shared_ptr> m_scrollBarControllerSubstitutes;

	PropagateConstPtr<QAbstractItemModel> m_modelPlayers;
	PropagateConstPtr<QAbstractItemModel> m_modelSubstitutes;

	Dictionary m_cards;
	Dictionary m_goals;

	std::optional<int>                    m_currentTeamId;
	SqlDatabase::SubscriptionWrapper::Ptr m_subscription;

	Ui::Team m_ui;
};

Team::Team(
	std::shared_ptr<ISettings>                 settings,
	std::shared_ptr<SqlDatabase>               db,
	std::shared_ptr<ViewDelegateStateFocus>    viewDelegateStateFocus,
	std::shared_ptr<Util::ItemViewToolTipper>  itemViewToolTipperPlayers,
	std::shared_ptr<Util::ScrollBarController> scrollBarControllerPlayers,
	std::shared_ptr<Util::ItemViewToolTipper>  itemViewToolTipperSubstitutes,
	std::shared_ptr<Util::ScrollBarController> scrollBarControllerSubstitutes,
	QWidget*                                   parent
)
	: QWidget(parent)
	, m_impl(
		  *this,
		  std::move(settings),
		  std::move(db),
		  std::move(viewDelegateStateFocus),
		  std::move(itemViewToolTipperPlayers),
		  std::move(scrollBarControllerPlayers),
		  std::move(itemViewToolTipperSubstitutes),
		  std::move(scrollBarControllerSubstitutes)
	  )
{
}

Team::~Team() = default;

void Team::SetMode(const Mode mode)
{
	m_impl->SetMode(mode);
}

MatchTeamInfo Team::SetTeam(const int idTeam)
{
	return m_impl->SetTeam(idTeam);
}

MatchTeamInfo Team::GetInfo() const
{
	return m_impl->GetTeamInfo();
}

void Team::AddPlayer()
{
	m_impl->OnAddPlayerTriggered();
}

void Team::RemovePlayer()
{
	m_impl->OnRemovePlayerTriggered();
}
