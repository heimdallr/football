#include "champ.h"

#include <QColor>
#include <QCoreApplication>

#include "fnd/IsOneOf.h"
#include "fnd/ScopedCall.h"

#include "SettingsConstant.h"
#include "SqlDatabase.h"
#include "reader.h"

using namespace HomeCompa;
using namespace HomeCompa::Football;

namespace
{

using Role = ModelChamp::Role;

constexpr auto CONTEXT = "ChampModel";

constexpr const char* HEADERS[] {
	QT_TRANSLATE_NOOP("ChampModel", "Date, Time"), QT_TRANSLATE_NOOP("ChampModel", "Stage"), QT_TRANSLATE_NOOP("ChampModel", "Group"),
	QT_TRANSLATE_NOOP("ChampModel", "Teams"),      QT_TRANSLATE_NOOP("ChampModel", "Score"), QT_TRANSLATE_NOOP("ChampModel", "Stadium. City"),
};

struct Item
{
	int       id;
	QString   city;
	QDateTime dateTime;
	int       ordNum;
	int       idStatus;
	int       ranking;
	QString   status;
	QString   goalCount;
	int       idTeam1, idTeam2;
	QString   countries;
	int       allExists;
	int       idGroup;
	QString   groupName;

	QVariant Display(const int column) const
	{
		switch (column)
		{
			case 0:
				return dateTime.toString("dd.MM hh:mm");
			case 1:
				return status;
			case 2:
				return groupName;
			case 3:
				return countries;
			case 4:
				return goalCount;
			case 5:
				return city;
			default:
				break;
		}
		return {};
	}
};

using Items = std::vector<Item>;

Items ReadItems(const ISettings& settings, const SqlDatabase& db)
{
	Items items;
	auto  query = db.CreateQuery("select ID, CITY_NAME, PLAY_AT, ORD_NUM, ID_STATUS, RANKING, STATUS, GOAL_COUNT, ID_C1, ID_C2, COUNTRIES, ALL_EXISTS, ID_GROUP, GROUP_NAME from GET_MATCH(?)");
	query.bindValue(0, settings.Get(Constant::CHAMP_ID_KEY, -1));
	if (query.exec())
		while (query.next())
			items.emplace_back(ReadItem<Item>(query));

	std::ranges::sort(items, {}, [](const auto& item) {
		return item.ordNum;
	});

	return items;
}

class Model final : public QAbstractTableModel
{
public:
	Model(std::shared_ptr<const ISettings> settings, std::shared_ptr<SqlDatabase> db)
		: m_settings { std::move(settings) }
		, m_db { std::move(db) }
		, m_items { ReadItems(*m_settings, *m_db) }
		, m_subscription { m_db->Subscribe("match", [this] {
			Reset();
		}) }
	{
	}

private: // QAbstractTableModel
	QVariant headerData(const int section, const Qt::Orientation orientation, const int role) const override
	{
		return role != Qt::DisplayRole       ? QAbstractTableModel::headerData(section, orientation, role)
		     : orientation == Qt::Horizontal ? QVariant::fromValue(QCoreApplication::translate(CONTEXT, HEADERS[section]))
		                                     : m_items[static_cast<size_t>(section)].ordNum;
	}

	int columnCount(const QModelIndex&) const override
	{
		return static_cast<int>(std::size(HEADERS));
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
			case Qt::ToolTipRole:
				return item.Display(index.column());

			case Qt::BackgroundRole:
				return IsNext(index.row()) ? QColor(Qt::darkGreen) : QVariant {};

			case Qt::TextAlignmentRole:
				return QVariant::fromValue((IsOneOf(index.column(), 0, 5) ? Qt::AlignLeft : Qt::AlignHCenter) | Qt::AlignVCenter);

			case Role::TeamIds:
				return QVariant::fromValue(std::make_pair(item.idTeam1, item.idTeam2));

			case Role::MatchId:
				return item.id;

			default:
				break;
		}
		return {};
	}

	bool setData(const QModelIndex& index, const QVariant& value, const int role) override
	{
		if (role == Role::Reset)
			return Reset(), true;

		assert(index.isValid() && index.row() < rowCount({}));
		const auto& item = m_items[index.row()];
		switch (role)
		{
			case Role::SwitchMatchEndFlag:
				return SwitchMatchEndFlag(item.id), true;

			default:
				break;
		}

		return QAbstractTableModel::setData(index, value, role);
	}

	Qt::ItemFlags flags(const QModelIndex& index) const override
	{
		Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

		if (index.column() == 1)
			return defaultFlags & ~Qt::ItemIsSelectable;

		return defaultFlags;
	}

private:
	void Reset()
	{
		const ScopedCall resetGuard(
			[this] {
				beginResetModel();
			},
			[this] {
				endResetModel();
			}
		);
		m_items = ReadItems(*m_settings, *m_db);
	}

	bool IsNext(const int row) const
	{
		const auto now = QDateTime::currentDateTime();
		return m_items[row].dateTime.addSecs(120 * 60) > now && (row == 0 || m_items[row - 1].dateTime.addSecs(120 * 60) < now);
	}

	void SwitchMatchEndFlag(const int id)
	{
		const auto tr = m_db->StartTransaction();

		QSqlQuery query("execute procedure SWITCH_MATCH_READY(?)");
		query.bindValue(0, id);
		query.exec();
	}

private:
	std::shared_ptr<const ISettings>                m_settings;
	PropagateConstPtr<SqlDatabase, std::shared_ptr> m_db;

	Items m_items;

	SqlDatabase::SubscriptionWrapper::Ptr m_subscription;
};

} // namespace

ModelChamp::ModelChamp(std::shared_ptr<ISettings> settings, std::shared_ptr<SqlDatabase> db, QObject* parent)
	: QIdentityProxyModel(parent)
	, m_sourceModel { std::unique_ptr<QAbstractItemModel> { std::make_unique<Model>(std::move(settings), std::move(db)) } }
{
	QIdentityProxyModel::setSourceModel(m_sourceModel.get());
}

ModelChamp::~ModelChamp() = default;
