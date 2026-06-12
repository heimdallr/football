#include "group.h"

#include <QFont>
#include <QPixmap>

#include "fnd/IsOneOf.h"
#include "fnd/ScopedCall.h"

using namespace HomeCompa::Football;
using namespace HomeCompa;

namespace
{

using Role  = ModelGroup::Role;
using Flags = std::unordered_map<int, QPixmap>;

struct Item
{
	int     id { -1 };
	QString group;
	int     num { -1 };
	QPixmap flag;
	QString name;
	int     place { -1 };

	std::vector<std::optional<std::pair<int, int>>> score;

	QVariant Display(int column) const
	{
		const auto count = [this](const auto& f) {
			return std::ranges::count_if(
				score | std::views::filter([](const auto& item) {
					return !!item;
				}),
				[f](const auto& item) {
					return f(*item);
				}
			);
		};

		switch (column)
		{
			case 0:
				return num == 1 ? group : QVariant {};
			case 1:
				return num;
			case 2:
				return {};
			case 3:
				return name;
			case 4:
				return count([](const auto&) {
					return true;
				});
			default:
				break;
		}
		column -= 5;

		if (column < static_cast<int>(score.size()))
			return score[column] ? QString("%1 : %2").arg(score[column]->first).arg(score[column]->second) : QVariant {};

		column -= static_cast<int>(score.size());

		switch (column)
		{
			case 0:
				return count([](const auto& item) {
					return item.first > item.second;
				});

			case 1:
				return count([](const auto& item) {
					return item.first == item.second;
				});

			case 2:
				return count([](const auto& item) {
					return item.first < item.second;
				});

			case 3:
			{
				const auto sum = std::accumulate(score.cbegin(), score.cend(), std::make_pair(0, 0), [](const auto& init, const auto& item) {
					return item ? std::make_pair(init.first + item->first, init.second + item->second) : init;
				});
				return QString("%1 : %2").arg(sum.first).arg(sum.second);
			}

			case 4:
				return 3 * count([](const auto& item) {
						   return item.first > item.second;
					   })
				     + count([](const auto& item) {
						   return item.first == item.second;
					   });
			case 5:
				return place ? QVariant { place } : QVariant {};

			default:
				break;
		}

		return assert(false && "unexpected column"), QVariant {};
	}

	QVariant Background(const int column) const
	{
		return column - 4 == num ? QColor(Qt::gray) : QVariant {};
	}

	QVariant Font(const int column) const
	{
		const auto c = static_cast<int>(score.size());
		if (!IsOneOf(column, 0, 3, 9 + c, 10 + c))
			return {};

		QFont font;
		font.setBold(true);
		return font;
	}
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

Items GetItems(const SqlDatabase& db, const int idChamp, int& groupCount)
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

	std::set<QString> groups;

	Items items;
	for (auto&& [id, r] : countryItems)
	{
		auto it = flags.find(id);
		assert(it != flags.end());
		auto& item = items.emplace_back(id, std::move(r.group), r.num, std::move(it->second), r.name, r.place);
		r.score.emplace(item.num, std::nullopt);
		std::ranges::copy(r.score | std::views::values, std::back_inserter(item.score));
		groups.insert(item.group);
	}

	std::ranges::transform(groups | std::views::take(groups.size() - 1), std::back_inserter(items), [](const QString& group) {
		return Item { .group = group, .num = std::numeric_limits<int>::max() };
	});

	std::ranges::sort(items, {}, [](const Item& item) {
		return std::make_tuple(item.group, item.num);
	});

	groupCount = static_cast<int>(groups.size());
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
	QVariant headerData(int section, const Qt::Orientation orientation, const int role) const override
	{
		if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
			return QAbstractTableModel::headerData(section, orientation, role);

		static constexpr const char* headers[] = { "", "#", "F", "Team", "G", "W", "D", "L", "Score", "P", "R" };
		if (section < 5)
			return headers[section];

		if (m_items.empty())
			return QAbstractTableModel::headerData(section, orientation, role);

		section -= 5;
		if (section < static_cast<int>(m_items.front().score.size()))
			return section + 1;

		section -= static_cast<int>(m_items.front().score.size());
		section += 5;
		return headers[section];
	}

	int columnCount(const QModelIndex&) const override
	{
		return 11 + (m_items.empty() ? 0 : static_cast<int>(m_items.front().score.size()));
	}

	int rowCount(const QModelIndex& parent) const override
	{
		return parent.isValid() ? 0 : static_cast<int>(m_items.size());
	}

	QVariant data(const QModelIndex& index, const int role) const override
	{
		return index.isValid() ? GetData(index, role) : GetData(role);
	}

	bool setData(const QModelIndex& index, const QVariant& value, const int role) override
	{
		return index.isValid() ? SetData(index, value, role) : SetData(value, role);
	}

	Qt::ItemFlags flags(const QModelIndex& index) const override
	{
		Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

		if (index.column() == 0)
			return defaultFlags & ~Qt::ItemIsSelectable;

		return defaultFlags;
	}

private:
	QVariant GetData(const QModelIndex& index, const int role) const
	{
		assert(index.isValid() && index.row() < rowCount({}));
		const auto& item = m_items[index.row()];
		if (item.id < 0)
			return {};

		switch (role)
		{
			case Qt::DisplayRole:
			case Qt::ToolTipRole:
				return item.Display(index.column());

			case Qt::DecorationRole:
				return index.column() == 2 ? item.flag : QVariant {};

			case Qt::BackgroundRole:
				return item.Background(index.column());

			case Qt::TextAlignmentRole:
				return Qt::AlignCenter;

			case Qt::FontRole:
				return item.Font(index.column());

			default:
				break;
		}

		return {};
	}

	QVariant GetData(const int role) const
	{
		switch (role)
		{
			case Role::GroupSize:
				return m_items.empty() ? 0 : static_cast<int>(m_items.front().score.size());

			case Role::GroupCount:
				return m_groupCount;

			default:
				break;
		}

		return assert(false && "unexpected role"), QVariant {};
	}

	bool SetData(const QModelIndex& /*index*/, const QVariant& /*value*/, const int /*role*/)
	{
		return false;
	}

	bool SetData(const QVariant& value, const int role)
	{
		switch (role)
		{
			case Role::ChampId:
				return Reset(value.toInt()), true;

			default:
				break;
		}

		return assert(false && "unexpected role"), false;
	}

	void Reset(const int idChamp)
	{
		const ScopedCall resetGuard(
			[this] {
				beginResetModel();
			},
			[this] {
				endResetModel();
			}
		);
		m_items = GetItems(*m_db, idChamp, m_groupCount);
	}

private:
	PropagateConstPtr<SqlDatabase, std::shared_ptr> m_db;

	Items m_items;
	int   m_groupCount { 0 };
};

} // namespace

ModelGroup::ModelGroup(std::shared_ptr<SqlDatabase> db, QObject* parent)
	: QIdentityProxyModel(parent)
	, m_sourceModel { std::unique_ptr<QAbstractItemModel> { std::make_unique<Model>(std::move(db)) } }
{
	QIdentityProxyModel::setSourceModel(m_sourceModel.get());
}

ModelGroup::~ModelGroup() = default;
