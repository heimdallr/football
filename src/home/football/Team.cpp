#include "ui_Team.h"

#include "Team.h"

#include "model/reader.h"
#include "model/team.h"
#include "utilgui/MultiHeaderView.h"

using namespace HomeCompa::Football;
using namespace HomeCompa;

namespace
{

constexpr auto CONTEXT = "Team";

constexpr auto NUMBER  = QT_TRANSLATE_NOOP("Team", "#");
constexpr auto PLAYERS = QT_TRANSLATE_NOOP("Team", "Players");
constexpr auto NAME    = QT_TRANSLATE_NOOP("Team", "Name");
constexpr auto TYPE    = QT_TRANSLATE_NOOP("Team", "Type");
constexpr auto GOAL    = QT_TRANSLATE_NOOP("Team", "Goal");
constexpr auto COUNT   = QT_TRANSLATE_NOOP("Team", "Count");
constexpr auto MINUTE  = QT_TRANSLATE_NOOP("Team", "Minute");

QString Tr(const char* str)
{
	return QCoreApplication::translate(CONTEXT, str);
}

void AddHeader(QAbstractItemModel& model, QTableView& view)
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

	const auto s = header->sectionSizeFromContents(0).height();
	header->resizeSection(0, s);
	header->resizeSection(3, 2 * s);
	header->resizeSection(4, 2 * s);

	header->setSectionResizeMode(0, QHeaderView::Fixed);
	header->setSectionResizeMode(1, QHeaderView::Stretch);
	header->setSectionResizeMode(2, QHeaderView::Stretch);
	header->setSectionResizeMode(3, QHeaderView::Fixed);
	header->setSectionResizeMode(4, QHeaderView::Fixed);
}

} // namespace

class Team::Impl
{
public:
	explicit Impl(Team& self)
		: m_self { self }
	{
		m_ui.setupUi(&m_self);
	}

	void Setup(std::shared_ptr<QSqlDatabase> db)
	{
		auto  model       = std::make_unique<ModelTeam>(db);
		auto* sourceModel = model->sourceModel();
		m_modelPlayers.reset(std::move(model));
		m_modelSubstitutes.reset(std::make_unique<ModelTeam>(sourceModel));

		m_db.reset(std::move(db));

		AddHeader(*m_modelPlayers, *m_ui.viewPlayers);
		AddHeader(*m_modelSubstitutes, *m_ui.viewSubstitutes);
	}

	MatchTeamInfo SetTeam(const int idTeam)
	{
		m_modelPlayers->setData({}, idTeam, ModelTeam::Role::TeamId);

		QSqlQuery query("select NAME, GOAL_COUNT, PENALTY_COUNT from GET_MATCH_COUNTRY_INFO(?)", *m_db);
		query.bindValue(0, idTeam);
		query.exec();
		query.next();

		auto result = ReadItem<MatchTeamInfo>(query);

		m_ui.teamName->setText(result.name);

		return result;
	}

private:
	Team& m_self;

	PropagateConstPtr<QSqlDatabase, std::shared_ptr> m_db { std::shared_ptr<QSqlDatabase> {} };
	PropagateConstPtr<QAbstractItemModel>            m_modelPlayers { std::unique_ptr<QAbstractItemModel> {} };
	PropagateConstPtr<QAbstractItemModel>            m_modelSubstitutes { std::unique_ptr<QAbstractItemModel> {} };

	Ui::Team m_ui;
};

Team::Team(QWidget* parent)
	: QWidget(parent)
	, m_impl(*this)
{
}

Team::~Team() = default;

void Team::Setup(std::shared_ptr<QSqlDatabase> db)
{
	m_impl->Setup(std::move(db));
}

MatchTeamInfo Team::SetTeam(const int idTeam)
{
	return m_impl->SetTeam(idTeam);
}
