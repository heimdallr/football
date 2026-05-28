#include "ui_Match.h"

#include "Match.h"

using namespace HomeCompa::Football;

class Match::Impl
{
public:
	explicit Impl(Match& self)
		: m_self { self }
	{
		m_ui.setupUi(&m_self);
	}

	void Setup(std::shared_ptr<QSqlDatabase> db) const
	{
		m_ui.team1->Setup(db);
		m_ui.team2->Setup(std::move(db));
	}

	std::pair<MatchTeamInfo, MatchTeamInfo> SetTeams(const int idTeam1, const int idTeam2) const
	{
		return std::make_pair(m_ui.team1->SetTeam(idTeam1), m_ui.team2->SetTeam(idTeam2));
	}

private:
	Match& m_self;

	Ui::Match m_ui {};
};

Match::Match(QWidget* parent)
	: QWidget(parent)
	, m_impl(*this)
{
}

Match::~Match() = default;

void Match::Setup(std::shared_ptr<QSqlDatabase> db) const
{
	m_impl->Setup(std::move(db));
}

std::pair<MatchTeamInfo, MatchTeamInfo> Match::SetTeams(const int idTeam1, const int idTeam2) const
{
	return m_impl->SetTeams(idTeam1, idTeam2);
}
