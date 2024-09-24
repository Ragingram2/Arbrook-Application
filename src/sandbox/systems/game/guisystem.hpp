#pragma once
#include <filesystem>
#include <tuple>
#include <rfl.hpp>
#include <rfl/json.hpp>

#include "core/utils/profiler.hpp"

#include "core/core.hpp"
#include "graphics/rendering.hpp"
#include "input/input.hpp"

#include "sandbox/components/camerasettings.hpp"

namespace rythe::game
{
	using namespace rythe::core::events;
	using namespace rythe::core;
	namespace fs = std::filesystem;
	namespace ast = rythe::core::assets;

	template<typename FieldType>
	bool DrawField(const char* label, int index, FieldType& field);

	template<typename T>
	struct remove_first_type { };

	template<typename T, typename... Ts>
	struct remove_first_type<std::tuple<T, Ts...>>
	{
		typedef std::tuple<Ts...> type;
	};

	struct GUI
	{
		inline static ecs::entity selected;
	};

	class GUISystem : public core::System<GUISystem, core::transform>
	{
	private:
		bool m_readPixel = false;
		bool m_isHoveringWindow = false;
		static gfx::framebuffer* mainFBO;
		static gfx::framebuffer* pickingFBO;
		static ImGuizmo::OPERATION currentGizmoOperation;
		static ImGuizmo::MODE currentGizmoMode;
	public:
		void setup();
		void update();
		void guiRender(core::transform, gfx::camera);

		void drawHeirarchy(ecs::entity_set heirarchy);
		//void lightEditor(core::ecs::entity);
		//void exampleCompEditor(core::ecs::entity);
		//void meshrendererEditor(core::ecs::entity);
		//void transformEditor(core::ecs::entity);

		void pushDisabledInspector();
		void popDisabledInspector();

		static void setModel(ast::asset_handle<gfx::model>);
		static void setMaterial(ast::asset_handle<gfx::material>);
		void createEmptyEntity();
		void createCubeEntity();
		void createSphereEntity();
		void drawGizmo(core::transform camTransf, gfx::camera camera, math::ivec2 dims);
		void doClick(key_input<inputmap::method::MOUSE_LEFT>& action);

		static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

		template<size_t... indices>
		void unrollFields(auto func, std::index_sequence<indices...>)
		{
			(func.template operator() < indices > (), ...);
		}

		template<typename Component>
		inline void componentEditor(core::ecs::entity ent)
		{
			Component& comp = ent.getComponent<Component>();
			const std::string& compName = ecs::Registry::componentNames[rsl::typeHash<Component>()];
			ImGui::PushID(std::format("Entity_{}##{}", compName.c_str(), ent->id).c_str());
			if constexpr (!std::is_same<Component, core::transform>::value)
			{
				ImGui::Checkbox("", &comp.enabled.get());
				ImGui::SameLine();
			}
			bool open = ImGui::TreeNodeEx(compName.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen);
			if (!ent->enabled)
				pushDisabledInspector();

			if (open)
			{
				if constexpr (std::is_same<Component, gfx::light>::value)
				{
					if (comp.type() == gfx::LightType::DIRECTIONAL)
					{
						const auto view = rfl::to_view(comp.dir_data);
						const auto fields = rfl::fields<gfx::dir_light_data>();
						if (ImGui::BeginTable("Component", 2))
						{
							unrollFields([&fields, &view]<size_t i>()
							{
								if constexpr (!rfl::internal::is_skip_v <decltype(*view.template get<i>())>)
								{
									ImGui::TableNextRow();
									ImGui::TableSetColumnIndex(0);
									DrawLabel(fields[i].name().c_str());
									ImGui::TableSetColumnIndex(1);
									DrawField(i, *view.template get<i>());
								}
							}, std::make_index_sequence<view.size()>{});
							ImGui::EndTable();
						}
					}
					else if (comp.type() == gfx::LightType::POINT)
					{
						const auto view = rfl::to_view(comp.point_data);
						const auto fields = rfl::fields<gfx::point_light_data>();
						if (ImGui::BeginTable("Component", 2))
						{
							unrollFields([&fields, &view]<size_t i>()
							{
								if constexpr (!rfl::internal::is_skip_v <decltype(*view.template get<i>())>)
								{
									ImGui::TableNextRow();
									ImGui::TableSetColumnIndex(0);
									DrawLabel(fields[i].name().c_str());
									ImGui::TableSetColumnIndex(1);
									DrawField(i, *view.template get<i>());
								}
							}, std::make_index_sequence<view.size()>{});
							ImGui::EndTable();
						}
					}
				}
				else
				{
					const auto view = rfl::to_view(comp);
					const auto fields = rfl::fields<Component>();
					if (ImGui::BeginTable("Component", 2))
					{
						unrollFields([&fields, &view]<size_t i>()
						{
							if constexpr (!rfl::internal::is_skip_v <decltype(*view.template get<i>())>)
							{
								ImGui::TableNextRow();
								ImGui::TableSetColumnIndex(0);
								DrawLabel(fields[i].name().c_str());
								ImGui::TableSetColumnIndex(1);
								DrawField(i, *view.template get<i>());
							}
						}, std::make_index_sequence<view.size()>{});
						ImGui::EndTable();
					}
				}
			}

			if (!ent->enabled)
				popDisabledInspector();
			ImGui::PopID();
		}

	};

