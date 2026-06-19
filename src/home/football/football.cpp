#include <QApplication>
#include <QDir>
#include <QMainWindow>
#include <QStyleHints>

#include "Hypodermic/Hypodermic.h"
#include "logging/init.h"
#include "settings/ISettings.h"

#include "di_app.h"
#include "log.h"

#include "config/git_hash.h"
#include "config/version.h"

using namespace HomeCompa::Football;
using namespace HomeCompa;

int main(int argc, char* argv[])
{
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

	[[maybe_unused]] QApplication app(argc, argv);
	QCoreApplication::setApplicationName(PRODUCT_ID);
	QCoreApplication::setApplicationVersion(PRODUCT_VERSION);

	const auto              defaultLogPath = QString("%1/%2.%3.log").arg(QDir::tempPath(), COMPANY_ID, PRODUCT_ID);
	Log::LoggingInitializer logging(defaultLogPath);

	try
	{
		PLOGI << "App started";
		PLOGI << "Version: " << PRODUCT_VERSION;
		PLOGI << "Commit hash: " << GIT_HASH;

		std::shared_ptr<Hypodermic::Container> container;
		{
			Hypodermic::ContainerBuilder builder;
			DiInit(builder, container);
		}
		PLOGD << "DI-container created";

		auto settings = container->resolve<ISettings>();
		QApplication::setStyle(settings->Get("ui/Style", QString { "fusion" }));
		QGuiApplication::styleHints()->setColorScheme(settings->Get("ui/ColorScheme", Qt::ColorScheme::Unknown));

		const auto mainWindow = container->resolve<QMainWindow>();
		mainWindow->show();

		return QApplication::exec();
	}
	catch (const std::exception& ex)
	{
		PLOGE << ex.what();
	}
}
