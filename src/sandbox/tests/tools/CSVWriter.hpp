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

			times.reserve(max*16);
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

						times.append(std::format("{},", ((float)list[i]) / 1000000.0f));
					}
				}
				times.append("\n");
			}

			times.shrink_to_fit();

			return times;
		}
	};
	using TestName = std::string;
	struct CSVWriter
	{
	public:
		static std::unordered_map<TestName, test_result> results;

		static void writeTime(const std::string& testName, APIType type, const std::string& propertyName, std::int64_t time)
		{
			if (type == None)
			{
				return;
			}
			results[testName].testTimes[type][propertyName].push_back(time);
		}

		static void printResults(const std::string& path)
		{
			for (auto& [name, result] : results)
			{
#if RenderingAPI == RenderingAPI_OGL
				std::filesystem::path testPath = std::format("{}{}_ogldata.csv", path, name);
#elif RenderingAPI == RenderingAPI_DX11
				std::filesystem::path testPath = std::format("{}{}_dx11data.csv", path, name);
#endif
				std::ofstream file;
				file.open(testPath);
				file << result.serialize() << std::endl;
				file.close();
			}

			results.clear();
		}
	};
	inline std::unordered_map<TestName, test_result> CSVWriter::results;

	struct FrameClock
	{
	public:
		std::string testName;
		APIType type;
		std::string propertyName;
		std::chrono::steady_clock::time_point start;
		std::chrono::steady_clock::time_point end;


		FrameClock(const std::string& _testName, APIType _type, const::std::string& _propertyName) : testName(_testName), type(_type), propertyName(_propertyName)
		{
			start = std::chrono::high_resolution_clock::now();
		}
		~FrameClock()
		{
			end = std::chrono::high_resolution_clock::now();
			CSVWriter::writeTime(testName, type, propertyName, std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
		}
	};
}