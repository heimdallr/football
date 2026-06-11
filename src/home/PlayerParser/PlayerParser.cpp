
#include <iostream>
#include <set>
#include <stdexcept>

#include <QDir>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>

namespace
{

constexpr const char* MONTHS[] { "января", "февраля", "марта", "апреля", "мая", "июня", "июля", "августа", "сентября", "октября", "ноября", "декабря" };

constexpr std::pair<const char*, const char*> TYPES[] {
	{		"Вратари",      "Вратарь" },
	{	  "Защитники",     "Защитник" },
	{  "Полузащитники", "Полузащитник" },
	{     "Нападающие",   "Нападающий" },
	{ "Главный тренер",       "Тренер" }
};

QString ParseBirthday(const QString& str)
{
	static QRegularExpression rx("^(\\d{1,2}) (.*?) (\\d{4})$");
	const auto                match = rx.match(str);
	assert(match.hasMatch());
	const auto it = std::ranges::find(MONTHS, match.captured(2));
	assert(it != std::end(MONTHS));
	auto result = QString("%1.%2.%3").arg(match.captured(3)).arg(std::distance(std::begin(MONTHS), it) + 1, 2, 10, QChar { '0' }).arg(match.captured(1).toInt(), 2, 10, QChar { '0' });
	return result;
}

struct Player
{
public:
	using List = std::set<Player>;

public:
	int     id;
	QString country, type, name, birthday;

	bool operator<(const Player& rhs) const noexcept
	{
		return id < rhs.id;
	}

	std::ostream& Write(std::ostream& stream) const
	{
		return stream << QString("insert into t_player_buf(country, player_type, num, name, birthday) values('%1', '%2', %3, '%4', '%5');\n").arg(country, type).arg(id).arg(name, birthday).toStdString();
	}
};

std::ostream& operator<<(std::ostream& stream, const Player& player)
{
	return player.Write(stream);
}

Player GetPlayer(QString country, QString type, const QRegularExpressionMatch& match)
{
	return { .id = match.captured(1).toInt(), .country = std::move(country), .type = std::move(type), .name = match.captured(2), .birthday = ParseBirthday(match.captured(3)) };
}

Player GetCoach(QString country, QString type, const QRegularExpressionMatch& match)
{
	return { .id = -1, .country = std::move(country), .type = std::move(type), .name = match.captured(1), .birthday = ParseBirthday(match.captured(2)) };
}

using Parser = Player (*)(QString, QString, const QRegularExpressionMatch&);

} // namespace

int main(const int argc, char* argv[])
{
	try
	{
		if (argc < 2) [[unlikely]]
			throw std::invalid_argument("usage:\nPlayerParser path_to_folder_with_players");

		QDir srcDir(argv[1]);
		if (!srcDir.exists()) [[unlikely]]
			throw std::invalid_argument(std::format("{} not exists", argv[1]));

		const std::pair<QRegularExpression, Parser> parsers[] {
			{   QRegularExpression("^(\\d*?)\t(.*?)\t.*?\t(.*?)\t.*?$"), &GetPlayer },
			{ QRegularExpression(QString("^\\D*?\t(.*?)\t(.*?)\t.*?$")),  &GetCoach },
		};

		for (const auto& fileName : srcDir.entryList({}, QDir::Files))
		{
			QFile file(srcDir.filePath(fileName));
			if (!file.open(QIODevice::ReadOnly)) [[unlikely]]
				throw std::invalid_argument(std::format("Cannot read {}", fileName.toStdString()));

			Player::List players;
			QString      type;
			QTextStream  stream(&file);
			while (true)
			{
				const auto str = stream.readLine();
				if (str.isEmpty())
					break;

				if (const auto it = std::ranges::find(TYPES, str, &std::pair<const char*, const char*>::first); it != std::end(TYPES))
				{
					type = it->second;
					continue;
				}

				for (const auto& [rx, parser] : parsers)
					if (const auto match = rx.match(str); match.hasMatch())
					{
						players.emplace(std::invoke(parser, fileName, type, match));
						break;
					}
			}

			for (const auto& player : players)
				std::cout << player;
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
