#include "di_app.h"

#include <QCoreApplication>

#include <config/version.h>

#include "Hypodermic/Hypodermic.h"
#include "settings//Settings.h"

#include "MainWindow.h"

namespace HomeCompa::Football
{

void DiInit(Hypodermic::ContainerBuilder& builder, std::shared_ptr<Hypodermic::Container>& container)
{
	builder.registerType<MainWindow>().as<QMainWindow>();

	builder
		.registerInstanceFactory([](auto&) -> std::shared_ptr<SettingsFactory::AbstractSettings> {
			return SettingsFactory::Create(QString("%1/%2.ini").arg(QCoreApplication::applicationDirPath()).arg(PRODUCT_ID));
		})
		.as<ISettings>()
		.singleInstance();

	builder
		.registerInstanceFactory([](Hypodermic::ComponentContext& ctx) -> std::shared_ptr<QSqlDatabase> {
			auto db = std::make_shared<QSqlDatabase>(QSqlDatabase::addDatabase("QIBASE"));
			if (!db->isValid())
				return {};

			const auto    settings = ctx.resolve<ISettings>();
			SettingsGroup connection(*settings, "Connection");

			db->setHostName(settings->Get("Host").toString());
			if (const auto port = settings->Get("Port"); port.isValid())
				db->setPort(port.toInt());
			db->setDatabaseName(settings->Get("DatabaseName").toString());
			if (db->open(settings->Get("User").toString(), settings->Get("Password").toString()))
				return db;

			return {};
		})
		.singleInstance();

	container = builder.build();
}

} // namespace HomeCompa::Football
