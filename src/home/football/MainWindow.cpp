#include "ui_MainWindow.h"

#include "MainWindow.h"

#include <QSqlQuery>

#include "utilgui/GeometryRestorable.h"

#include "SettingsConstant.h"

using namespace HomeCompa::Football;
using namespace HomeCompa;

namespace
{

constexpr auto MAIN_WINDOW = "MainWindow";

}

class MainWindow::Impl final
	: Util::GeometryRestorable
	, Util::GeometryRestorableObserver
{
	NON_COPY_MOVABLE(Impl)

public:
	Impl(MainWindow& self, std::shared_ptr<ISettings> settings, std::shared_ptr<QSqlDatabase> db, std::shared_ptr<ModelChamp> modelChamp)
		: GeometryRestorable(*this, settings, MAIN_WINDOW)
		, GeometryRestorableObserver(self)
		, m_self { self }
		, m_settings { std::move(settings) }
		, m_db { std::move(db) }
		, m_modelChamp { std::shared_ptr<QAbstractItemModel> { std::move(modelChamp) } }
	{
		m_ui.setupUi(&m_self);

		m_ui.viewChamp->setModel(m_modelChamp.get());
		m_ui.viewChamp->resizeColumnsToContents();
		m_ui.viewChamp->addActions({ m_ui.actionMatchDetails, m_ui.actionChangeMatchEndFlag });
		SetSpans(1);

		m_ui.pageMatch->Setup(m_db);

		connect(m_ui.stackedWidget, &QStackedWidget::currentChanged, [this](const int index) {
			if (index == 1)
			{
				const auto [idTeam1, idTeam2] = m_modelChamp->data(m_ui.viewChamp->currentIndex(), ModelChamp::Role::TeamIds).value<std::pair<int, int>>();
				m_ui.pageMatch->SetTeams(idTeam1, idTeam2);
			}
		});

		connect(m_ui.actionChangeMatchEndFlag, &QAction::triggered, [this] {
			m_modelChamp->setData(m_ui.viewChamp->currentIndex(), {}, ModelChamp::Role::SwitchMatchEndFlag);
		});
		connect(m_ui.actionHome, &QAction::triggered, [this] {
			m_ui.stackedWidget->setCurrentIndex(0);
		});
		connect(m_ui.actionMatchDetails, &QAction::triggered, [this] {
			m_ui.stackedWidget->setCurrentIndex(1);
		});

		LoadGeometry();

		if (QSqlQuery query(QString("select info from get_champ_info(?)"), *m_db); query.bindValue(0, m_settings->Get(Constant::CHAMP_ID_KEY)), query.exec() && query.next())
			m_self.setWindowTitle(query.value(0).toString());

		m_self.addActions({ m_ui.actionExit, m_ui.actionHome });
	}

	~Impl() override
	{
		SaveGeometry();
	}

private:
	void SetSpans(const int column)
	{
		QVariant currentValue;
		int     currentRow = 0;
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

private:
	MainWindow& m_self;

	PropagateConstPtr<ISettings, std::shared_ptr>          m_settings;
	PropagateConstPtr<QSqlDatabase, std::shared_ptr>       m_db;
	PropagateConstPtr<QAbstractItemModel, std::shared_ptr> m_modelChamp;

	Ui::MainWindow m_ui {};
};

MainWindow::MainWindow(std::shared_ptr<ISettings> settings, std::shared_ptr<QSqlDatabase> db, std::shared_ptr<ModelChamp> modelChamp, QWidget* parent)
	: QMainWindow(parent)
	, m_impl(*this, std::move(settings), std::move(db), std::move(modelChamp))
{
}

MainWindow::~MainWindow() = default;
