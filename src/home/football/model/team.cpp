#include "team.h"

#include <QColor>

#include "fnd/IsOneOf.h"
#include "fnd/ScopedCall.h"

#include "reader.h"

using namespace HomeCompa::Football;
using namespace HomeCompa;

namespace
{

using Role = ModelTeam::Role;

QVariant FromInt(const int value)
{
	constexpr auto white = 0x00FFFFFF;
	if (value == white)
		return {};

	const QRgb rgb(value);
	QColor     color;
	color.setRed(qBlue(rgb));
	color.setGreen(qGreen(rgb));
	color.setBlue(qRed(rgb));
	return color;
}

struct Item
{
	int      ordNum;
	int      number;
	QString  name;
	QString  type;
	int      champId;
	int      matchId;
	int      substituteMinute;
	QVariant goalCount;
	QString  goalMinute;
	int      cardColor;
	QDate    birthday;
	int      playerColor;

	QVariant Display(const int column) const
	{
		switch (column)
		{
			case 0:
				return number ? number : QVariant {};
			case 1:
				return name;
			case 2:
				return type;
			case 3:
				return goalCount;
			case 4:
				return goalMinute;
			default:
				break;
		}
		return {};
	}

	QVariant Color(const int column) const
	{
		switch (column)
		{
			case 0:
				return FromInt(cardColor);
			case 1:
				return FromInt(playerColor);
			default:
				break;
		}

		return {};
	}
};

using Items = std::vector<Item>;

class Model final : public QAbstractTableModel
{
public:
	Model(std::shared_ptr<QSqlDatabase> db)
		: m_db { std::move(db) }
	{
	}

private: // QAbstractTableModel
	int columnCount(const QModelIndex&) const override
	{
		return 5;
	}

	int rowCount(const QModelIndex& parent) const override
	{
		return parent.isValid() ? 0 : static_cast<int>(m_items.size());
	}

	QVariant data(const QModelIndex& index, const int role) const override
	{
		assert(index.isValid() && index.row() < rowCount({}));
		const auto& item = m_items[index.row()];

		switch (role)
		{
			case Qt::DisplayRole:
				return item.Display(index.column());

			case Qt::TextAlignmentRole:
				return QVariant::fromValue((index.column() == 0 ? Qt::AlignRight : IsOneOf(index.column(), 3, 4) ? Qt::AlignHCenter : Qt::AlignLeft) | Qt::AlignVCenter);

			case Qt::BackgroundRole:
				return item.Color(index.column());

			case Qt::ForegroundRole:
				if (const auto background = item.Color(index.column()); background.isValid())
					return QColor(Qt::black);

				return {};

			case Role::MatchId:
				return item.matchId;

			case Role::SubstituteMinute:
				return item.substituteMinute;

			default:
				break;
		}
		return {};
	}

	bool setData(const QModelIndex& index, const QVariant& value, const int role) override
	{
		return index.isValid() ? SetData(index, value, role) : SetData(value, role);
	}

private:
	bool SetData(const QModelIndex& index, const QVariant& value, const int role)
	{
		return QAbstractTableModel::setData(index, value, role);
	}

	bool SetData(const QVariant& value, const int role)
	{
		switch (role)
		{
			case Role::TeamId:
				return Reset(value.toInt()), true;
			default:
				break;
		}

		return assert(false && "unexpected role"), QAbstractTableModel::setData({}, value, role);
	}

	void Reset(const int teamId)
	{
		Items items;

		QSqlQuery query("select ORD_NUM, NUMBER, NAME, PLAYER_TYPE, ID_CHAMP_PLAYER, ID_MATCH_PLAYER, SUBST_MIN, GOAL_COUNT, GOAL_MINUTE, CARD_COLOR, BIRTHDAY, PLAYER_COLOR from GET_MATCH_PLAYER(?)", *m_db);
		query.bindValue(0, teamId);
		if (query.exec())
			while (query.next())
				items.emplace_back(ReadItem<Item>(query));

		std::ranges::sort(items, {}, [](const auto& item) {
			return item.ordNum;
		});

		const ScopedCall resetGuard(
			[this] {
				beginResetModel();
			},
			[this] {
				endResetModel();
			}
		);

		m_items = std::move(items);
	}

private:
	PropagateConstPtr<QSqlDatabase, std::shared_ptr> m_db;

	Items m_items;
};

} // namespace

ModelTeam::ModelTeam(std::shared_ptr<QSqlDatabase> db, QObject* parent)
	: QSortFilterProxyModel(parent)
	, m_sourceModel { std::unique_ptr<QAbstractItemModel> { std::make_unique<Model>(std::move(db)) } }
{
	QSortFilterProxyModel::setSourceModel(m_sourceModel.get());
}

ModelTeam::ModelTeam(QAbstractItemModel* sourceModel, QObject* parent)
	: QSortFilterProxyModel(parent)
	, m_isSubstitutes { true }
	, m_sourceModel { std::unique_ptr<QAbstractItemModel> {} }
{
	QSortFilterProxyModel::setSourceModel(sourceModel);
}

ModelTeam::~ModelTeam() = default;

bool ModelTeam::filterAcceptsRow(const int sourceRow, const QModelIndex& sourceParent) const
{
	const auto sourceIndex  = sourceModel()->index(sourceRow, 0, sourceParent);
	const auto isSubstitute = sourceIndex.data(Role::MatchId).toInt() == 0 || sourceIndex.data(Role::SubstituteMinute).toInt() != 0;
	return isSubstitute == m_isSubstitutes;
}
