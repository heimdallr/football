__pragma(warning(push, 0))
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

#include "fmt/core.h"
__pragma(warning(pop))


struct Player
{
public:
    using List = std::set<Player>;

public:
    int Id;
    std::string Country, Type, Name, Birthday;
    Player(int id, const std::filesystem::path & path, std::string type, std::string && name, std::string && birthday)
	    : Id(id)
		, Country(path.filename().string())
	    , Type(std::move(type))
		, Name(std::move(name))
		, Birthday(std::move(birthday))
    {
    }

    Player(const Player &) = delete;
    Player(Player &&) = delete;
    Player & operator=(const Player &) = delete;
    Player & operator=(Player && rhs) = delete;
	bool operator<(const Player & rhs) const noexcept
	{
        return Id < rhs.Id;
	}

    std::ostream & Write(std::ostream & stream) const
    {
        return stream << fmt::format("insert into t_player_buf(country, player_type, num, name, birthday) values('{0}', '{1}', {2}, '{3}', '{4}');\n", Country, Type, Id, Name, Birthday);
    }
};

std::ostream & operator<<(std::ostream & stream, const Player & player)
{
    return player.Write(stream);
}

bool GetLine(std::istream & stream, std::string & buf)
{
    std::string s;

	while (s.empty())
	{
        if (!std::getline(stream, s))
            return false;

        s.erase(std::ranges::find(s, '\t'), std::end(s));

        s.erase(std::find_if(s.rbegin(), s.rend(), [] (const char ch)
        {
            return true
                && ch != ' '
                && ch != '\n'
                && ch != '\r'
                && ch != '\t'
                ;
        }).base(), s.end());

        if (s == "-")
            s.clear();

		buf.clear();
        for (const auto c : s)
        {
            buf.push_back(c);
            if (c == '\'')
                buf.push_back(c);
        }
    }
	
    return true;
}

int main(int argc, char * argv[])
{
    try
    {
        if (argc < 2)
            throw std::invalid_argument("usage:\nPlayerParser path_to_folder_with_players");

        for (const auto & entry : std::filesystem::directory_iterator(argv[1]))
        {
            Player::List players;
            std::string type;
            std::ifstream inp(entry.path());
            std::string buf;
            while (GetLine(inp, buf))
            {            	
                std::string name, birthday;
            	if (!std::ranges::all_of(std::as_const(buf), [](const char c) { return c >= '0' && c <= '9'; }))
            	{
                    type = std::move(buf);
                    continue;
            	}
                const auto id = std::stoi(buf);
                GetLine(inp, name);
                GetLine(inp, birthday);
                GetLine(inp, buf); // клуб
            	
                players.emplace(id, entry.path(), type, std::move(name), std::move(birthday));
            }

            for (const auto & player : players)
                std::cout << player;
        }    	
    }
    catch (const std::exception & ex)
    {
        std::cerr << ex.what();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
