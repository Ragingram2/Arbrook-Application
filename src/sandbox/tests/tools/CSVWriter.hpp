#pragma once
#include <chrono>
#include <ctime>
#include <memory>
#include <iostream>
#include <filesystem>

#include <rsl/logging>
#include <rsl/math>

#include "graphics/interface/definitions/definitions.hpp"
#include "sandbox/tests/rendertest.hpp"

namespace rythe::testing
{
	struct result_times
	{
		std::int64_t setupTime;
		std::vector<std::int64_t> frameTimes;
		std::unordered_map<std::string, std::vector<std::int64_t>> times;
	};

	struct test_result
	{
		std::unordered_map<APIType, std::unordered_map<std::string, std::vector<std::int64_t>>> testTimes;

		std::string serialize()
		{
			std::string times;

			for (auto& [t, propertyList] : testTimes)
			{
				if (t == APIType::None)
					continue;
				times.append(std::format("{},", getAPIName(t)));
				for (auto& [name, list] : propertyList)
				{
					times.append(std::format("{},", name));
				}
			}

			size_t max = 0;
			times.append("\n");

			//Looks for the largest list among the properties
			for (auto& [t, propertyList] : testTimes)
			{
				for (auto& [name, list] : propertyList)
				{
					if (propertyList[name].size() >= max)
					{
						max = propertyList[name].size();
					}
				}
			}

			for (size_t i = 0; i < max; i++)
			{
				for (auto& [t, propertyList] : testTimes)
				{
					if (t == APIType::None)
						continue;
					times.append(" ,");
					for (auto [name, list] : propertyList)
					{
						if (i >= list.size()) 
						{ 
							times.append(" ,"); 
							continue;
						}

						times.append(std::format("{},", ((float)list[i])/1000000.0f));
					}
				}
				times.append("\n");
			}

			return times;
		}
	};

	struct CSVWriter
	{
	private:
		std::string fullPath;
		std::string filePath;
		std::string fileName;

		std::string output;
	public:
		using TestName = std::string;
		std::unordered_map<TestName, test_result> results;

		CSVWriter(std::string path)
		{
			fullPath = path;
			auto idx = path.find_last_of('/');
			fileName = path.substr(idx + 1, path.size());
			filePath = path.substr(0, idx);
		}

		//void writeSetupTime(std::string testName, APIType type, std::int64_t setupTime)
		//{
		//	if (type == None)
		//		return;
		//	results[testName].testTimes[type]["Setup"].push_back(setupTime);
		//}

		//void writeFrameTime(std::string testName, APIType type, std::int64_t frameTime)
		//{
		//	if (type == None)
		//		return;
		//	results[testName].testTimes[type]["FrameTime"].push_back(frameTime);
		//}

		void writeTime(std::string testName, APIType type, std::string propertyName, std::int64_t time)
		{
			if (type == None)
				return;
			results[testName].testTimes[type][propertyName].push_back(time);
		}

		void printResults()
		{
			for (auto& [name, result] : results)
			{
				std::filesystem::path testPath = std::filesystem::path(fullPath);
				std::ofstream file;
				file.open(testPath);
				file << result.serialize() << std::endl;
				file.close();
			}

			results.clear();
		}
	};
}