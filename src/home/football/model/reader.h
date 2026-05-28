#pragma once

#include <QDateTime>
#include <QSqlQuery>
#include <QString>
#include <QVariant>

#include "fnd/ToTuple.h"

namespace HomeCompa::Football
{

namespace details
{

template <typename T>
void Read(T& /*dst*/, const QVariant& /*src*/) = delete;

template <>
inline void Read<int>(int& dst, const QVariant& src)
{
	dst = src.toInt();
}

template <>
inline void Read<QString>(QString& dst, const QVariant& src)
{
	dst = src.toString();
}

template <>
inline void Read<QVariant>(QVariant& dst, const QVariant& src)
{
	dst = src;
}

template <>
inline void Read<QDateTime>(QDateTime& dst, const QVariant& src)
{
	dst = src.toDateTime();
}

template <>
inline void Read<QDate>(QDate& dst, const QVariant& src)
{
	dst = src.toDate();
}

} // namespace details

template <size_t I = 0, typename... Tp>
void ReadTuple(const QSqlQuery& query, std::tuple<Tp...>& t)
{
	details::Read(std::get<I>(t), query.value(static_cast<int>(I)));
	if constexpr (I + 1 != sizeof...(Tp))
		ReadTuple<I + 1>(query, t);
}

template <typename Item>
Item ReadItem(const QSqlQuery& query)
{
	Item item;
	auto tuple = Util::ToTupleRef(item);
	ReadTuple(query, tuple);
	return item;
}

} // namespace HomeCompa::Football
