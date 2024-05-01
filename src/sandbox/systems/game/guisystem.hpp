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
		void onRender(core::transform, gfx::camera);
		void guiRender(core::transform, gfx::camera);

		void drawHeirarchy(ecs::entity_set heirarchy);
		void lightEditor(core::ecs::entity);
		//void exampleCompEditor(core::ecs::entity);
		void meshrendererEditor(core::ecs::entity);
		//void transformEditor(core::ecs::entity);

		void pushDisabledInspector();
		void popDisabledInspector();

		static void setModel(ast::asset_handle<gfx::model>);
		static void setMaterial(ast::asset_handle<gfx::material>);
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
			auto& comp = ent.getComponent<Component>();
			auto compName = ecs::Registry::componentNames[rsl::typeHash<Component>()];
			ImGui::PushID(std::format("Entity_{}##{}", compName.c_str(), ent->id).c_str());
			if constexpr (!std::is_same<Component, core::transform>::value)
			{
				ImGui::Checkbox("", &comp.enabled);
				ImGui::SameLine();
			}
			bool open = ImGui::TreeNodeEx(compName.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen);
			if (!ent->enabled)
				pushDisabledInspector();

			if (open)
			{
				const auto view = rfl::to_view(comp);
				const auto fields = rfl::fields<Component>();
				unrollFields([&fields, &view]<size_t i>() { DrawField(fields[i].name().c_str(), i, *rfl::get<i>(view)); }, std::make_index_sequence<view.size()>{});
			}

			if (!ent->enabled)
				popDisabledInspector();
			ImGui::PopID();
		}

	};

	template<typename ItemType>
	inline void createAssetDropDown(const char* label, ItemType current, std::vector<ast::asset_handle<ItemType>> items, void(*func)(ast::asset_handle<ItemType>))
	{
		if (ImGui::BeginCombo(label, current.name.c_str()))
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

	template<typename FieldType>
	inline bool DrawField(const char* label, int index, FieldType& field)
	{
		ImGui::Text(std::format("{}##{}", label, index).c_str());
		return true;
	}

	template<>
	inline bool DrawField<math::vec2>(const char* label, int index, math::vec2& field)
	{
		return ImGui::InputFloat2(std::format("{}##{}", label, index).c_str(), field.data);
	}

	template<>
	inline bool DrawField<math::vec3>(const char* label, int index, math::vec3& field)
	{
		return ImGui::InputFloat3(std::format("{}##{}", label, index).c_str(), field.data);
	}

	template<>
	inline bool DrawField<math::vec4>(const char* label, int index, math::vec4& field)
	{
		return ImGui::InputFloat4(std::format("{}##{}", label, index).c_str(), field.data);
	}

	template<>
	inline bool DrawField<math::quat>(const char* label, int index, math::quat& field)
	{
		math::vec3 rot = math::toEuler(field);
		bool b = ImGui::InputFloat3(std::format("{}##{}", label, index).c_str(), rot.data);
		if (b) field = math::toQuat(rot);
		return b;
	}

	template<>
	inline bool DrawField<bool>(const char* label, int index, bool& field)
	{
		return ImGui::Checkbox(std::format("{}##{}", label, index).c_str(), &field);
	}

	template<>
	inline bool DrawField<float>(const char* label, int index, float& field)
	{
		return ImGui::InputFloat(std::format("{}##{}", label, index).c_str(), &field);
	}

	template<>
	inline bool DrawField<double>(const char* label, int index, double& field)
	{
		return ImGui::InputDouble(std::format("{}##{}", label, index).c_str(), &field);
	}

	template<>
	inline bool DrawField<int>(const char* label, int index, int& field)
	{
		return ImGui::InputInt(std::format("{}##{}", label, index).c_str(), &field);
	}

	template<>
	inline bool DrawField<gfx::model>(const char* label, int index, gfx::model& field)
	{
		createAssetDropDown(label, field, gfx::ModelCache::getModels(), &GUISystem::setModel);
		return true;
	}

	template<>
	inline bool DrawField<gfx::material>(const char* label, int index, gfx::material& field)
	{
		createAssetDropDown(label, field, gfx::MaterialCache::getMaterials(), &GUISystem::setMaterial);
		return true;
	}


}