#include "ui_MainWindow.h"

#include "MainWindow.h"

#include "utilgui/GeometryRestorable.h"

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
	Impl(MainWindow& self, std::shared_ptr<ISettings> settings, std::shared_ptr<ModelChamp> modelChamp)
		: GeometryRestorable(*this, std::move(settings), MAIN_WINDOW)
		, GeometryRestorableObserver(self)
		, m_self { self }
		, m_modelChamp { std::move(modelChamp) }
	{
		m_ui.setupUi(&m_self);

		m_ui.viewChamp->setModel(modelChamp.get());

		LoadGeometry();

		m_self.addAction(m_ui.actionExit);
	}

	~Impl() override
	{
		SaveGeometry();
	}

private:
	MainWindow& m_self;

	PropagateConstPtr<ModelChamp, std::shared_ptr> m_modelChamp;

	Ui::MainWindow m_ui {};
};

MainWindow::MainWindow(std::shared_ptr<ISettings> settings, std::shared_ptr<ModelChamp> modelChamp, QWidget* parent)
	: QMainWindow(parent)
	, m_impl(*this, std::move(settings), std::move(modelChamp))
{
}

MainWindow::~MainWindow() = default;