	template<typename ItemType>
	inline void createAssetDropDown(ItemType current, std::vector<ast::asset_handle<ItemType>> items, void(*func)(ast::asset_handle<ItemType>))
	{
		if (ImGui::BeginCombo("Dropdown", current.name.c_str()))
		{
			for (auto item : items)
			{
				const bool is_selected = (item == current);
				if (ImGui::Selectable(item.getNameC(), is_selected))
				{
					current = *item.m_data;
					func(item);
				}

				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}

	inline bool DrawLabel(const char* label)
	{
		ImGui::Text(label);
		return true;
	}

	template<typename FieldType>
	inline bool DrawField(int index, FieldType& field)
	{
		return true;
	}

	template<>
	inline bool DrawField<math::vec2>(int index, math::vec2& field)
	{
		return ImGui::InputFloat2(std::format("##{}", index).c_str(), field.data);
	}

	template<>
	inline bool DrawField<math::vec3>(int index, math::vec3& field)
	{
		return ImGui::InputFloat3(std::format("##{}", index).c_str(), field.data);
	}

	template<>
	inline bool DrawField<math::vec4>(int index, math::vec4& field)
	{
		return ImGui::InputFloat4(std::format("##{}", index).c_str(), field.data);
	}

	template<>
	inline bool DrawField<math::quat>(int index, math::quat& field)
	{
		math::vec3 rot = math::toEuler(field);
		bool b = ImGui::InputFloat3(std::format("##{}", index).c_str(), rot.data);
		if (b) field = math::toQuat(rot);
		return b;
	}

	template<>
	inline bool DrawField<bool>(int index, bool& field)
	{
		return ImGui::Checkbox(std::format("##{}", index).c_str(), &field);
	}

	template<>
	inline bool DrawField<float>(int index, float& field)
	{
		return ImGui::InputFloat(std::format("##{}", index).c_str(), &field);
	}

	template<>
	inline bool DrawField<double>(int index, double& field)
	{
		return ImGui::InputDouble(std::format("##{}", index).c_str(), &field);
	}

	template<>
	inline bool DrawField<int>(int index, int& field)
	{
		return ImGui::InputInt(std::format("##{}", index).c_str(), &field);
	}

	template<>
	inline bool DrawField<gfx::model>(int index, gfx::model& field)
	{
		createAssetDropDown<gfx::model>(field, gfx::ModelCache::getModels(), &GUISystem::setModel);
		return true;
	}

	template<>
	inline bool DrawField<gfx::material>(int index, gfx::material& field)
	{
		createAssetDropDown<gfx::material>(field, gfx::MaterialCache::getMaterials(), &GUISystem::setMaterial);
		return true;
	}

	template<>
	inline bool DrawField<std::unordered_map<rsl::id_type, gfx::material>>(const char* label, int index, std::unordered_map<rsl::id_type, gfx::material>& field)
	{
		//if (ImGui::BeginTable("", 2))
		//{
		//	for (int row = 0; row < field.size(); row++)
		//	{
		//		ImGui::TableNextRow();
		//		for (int column = 0; column < 2; column++)
		//		{
		//			ImGui::TableSetColumnIndex(column);
		//			ImGui::Text("Row %d Column %d", row, column);
		//		}
		//	}
		//	ImGui::EndTable();
		//}
		return true;
	}
}