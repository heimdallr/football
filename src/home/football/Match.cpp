#include "ui_Match.h"

#include "Match.h"

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
public:
	explicit Impl(Match& self)
		: m_self { self }
	{
		m_ui.setupUi(&m_self);
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

	void Setup(std::shared_ptr<SqlDatabase> db) const
	{
		m_ui.team1->Setup(db);
		m_ui.team2->Setup(std::move(db));
	}

	std::pair<MatchTeamInfo, MatchTeamInfo> SetTeams(const int idTeam1, const int idTeam2) const
	{
		return std::make_pair(m_ui.team1->SetTeam(idTeam1), m_ui.team2->SetTeam(idTeam2));
	}

private:
	Team* GetActiveTeam() const
	{
		return HasFocus(*m_ui.team1) ? m_ui.team1 : HasFocus(*m_ui.team2) ? m_ui.team2 : nullptr;
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

void Match::Setup(std::shared_ptr<SqlDatabase> db) const
{
	m_impl->Setup(std::move(db));
}

std::pair<MatchTeamInfo, MatchTeamInfo> Match::SetTeams(const int idTeam1, const int idTeam2) const
{
	return m_impl->SetTeams(idTeam1, idTeam2);
}
