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
		.registerInstanceFactory([](Hypodermic::ComponentContext&) -> std::shared_ptr<SettingsFactory::AbstractSettings> {
			return SettingsFactory::Create(QString("%1/%2.ini").arg(QCoreApplication::applicationDirPath()).arg(PRODUCT_ID));
		})
		.as<ISettings>()
		.singleInstance();

	container = builder.build();
}

}
