#pragma once
#include <QSqlQuery>

#include "fnd/memory.h"

#include "settings/ISettings.h"

namespace HomeCompa::Football
{

class SqlDatabase
{
	NON_COPY_MOVABLE(SqlDatabase)

	class Impl;

public:
	class TransactionWrapper
	{
		NON_COPY_MOVABLE(TransactionWrapper)

	public:
		explicit TransactionWrapper(Impl& impl);
		~TransactionWrapper();

	private:
		Impl& m_impl;
	};

	class SubscriptionWrapper
	{
		NON_COPY_MOVABLE(SubscriptionWrapper)

	public:
		using Ptr = std::unique_ptr<SubscriptionWrapper>;

	public:
		explicit SubscriptionWrapper(Impl& impl, size_t id);
		~SubscriptionWrapper();

	private:
		Impl&  m_impl;
		size_t m_id;
	};

public:
	explicit SqlDatabase(const std::shared_ptr<const ISettings>& settings);
	~SqlDatabase();

public:
	[[nodiscard]] TransactionWrapper       StartTransaction();
	[[nodiscard]] SubscriptionWrapper::Ptr Subscribe(QString eventName, std::function<void()> callback);
	[[nodiscard]] QSqlQuery                CreateQuery(const QString& text) const;

private:
	PropagateConstPtr<Impl> m_impl;
};

} // namespace HomeCompa::Football
