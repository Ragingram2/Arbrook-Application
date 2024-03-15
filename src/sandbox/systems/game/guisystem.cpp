#include "sandbox/systems/game/guisystem.hpp"

namespace rythe::game
{
	void GUISystem::setup()
	{
		log::info("Initializing GUI system");
		gfx::gui_stage::addGuiRender<GUISystem, &GUISystem::guiRender>(this);
		gfx::render_stage::addRender<GUISystem, &GUISystem::onRender>(this);

		bindEvent<key_input<inputmap::method::MOUSE_LEFT>, &GUISystem::readPixel>();
	}

	void GUISystem::update()
	{

	}

	void GUISystem::onRender(core::transform camTransf, gfx::camera camera)
	{
		if (Input::mouseCaptured) return;

		if (!m_readPixel) return;

		gfx::framebuffer* pickingFBO = gfx::Renderer::getCurrentPipeline()->getFramebuffer("PickingBuffer");
		pickingFBO->bind();

		auto color = gfx::Renderer::RI->readPixels(math::ivec2(input::Input::mousePos.x, input::Input::mousePos.y), math::ivec2(1, 1));

		rsl::id_type id = color.x + (color.y * 256) + (color.z * 256 * 256) + (color.w * 256 * 256 * 256);

		pickingFBO->unbind();

		if (id != invalid_id)
			GUI::selected = ecs::entity{ &ecs::Registry::entities[id] };
		else
			GUI::selected = ecs::entity();

		m_readPixel = false;
	}

	void GUISystem::guiRender(core::transform camTransf, gfx::camera camera)
	{
		using namespace ImGui;
		using namespace ImGuizmo;

		ImGuiIO& io = GetIO();
		m_isHoveringWindow = io.WantCaptureMouse;

		static OPERATION currentGizmoOperation = TRANSLATE;
		static MODE currentGizmoMode = WORLD;

		if (!Input::mouseCaptured)
		{
			if (IsKeyPressed(49))//1 key
				currentGizmoOperation = TRANSLATE;
			if (IsKeyPressed(50))//2 key
				currentGizmoOperation = ROTATE;
			if (IsKeyPressed(51))//3 key
				currentGizmoOperation = SCALE;
		}

		SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		if (Begin("Inspector"))
		{
			if (GUI::selected == invalid_id)
			{
				End();
				return;
			}

			auto ent = GUI::selected;
			if (CollapsingHeader(ent->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ent.hasComponent<core::transform>())
				{
					Indent();
					transformEditor(ent);
					Unindent();

					core::transform& transf = ent.getComponent<core::transform>();
					math::vec3 eulerRot = math::toEuler(transf.rotation);
					float matrix[16];
					RecomposeMatrixFromComponents(transf.position.data, eulerRot.data, transf.scale.data, matrix);
					Manipulate(camera.view.data, camera.projection.data, currentGizmoOperation, currentGizmoMode, matrix);
					math::vec3 pos;
					math::vec3 rot;
					math::vec3 scale;
					DecomposeMatrixToComponents(matrix, pos.data, rot.data, scale.data);
					transf.position = pos;
					transf.rotation = math::toQuat(rot);
					transf.scale = scale;
				}

				if (ent.hasComponent<gfx::mesh_renderer>())
				{
					Indent();
					meshrendererEditor(ent);
					Unindent();
				}

				if (ent.hasComponent<gfx::light>())
				{
					Indent();
					lightEditor(ent);
					Unindent();
				}

				if (ent.hasComponent<examplecomp>())
				{
					Indent();
					exampleCompEditor(ent);
					Unindent();
				}
			}
			End();
		}
	}

	void GUISystem::lightEditor(core::ecs::entity ent)
	{
		using namespace ImGui;
		auto& comp = ent.getComponent<gfx::light>();

		PushID(std::format("Entity##{}", ent->id).c_str());
		if (comp.type == gfx::LightType::DIRECTIONAL)
		{
			if (TreeNode("Directional Light"))
			{
				math::color color = comp.dir_data.color;
				if (ColorEdit4("Light Color", color.data))
				{
					comp.dir_data.color = color;
				}
				TreePop();
			}
		}
		else if (comp.type == gfx::LightType::POINT)
		{
			if (TreeNode("Point Light"))
			{
				auto& comp = ent.getComponent<gfx::light>();
				math::color color = comp.point_data.color;
				if (ColorEdit4("Light Color", color.data))
				{
					comp.point_data.color = color;
				}
				TreePop();
			}
		}
		PopID();
	}

