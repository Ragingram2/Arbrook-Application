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
	public:
		void setup();
		void update();
		void onRender(core::transform, gfx::camera);
		void guiRender(core::transform, gfx::camera);

		void lightEditor(core::ecs::entity);
		void exampleCompEditor(core::ecs::entity);
		void meshrendererEditor(core::ecs::entity);
		void transformEditor(core::ecs::entity);

		static void setModel(ast::asset_handle<gfx::model>, ecs::entity);
		static void setMaterial(ast::asset_handle<gfx::material>, ecs::entity);
		void readPixel(key_input<inputmap::method::MOUSE_LEFT>& action);

		template<typename ItemType>
		inline void createAssetDropDown(ecs::entity ent, const char* label, ItemType current, std::vector<ItemType> items, void(*func)(ItemType, ecs::entity))
		{
			using namespace ImGui;
			if (BeginCombo(label, current.getNameC()))
			{
				for (auto item : items)
				{
					const bool is_selected = (current == item);
					if (Selectable(item.getNameC(), is_selected))
					{
						current = item;
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