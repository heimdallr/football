#include "group.h"

#include <QPixmap>

using namespace HomeCompa::Football;
using namespace HomeCompa;

namespace
{

using Role  = ModelGroup::Role;
using Flags = std::unordered_map<int, QPixmap>;

struct Item
{
	int     id;
	QString group;
	int     num;
	QPixmap flag;
	QString name;
	int     place;

	std::vector<std::optional<std::pair<int, int>>> score;
};

using Items = std::vector<Item>;

QPixmap ToPixmap(const QByteArray& bytes)
{
	QPixmap pixmap;
	pixmap.loadFromData(bytes);
	return pixmap;
}

Flags GetFlags(const SqlDatabase& db, const int idChamp)
{
	Flags flags;

	auto query = db.CreateQuery("select id, flag from get_champ_country(?)");
	query.bindValue(0, idChamp);
	if (query.exec())
		while (query.next())
			flags.try_emplace(query.value(0).toInt(), ToPixmap(query.value(1).toByteArray()));

	return flags;
}

Items GetItems(const SqlDatabase& db, const int idChamp)
{
	struct CountryItem
	{
		QString name;
		QString group;
		int     place;
		int     num;

		std::map<int, std::optional<std::pair<int, int>>> score;
	};

	std::unordered_map<int, CountryItem> countryItems;

	auto query =
		db.CreateQuery("select id_country1, id_country2, country_name1, country_name2, group_name, place1, place2, num1, num2, goal1, goal2, ready from get_match_detail(?) where id_group is not null");
	query.bindValue(0, idChamp);

	const auto addItem = [&](const int shift) {
		auto& countryItem = countryItems[query.value(0 + shift).toInt()];
		if (countryItem.name.isEmpty())
		{
			countryItem.name  = query.value(2 + shift).toString();
			countryItem.group = query.value(4).toString();
			countryItem.place = query.value(5 + shift).toInt();
			countryItem.num   = query.value(7 + shift).toInt();
		}

		countryItem.score.try_emplace(query.value(8 - shift).toInt(), query.value(11).toInt() ? std::optional(std::make_pair(query.value(9 + shift).toInt(), query.value(10 - shift).toInt())) : std::nullopt);
	};

	if (query.exec())
		while (query.next())
		{
			addItem(0);
			addItem(1);
		}

	auto flags = GetFlags(db, idChamp);

	std::unordered_set<QString> groups;

	Items items;
	for (auto&& [id, r] : countryItems)
	{
		const auto it = flags.find(id);
		assert(it != flags.end());
		auto& item = items.emplace_back(id, std::move(r.group), r.num, std::move(it->second), r.name, r.place);
		r.score.emplace(item.num, std::nullopt);
		std::ranges::copy(r.score | std::views::values, std::back_inserter(item.score));
	}

	return items;
}

class Model final : public QAbstractTableModel
{
public:
	explicit Model(std::shared_ptr<SqlDatabase> db)
		: m_db { std::move(db) }
	{
	}

private: // QAbstractItemModel
	int columnCount(const QModelIndex&) const override
	{
		return 10;
	}

	int rowCount(const QModelIndex& parent) const override
	{
		return parent.isValid() ? 0 : static_cast<int>(m_items.size());
	}

	QVariant data(const QModelIndex& /*index*/, const int /*role*/) const override
	{
		return {};
	}

	bool setData(const QModelIndex& index, const QVariant& value, const int role) override
	{
		return index.isValid() ? SetData(index, value, role) : SetData(value, role);
	}

private:
	bool SetData(const QModelIndex& /*index*/, const QVariant& /*value*/, const int /*role*/)
	{
		return false;
	}

	bool SetData(const QVariant& value, const int role)
	{
		switch (role)
		{
			case Role::ChampId:
				m_items = GetItems(*m_db, value.toInt());
				return true;

			default:
				break;
		}

		return assert(false && "unexpected role"), false;
	}

private:
	PropagateConstPtr<SqlDatabase, std::shared_ptr> m_db;

	Items m_items;
};

} // namespace

ModelGroup::ModelGroup(std::shared_ptr<SqlDatabase> db, QObject* parent)
	: QIdentityProxyModel(parent)
	, m_sourceModel { std::unique_ptr<QAbstractItemModel> { std::make_unique<Model>(std::move(db)) } }
{
	QIdentityProxyModel::setSourceModel(m_sourceModel.get());
}

ModelGroup::~ModelGroup() = default;
