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
		if (Input::mouseCaptured || !m_readPixel) return;

		gfx::framebuffer* pickingFBO = gfx::Renderer::getCurrentPipeline()->getFramebuffer("PickingBuffer");
		auto color = gfx::Renderer::RI->readPixels(*pickingFBO, math::ivec2(input::Input::mousePos.x, input::Input::mousePos.y), math::ivec2(1, 1));

		rsl::id_type id = color.x + (color.y * 256) + (color.z * 256 * 256) + (color.w * 256 * 256 * 256);

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

		//ShowDemoWindow();
		ImGuiIO& io = GetIO();
		m_isHoveringWindow = io.WantCaptureMouse;

		gfx::framebuffer* mainFBO = gfx::Renderer::getCurrentPipeline()->getFramebuffer("MainBuffer");
		gfx::framebuffer* pickingFBO = gfx::Renderer::getCurrentPipeline()->getFramebuffer("PickingBuffer");

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

		if (Begin("GameWindow"))
		{
			auto pickingTex = pickingFBO->getAttachment(gfx::AttachmentSlot::COLOR0).m_data->getId();
			auto mainTex = mainFBO->getAttachment(gfx::AttachmentSlot::COLOR0).m_data->getId();
			//GetWindowDrawList()->AddImage( (ImTextureID)pickingTex, ImVec2(GetCursorScreenPos()),ImVec2(GetCursorScreenPos().x + Screen_Width / 2, GetCursorScreenPos().y + Screen_Height / 2), ImVec2(0, 1), ImVec2(1, 0));
			GetWindowDrawList()->AddImage((ImTextureID)mainTex, ImVec2(GetCursorScreenPos()),ImVec2(GetCursorScreenPos().x + Screen_Width / 2, GetCursorScreenPos().y + Screen_Height / 2), ImVec2(0, 1), ImVec2(1, 0));
			End();
		}

		if (Begin("Inspector"))
		{
			if (GUI::selected == invalid_id)
			{
				End();
			}
			else
			{
				auto ent = GUI::selected;
				if (CollapsingHeader(ent->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					Indent();
					if (ent.hasComponent<core::transform>())
					{
						transformEditor(ent);

						core::transform& transf = ent.getComponent<core::transform>();
						math::vec3 eulerRot = math::toEuler(transf.rotation);
						float matrix[16];
						RecomposeMatrixFromComponents(transf.position.data, eulerRot.data, transf.scale.data, matrix);
						if (Manipulate(camera.view.data, camera.projection.data, currentGizmoOperation, currentGizmoMode, matrix))
						{
							math::vec3 pos;
							math::vec3 rot;
							math::vec3 scale;
							DecomposeMatrixToComponents(matrix, pos.data, rot.data, scale.data);
							transf.position = pos;
							transf.rotation = math::toQuat(rot);
							transf.scale = scale;
						}
					}

					if (ent.hasComponent<gfx::mesh_renderer>())
					{
						meshrendererEditor(ent);
					}

					if (ent.hasComponent<gfx::light>())
					{
						lightEditor(ent);
					}

					if (ent.hasComponent<examplecomp>())
					{
						exampleCompEditor(ent);
					}

					Unindent();
				}

				End();
			}
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
			createAssetDropDown(ent, "Mesh", ent.getComponent<gfx::mesh_renderer>().model, gfx::ModelCache::getModels(), &GUISystem::setModel);
			createAssetDropDown(ent, "Material", ent.getComponent<gfx::mesh_renderer>().material, gfx::MaterialCache::getMaterials(), &GUISystem::setMaterial);
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

	void GUISystem::readPixel(key_input<inputmap::method::MOUSE_LEFT>& action)
	{
		if (m_isHoveringWindow || Input::mouseCaptured) return;

		if (action.wasPressed())
		{
			m_readPixel = true;
		}
	}
}