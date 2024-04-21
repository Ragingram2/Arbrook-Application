#pragma once
#include <filesystem>

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
		void exampleCompEditor(core::ecs::entity);
		void meshrendererEditor(core::ecs::entity);
		void transformEditor(core::ecs::entity);

		void pushDisabledInspector();
		void popDisabledInspector();

		static void setModel(ast::asset_handle<gfx::model>, ecs::entity);
		static void setMaterial(ast::asset_handle<gfx::material>, ecs::entity);
		void drawGizmo(core::transform camTransf, gfx::camera camera, math::ivec2 dims);
		void doClick(key_input<inputmap::method::MOUSE_LEFT>& action);

		static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

		template<typename ItemType>
		inline void createAssetDropDown(ecs::entity ent, const char* label, ItemType current, std::vector<ast::asset_handle<ItemType>> items, void(*func)(ast::asset_handle<ItemType>, ecs::entity))
		{
			using namespace ImGui;
			if (BeginCombo(label, current.name.c_str()))
			{
				for (auto item : items)
				{
					const bool is_selected = (item == current);
					if (Selectable(item.getNameC(), is_selected))
					{
						current = *item.m_data;
						func(item, ent);
					}

					if (is_selected)
					{
						SetItemDefaultFocus();
					}
				}
				EndCombo();
			}
		}
	};
}