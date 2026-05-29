#include "ui_MainWindow.h"

#include "MainWindow.h"

#include <QMenu>
#include <QSqlQuery>
#include <QTimer>

#include <config/version.h>

#include "utilgui/GeometryRestorable.h"

#include "SettingsConstant.h"
#include "Team.h"

using namespace HomeCompa::Football;
using namespace HomeCompa;

namespace
{

constexpr auto MAIN_WINDOW      = "MainWindow";
constexpr auto FONT_SIZE_KEY    = "ui/Font/pointSizeF";
constexpr auto CHAMP_HEADER_KEY = "ui/ChampHeaderView/layout";

constexpr auto FONT_SIZE_DEFAULT = 9;

QString GetChampInfo(const ISettings& settings, const SqlDatabase& db)
{
	if (auto query = db.CreateQuery("select info from get_champ_info(?)"); query.bindValue(0, settings.Get(Constant::CHAMP_ID_KEY)), query.exec() && query.next())
		return query.value(0).toString();
	return {};
}

} // namespace

class MainWindow::Impl final
	: Util::GeometryRestorable
	, Util::GeometryRestorableObserver
{
	NON_COPY_MOVABLE(Impl)

public:
	Impl(MainWindow& self, std::shared_ptr<ISettings> settings, std::shared_ptr<SqlDatabase> db, std::shared_ptr<ModelChamp> modelChamp, std::shared_ptr<Match> match)
		: GeometryRestorable(*this, settings, MAIN_WINDOW)
		, GeometryRestorableObserver(self)
		, m_self { self }
		, m_settings { std::move(settings) }
		, m_db { std::move(db) }
		, m_modelChamp { std::shared_ptr<QAbstractItemModel> { std::move(modelChamp) } }
		, m_match { std::move(match) }
		, m_champInfo { GetChampInfo(*m_settings, *m_db) }
	{
		m_ui.setupUi(&m_self);

		m_ui.viewChamp->setModel(m_modelChamp.get());
		m_ui.viewChamp->resizeColumnsToContents();
		m_ui.viewChamp->addActions({ m_ui.actionMatchDetails, m_ui.actionChangeMatchEndFlag });
		SetSpans(1);

		m_ui.stackedWidget->addWidget(m_match.get());

		connect(m_ui.stackedWidget, &QStackedWidget::currentChanged, [this](const int index) {
			OnStackedWidgetCurrentChanged(index);
		});
		connect(m_modelChamp.get(), &QAbstractItemModel::modelAboutToBeReset, [this] {
			QTimer::singleShot(0, [this, row = m_ui.viewChamp->currentIndex().row()] {
				m_ui.viewChamp->setCurrentIndex(m_modelChamp->index(row, 0));
			});
		});
		connect(m_ui.viewChamp->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selection) {
			m_ui.actionChangeMatchEndFlag->setEnabled(!selection.isEmpty());
			m_ui.actionMatchDetails->setEnabled(!selection.isEmpty());
		});
		connect(m_ui.actionChangeMatchEndFlag, &QAction::triggered, [this] {
			if (const auto index = m_ui.viewChamp->currentIndex(); index.isValid())
				m_modelChamp->setData(index, {}, ModelChamp::Role::SwitchMatchEndFlag);
		});
		connect(m_ui.actionHome, &QAction::triggered, [this] {
			m_ui.stackedWidget->setCurrentIndex(0);
		});
		connect(m_ui.actionMatchDetails, &QAction::triggered, [this] {
			if (const auto index = m_ui.viewChamp->currentIndex(); index.isValid())
				m_ui.stackedWidget->setCurrentIndex(1);
		});
		const auto incrementFontSize = [&](const int value) {
			const auto fontSize = m_settings->Get(FONT_SIZE_KEY, FONT_SIZE_DEFAULT);
			m_settings->Set(FONT_SIZE_KEY, fontSize + value);
		};
		connect(m_ui.actionFontSizeUp, &QAction::triggered, &m_self, [=] {
			incrementFontSize(1);
		});
		connect(m_ui.actionFontSizeDown, &QAction::triggered, &m_self, [=] {
			incrementFontSize(-1);
		});
		connect(m_ui.viewChamp, &QWidget::customContextMenuRequested, [this] {
			OnViewChampContextMenuRequested();
		});
		connect(m_match.get(), &Match::MatchTeamInfoChanged, [this](const std::pair<MatchTeamInfo, MatchTeamInfo>& teamInfo) {
			const auto& [team1, goal1, penalty1] = teamInfo.first;
			const auto& [team2, goal2, penalty2] = teamInfo.second;
			SetTitle(QString("%1 - %2, %3:%4%5").arg(team1, team2).arg(goal1).arg(goal2).arg(penalty1 + penalty2 > 0 ? QString(" (%1:%2)").arg(penalty1).arg(penalty2) : QString {}));
		});

		LoadGeometry();
		if (const auto viewChampLayout = m_settings->Get(CHAMP_HEADER_KEY); viewChampLayout.isValid())
			m_ui.viewChamp->horizontalHeader()->restoreState(viewChampLayout.toByteArray());

		SetTitle(m_champInfo);

		m_self.addActions({ m_ui.actionExit, m_ui.actionHome, m_ui.actionFontSizeUp, m_ui.actionFontSizeDown });
	}

	~Impl() override
	{
		SaveGeometry();
		m_settings->Set(CHAMP_HEADER_KEY, m_ui.viewChamp->horizontalHeader()->saveState());
	}

