#pragma once

#include "simdjson.h"

class UserInfo
{
public:
	struct Project {
		std::string name;
		bool completed;
	};

	struct Team {
		std::string name;
		bool leader;
		std::vector<Project> projects;
	};

	struct Log {
		std::string date;
		std::string action;
	};

	struct User {
		std::string id;
		std::string name;
		int age;
		int score;
		bool active;
		std::string country;
		Team team;
		std::vector<Log> logs;
	};
};


namespace simdjson
{
    template <typename simdjson_value>
    auto tag_invoke(deserialize_tag, simdjson_value& val, UserInfo::Project& project)
    {
        ondemand::object obj;
        auto error = val.get_object().get(obj);
        if (error) { return error; }
        if (error = obj["name"].get_string(project.name)) { return error; }
        if (error = obj["completed"].get(project.completed)) { return error; }
        return simdjson::SUCCESS;
    }

    template <typename simdjson_value>
    auto tag_invoke(deserialize_tag, simdjson_value& val, UserInfo::Team& team)
    {
        ondemand::object obj;
        auto error = val.get_object().get(obj);
        if (error) { return error; }
        if (error = obj["name"].get_string(team.name)) { return error; }
        if (error = obj["leader"].get(team.leader)) { return error; }
        if (error = obj["projects"].get<std::vector<UserInfo::Project>>().get(team.projects)) { return error; }
        return simdjson::SUCCESS;
    }

    template <typename simdjson_value>
    auto tag_invoke(deserialize_tag, simdjson_value& val, UserInfo::Log& log)
    {
        ondemand::object obj;
        auto error = val.get_object().get(obj);
        if (error) { return error; }
        if (error = obj["date"].get_string(log.date)) { return error; }
        if (error = obj["action"].get_string(log.action)) { return error; }
        return simdjson::SUCCESS;
    }

    template <typename simdjson_value>
    auto tag_invoke(deserialize_tag, simdjson_value& val, UserInfo::User& user)
    {
        ondemand::object obj;
        auto error = val.get_object().get(obj);
        if (error) { return error; }
        if (error = obj["id"].get_string(user.id)) { return error; }
        if (error = obj["name"].get_string(user.name)) { return error; }
        if (error = obj["age"].get(user.age)) { return error; }
        if (error = obj["score"].get(user.score)) { return error; }
        if (error = obj["active"].get(user.active)) { return error; }
        if (error = obj["country"].get_string(user.country)) { return error; }
        if (error = obj["team"].get(user.team)) { return error; }
        if (error = obj["logs"].get<std::vector<UserInfo::Log>>().get(user.logs)) { return error; }
        return simdjson::SUCCESS;
    }
}