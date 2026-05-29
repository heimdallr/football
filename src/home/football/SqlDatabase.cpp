#include "SqlDatabase.h"

#include <QSqlDriver>

using namespace HomeCompa::Football;

class SqlDatabase::Impl
{
public:
	explicit Impl(const ISettings& settings)
		: m_db { QSqlDatabase::addDatabase("QIBASE") }
	{
		if (!m_db.isValid())
			throw std::runtime_error("cannot create QIBASE plugin");

		SettingsGroup connection(settings, "Connection");
		m_db.setHostName(settings.Get("Host").toString());
		if (const auto port = settings.Get("Port"); port.isValid())
			m_db.setPort(port.toInt());

		m_db.setDatabaseName(settings.Get("DatabaseName").toString());
		if (!m_db.open(settings.Get("User").toString(), settings.Get("Password").toString()))
			throw std::runtime_error("cannot connect to database");

		QObject::connect(m_db.driver(), &QSqlDriver::notification, [this](const QString& name) {
			const auto callbacks = m_subscriptions | std::views::values | std::views::filter([&](const auto& item) {
									   return item.first == name;
								   })
			                     | std::views::values | std::ranges::to<std::vector>();
			for (auto&& callback : callbacks)
				callback();
		});
	}

	SubscriptionWrapper::Ptr Subscribe(QString eventName, std::function<void()> callback)
	{
		if (const auto it = m_subscriptionCount.find(eventName); it != m_subscriptionCount.end())
			++it->second;
		else
		{
			m_subscriptionCount.try_emplace(eventName, 1);
			m_db.driver()->subscribeToNotification(eventName);
		}

		const auto id = m_subscriptions.try_emplace(++m_subscribeId, std::make_pair(std::move(eventName), std::move(callback))).first->first;
		return std::make_unique<SubscriptionWrapper>(*this, id);
	}

	void UnSubscribe(const size_t id)
	{
		const auto it = m_subscriptions.find(id);
		assert(it != m_subscriptions.end());

		const auto countIt = m_subscriptionCount.find(it->second.first);
		assert(countIt != m_subscriptionCount.end());
		if (--countIt->second == 0)
		{
			m_db.driver()->unsubscribeFromNotification(it->second.first);
			m_subscriptionCount.erase(countIt);
		}

		m_subscriptions.erase(it);
	}

	QSqlQuery CreateQuery(const QString& text) const
	{
		QSqlQuery query(text, m_db);
		return query;
	}

	void StartTransaction()
	{
		m_db.transaction();
	}

	void Commit()
	{
		m_db.commit();
	}

private:
	QSqlDatabase m_db;

	size_t                                                      m_subscribeId { 0 };
	std::map<size_t, std::pair<QString, std::function<void()>>> m_subscriptions;
	std::unordered_map<QString, size_t>                         m_subscriptionCount;
};

SqlDatabase::TransactionWrapper::TransactionWrapper(Impl& impl)
	: m_impl { impl }
{
	m_impl.StartTransaction();
}

SqlDatabase::TransactionWrapper::~TransactionWrapper()
{
	m_impl.Commit();
}

SqlDatabase::SubscriptionWrapper::SubscriptionWrapper(Impl& impl, const size_t id)
	: m_impl { impl }
	, m_id { id }
{
}

SqlDatabase::SubscriptionWrapper::~SubscriptionWrapper()
{
	m_impl.UnSubscribe(m_id);
}

SqlDatabase::SqlDatabase(const std::shared_ptr<const ISettings>& settings)
	: m_impl(*settings)
{
}

SqlDatabase::~SqlDatabase() = default;

QSqlQuery SqlDatabase::CreateQuery(const QString& text) const
{
	return m_impl->CreateQuery(text);
}

SqlDatabase::TransactionWrapper SqlDatabase::StartTransaction()
{
	return TransactionWrapper(*m_impl);
}

SqlDatabase::SubscriptionWrapper::Ptr SqlDatabase::Subscribe(QString eventName, std::function<void()> callback)
{
	return m_impl->Subscribe(std::move(eventName), std::move(callback));
}