private:
	void OnStackedWidgetCurrentChanged(const int pageIndex)
	{
		switch (pageIndex)
		{
			case 0:
				return SetTitle(m_champInfo);

			case 1:
			{
				const auto index              = m_ui.viewChamp->currentIndex();
				const auto [idTeam1, idTeam2] = index.data(ModelChamp::Role::TeamIds).value<std::pair<int, int>>();
				return m_match->SetTeams(index.data(ModelChamp::Role::MatchId).toInt(), idTeam1, idTeam2);
			}

			default:
				break;
		}

		assert(false && "unexpected page index");
	}

	void SetSpans(const int column)
	{
		QVariant currentValue;
		int      currentRow = 0;
		for (auto row = 0, rowCount = m_modelChamp->rowCount(); row < rowCount; ++row)
		{
			if (auto value = m_modelChamp->index(row, column).data(); currentValue != value)
			{
				currentValue = std::move(value);
				if (row - currentRow > 1)
					m_ui.viewChamp->setSpan(currentRow, column, row - currentRow, 1);
				currentRow = row;
			}
		}
	}

	void SetTitle(const QString& title) const
	{
		m_self.setWindowTitle(QString("%1 - [%2]").arg(PRODUCT_ID, title));
	}

	void OnViewChampContextMenuRequested()
	{
		QMenu menu;
		menu.setFont(m_self.font());
		menu.addActions({ m_ui.actionMatchDetails, m_ui.actionChangeMatchEndFlag });
		menu.addSeparator();
		menu.exec(QCursor::pos());
	}

private:
	MainWindow& m_self;

	PropagateConstPtr<ISettings, std::shared_ptr>          m_settings;
	PropagateConstPtr<SqlDatabase, std::shared_ptr>        m_db;
	PropagateConstPtr<QAbstractItemModel, std::shared_ptr> m_modelChamp;
	PropagateConstPtr<Match, std::shared_ptr>              m_match;

	QString m_champInfo;

	Ui::MainWindow m_ui {};
};

MainWindow::MainWindow(std::shared_ptr<ISettings> settings, std::shared_ptr<SqlDatabase> db, std::shared_ptr<ModelChamp> modelChamp, std::shared_ptr<Match> match, QWidget* parent)
	: QMainWindow(parent)
	, m_impl(*this, std::move(settings), std::move(db), std::move(modelChamp), std::move(match))
{
}

MainWindow::~MainWindow() = default;
