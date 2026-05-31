#include "ui_Match.h"

#include "Match.h"

#include "SqlDatabase.h"

using namespace HomeCompa::Football;

namespace
{

bool HasFocus(const QWidget& widget)
{
	return widget.hasFocus() || std::ranges::any_of(widget.findChildren<const QWidget*>(), [](const QWidget* w) {
			   return w->hasFocus();
		   });
}

}

class Match::Impl
{
	NON_COPY_MOVABLE(Impl)

public:
	explicit Impl(Match& self, std::shared_ptr<SqlDatabase> db, std::shared_ptr<Team> team1, std::shared_ptr<Team> team2)
		: m_self { self }
		, m_db { std::move(db) }
		, m_team1 { std::move(team1) }
		, m_team2 { std::move(team2) }
	{
		m_ui.setupUi(&m_self);

		m_team1->SetMode(Team::Mode::Left);
		m_team2->SetMode(Team::Mode::Right);
		m_ui.layout->addWidget(m_team2.get());
		m_ui.layout->insertWidget(0, m_team1.get());

		m_self.addActions({ m_ui.actionAddPlayer, m_ui.actionRemovePlayer });

		connect(m_ui.actionAddPlayer, &QAction::triggered, [this] {
			if (auto* team = GetActiveTeam())
				team->AddPlayer();
		});
		connect(m_ui.actionRemovePlayer, &QAction::triggered, [this] {
			if (auto* team = GetActiveTeam())
				team->RemovePlayer();
		});
	}

	~Impl()
	{
		m_ui.layout->removeWidget(m_team1.get());
		m_ui.layout->removeWidget(m_team2.get());
	}

	void SetTeams(const int idMatch, const int idTeam1, const int idTeam2)
	{
		emit m_self.MatchTeamInfoChanged(std::make_pair(m_team1->SetTeam(idTeam1), m_team2->SetTeam(idTeam2)));

		if (m_idMatch && *m_idMatch == idMatch)
			return;

		m_idMatch      = idMatch;
		m_subscription = m_db->Subscribe(QString("match_%1").arg(idMatch), [this] {
			emit m_self.MatchTeamInfoChanged(std::make_pair(m_team1->GetInfo(), m_team2->GetInfo()));
		});
	}

private:
	Team* GetActiveTeam()
	{
		return HasFocus(*m_team1) ? m_team1.get() : HasFocus(*m_team2) ? m_team2.get() : nullptr;
	}

private:
	Match& m_self;

	PropagateConstPtr<SqlDatabase, std::shared_ptr> m_db;
	PropagateConstPtr<Team, std::shared_ptr>        m_team1, m_team2;

	std::optional<int>                    m_idMatch;
	SqlDatabase::SubscriptionWrapper::Ptr m_subscription;

	Ui::Match m_ui {};
};

Match::Match(std::shared_ptr<SqlDatabase> db, std::shared_ptr<Team> team1, std::shared_ptr<Team> team2, QWidget* parent)
	: QWidget(parent)
	, m_impl(*this, std::move(db), std::move(team1), std::move(team2))
{
}

Match::~Match() = default;

void Match::SetTeams(const int idMatch, const int idTeam1, const int idTeam2)
{
	m_impl->SetTeams(idMatch, idTeam1, idTeam2);
}