	void GUISystem::exampleCompEditor(core::ecs::entity ent)
	{
		using namespace ImGui;
		PushID(std::format("Entity##{}", ent->id).c_str());
		if (TreeNode("Example Component"))
		{
			auto& comp = ent.getComponent<core::examplecomp>();
			float range = comp.range;
			float speed = comp.speed;
			float angularSpeed = comp.angularSpeed;
			math::vec3 direction = comp.direction;

			if (InputFloat("Range", &range))
			{
				comp.range = range;
			}
			if (InputFloat("Speed", &speed))
			{
				comp.speed = speed;
			}
			if (InputFloat("Angular Speed", &angularSpeed))
			{
				comp.angularSpeed = angularSpeed;
			}
			if (InputFloat3("Direction", direction.data))
			{
				comp.direction = direction;
			}

			TreePop();
		}
		PopID();
	}

	void GUISystem::meshrendererEditor(core::ecs::entity ent)
	{
		using namespace ImGui;
		PushID(std::format("Entity##{}", ent->id).c_str());
		if (TreeNode("Mesh Renderer"))
		{
			auto models = gfx::ModelCache::getModels();
			ast::asset_handle<gfx::model> currentMesh = ent.getComponent<gfx::mesh_renderer>().model;
			if (BeginCombo("Mesh", currentMesh->name.c_str()))
			{
				for (auto handle : models)
				{
					const bool is_selected = (currentMesh == handle);
					if (Selectable(handle->name.c_str(), is_selected))
						currentMesh = handle;

					if (is_selected)
					{
						SetItemDefaultFocus();
					}
				}
				setModel(currentMesh, ent);
				EndCombo();
			}

			auto mats = gfx::MaterialCache::getMaterials();
			ast::asset_handle<gfx::material> currentMat = ent.getComponent<gfx::mesh_renderer>().material;
			if (BeginCombo("Material", currentMat->name.c_str()))
			{
				for (auto handle : mats)
				{
					const bool is_selected = (currentMat == handle);
					if (Selectable(handle->name.c_str(), is_selected))
						currentMat = handle;

					if (is_selected)
					{
						SetItemDefaultFocus();
					}
				}
				setMaterial(currentMat, ent);
				EndCombo();
			}
			TreePop();
		}
		PopID();
	}

	void GUISystem::transformEditor(core::ecs::entity ent)
	{
		using namespace ImGui;

		PushID(std::format("Entity##{}", ent->id).c_str());
		if (TreeNode("Transform"))
		{
			auto& transf = ent.getComponent<core::transform>();
			math::vec3 pos = transf.position;
			math::vec3 rot = math::toEuler(transf.rotation);
			math::vec3 scale = transf.scale;

			if (InputFloat3("Position##1", pos.data))
			{
				transf.position = pos;
			}
			if (InputFloat3("Rotation##2", rot.data))
			{
				transf.rotation = math::toQuat(rot);
			}
			if (InputFloat3("Scale##3", scale.data))
			{
				transf.scale = scale;
			}
			TreePop();
		}
		PopID();
	}

	void GUISystem::setModel(ast::asset_handle<gfx::model> handle, ecs::entity ent)
	{
		auto& renderer = ent.getComponent<gfx::mesh_renderer>();
		if (renderer.model != handle)
		{
			renderer.model = handle;
			renderer.dirty = true;
		}
	}

	void GUISystem::setMaterial(ast::asset_handle<gfx::material> handle, ecs::entity ent)
	{
		auto& renderer = ent.getComponent<gfx::mesh_renderer>();
		if (renderer.material != handle)
		{
			renderer.material = handle;
			renderer.dirty = true;
		}
	}
}