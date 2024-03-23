#include "sandbox/systems/game/guisystem.hpp"

namespace rythe::game
{
	gfx::framebuffer* GUISystem::mainFBO;
	gfx::framebuffer* GUISystem::pickingFBO;
	ImGuizmo::OPERATION GUISystem::currentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE GUISystem::currentGizmoMode = ImGuizmo::WORLD;

	void GUISystem::setup()
	{
		log::info("Initializing GUI system");
		gfx::gui_stage::addGuiRender<GUISystem, &GUISystem::guiRender>(this);
		gfx::render_stage::addRender<GUISystem, &GUISystem::onRender>(this);

		bindEvent<key_input<inputmap::method::MOUSE_LEFT>, &GUISystem::doClick>();

		glfwSetFramebufferSizeCallback(gfx::WindowProvider::activeWindow->getGlfwWindow(), GUISystem::framebuffer_size_callback);
	}

	void GUISystem::update()
	{

	}

	void GUISystem::onRender(core::transform camTransf, gfx::camera camera)
	{

	}

	void GUISystem::guiRender(core::transform camTransf, gfx::camera camera)
	{
		pickingFBO = gfx::Renderer::getCurrentPipeline()->getFramebuffer("PickingBuffer");
		mainFBO = gfx::Renderer::getCurrentPipeline()->getFramebuffer("MainBuffer");
		ImGuiIO& io = ImGui::GetIO();
		m_isHoveringWindow = io.WantCaptureMouse;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


		ImGui::Begin("Editor", 0, window_flags);

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		//ImGui::ShowDemoWindow();

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("New"))
			{
				ImGui::MenuItem("Entity");
				ImGui::MenuItem("Material");
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (ImGui::Begin("Heirarchy"))
		{
			ImGui::Separator();
			ImGui::Indent();
			drawHeirarchy(m_filter.m_entities);
			ImGui::Unindent();
			ImGui::End();
		}

		if (ImGui::Begin("Inspector"))
		{
			if (GUI::selected == invalid_id)
			{
				ImGui::End();
			}
			else
			{
				auto ent = GUI::selected;
				if (ImGui::CollapsingHeader(ent->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Indent();
					if (ent.hasComponent<core::transform>())
						transformEditor(ent);

					if (ent.hasComponent<gfx::mesh_renderer>())
						meshrendererEditor(ent);

					if (ent.hasComponent<gfx::light>())
						lightEditor(ent);

					if (ent.hasComponent<examplecomp>())
						exampleCompEditor(ent);

					ImGui::Unindent();
				}

				ImGui::End();
			}
		}

		if (ImGui::Begin("Scene", 0, ImGuiWindowFlags_NoBackground))
		{
			auto mainTex = mainFBO->getAttachment(gfx::AttachmentSlot::COLOR0).m_data->getId();
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			const float width = viewportPanelSize.x;
			const float height = viewportPanelSize.y;
			math::vec2 windowPos = math::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
			gfx::Renderer::RI->setViewport(1, 0, 0, width, height);
			mainFBO->rescale(width, height);
			pickingFBO->rescale(width, height);


			if (!Input::mouseCaptured && m_readPixel && ImGui::IsItemHovered())
			{
				auto color = gfx::Renderer::RI->readPixels(*pickingFBO, math::ivec2(input::Input::mousePos.x, input::Input::mousePos.y-19) - windowPos, math::ivec2(1, 1));
				rsl::id_type id = color.x + (color.y * 256) + (color.z * 256 * 256) + (color.w * 256 * 256 * 256);
				if (id != invalid_id)
					GUI::selected = ecs::entity{ &ecs::Registry::entities[id] };
				else
					GUI::selected = ecs::entity();

				m_readPixel = false;
			}

			ImGui::Image(reinterpret_cast<void*>(mainTex), ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
			if (GUI::selected != invalid_id)
				drawGizmo(camTransf, camera, math::ivec2(width, height));

			ImGui::End();
		}

		if (ImGui::Begin("Console"))
		{

			ImGui::End();
		}

		ImGui::End();
		ImGui::PopStyleVar(3);
	}

	void GUISystem::drawHeirarchy(ecs::entity_set heirarchy)
	{
		for (auto ent : heirarchy)
		{
			if (ent.hasComponent<gfx::camera>()) continue;

			if (ImGui::Button(ent->name.c_str()))
			{
				GUI::selected = ent;
			}
		}
	}
	void GUISystem::lightEditor(core::ecs::entity ent)
	{
		auto& comp = ent.getComponent<gfx::light>();

		ImGui::PushID(std::format("Entity##{}", ent->id).c_str());
		if (comp.type == gfx::LightType::DIRECTIONAL)
		{
			if (ImGui::TreeNode("Directional Light"))
			{
				ImGui::ColorEdit4("Light Color", comp.dir_data.color.data);
				ImGui::TreePop();
			}
		}
		else if (comp.type == gfx::LightType::POINT)
		{
			if (ImGui::TreeNode("Point Light"))
			{
				ImGui::ColorEdit4("Light Color", comp.point_data.color.data);
				ImGui::TreePop();
			}
		}
		ImGui::PopID();
	}
	void GUISystem::exampleCompEditor(core::ecs::entity ent)
	{
		using namespace ImGui;
		ImGui::PushID(std::format("Entity##{}", ent->id).c_str());
		if (ImGui::TreeNode("Example Component"))
		{
			auto& comp = ent.getComponent<core::examplecomp>();
			ImGui::InputFloat("Range", &comp.range);
			ImGui::InputFloat("Speed", &comp.speed);
			ImGui::InputFloat("Angular Speed", &comp.angularSpeed);
			ImGui::InputFloat3("Direction", comp.direction.data);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	void GUISystem::meshrendererEditor(core::ecs::entity ent)
	{
		using namespace ImGui;
		ImGui::PushID(std::format("Entity##{}", ent->id).c_str());
		if (ImGui::TreeNode("Mesh Renderer"))
		{
			createAssetDropDown(ent, "Mesh", ent.getComponent<gfx::mesh_renderer>().model, gfx::ModelCache::getModels(), &GUISystem::setModel);
			createAssetDropDown(ent, "Material", ent.getComponent<gfx::mesh_renderer>().material, gfx::MaterialCache::getMaterials(), &GUISystem::setMaterial);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	void GUISystem::transformEditor(core::ecs::entity ent)
	{
		using namespace ImGui;
		using namespace ImGuizmo;
		auto& transf = ent.getComponent<core::transform>();

		ImGui::PushID(std::format("Entity##{}", ent->id).c_str());
		if (ImGui::TreeNode("Transform"))
		{
			math::vec3 rot = math::toEuler(transf.rotation);
			ImGui::InputFloat3("Position##1", transf.position.data);
			if (ImGui::InputFloat3("Rotation##2", rot.data))
				transf.rotation = math::toQuat(rot);
			ImGui::InputFloat3("Scale##3", transf.scale.data);
			ImGui::TreePop();
		}
		ImGui::PopID();
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

	void GUISystem::drawGizmo(core::transform camTransf, gfx::camera camera, math::ivec2 dims)
	{
		if (GUI::selected == invalid_id || !GUI::selected.hasComponent<core::transform>()) return;

		auto ent = GUI::selected;
		if (!Input::mouseCaptured)
		{
			if (ImGui::IsKeyPressed(49))//1 key
				currentGizmoOperation = ImGuizmo::TRANSLATE;
			if (ImGui::IsKeyPressed(50))//2 key
				currentGizmoOperation = ImGuizmo::ROTATE;
			if (ImGui::IsKeyPressed(51))//3 key
				currentGizmoOperation = ImGuizmo::SCALE;
		}

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();

		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y - (Screen_Height - (19 + dims.y)), Screen_Width, Screen_Height);

		core::transform& transf = ent.getComponent<core::transform>();
		float* matrix = transf.to_world().data;
		if (ImGuizmo::Manipulate(camera.view.data, camera.projection.data, currentGizmoOperation, currentGizmoMode, matrix))
		{
			math::vec3 rot;
			ImGuizmo::DecomposeMatrixToComponents(matrix, transf.position.data, rot.data, transf.scale.data);
			transf.rotation = math::toQuat(rot);
		}
	}

	void GUISystem::doClick(key_input<inputmap::method::MOUSE_LEFT>& action)
	{
		if (Input::mouseCaptured) return;

		if (action.wasPressed())
		{
			m_readPixel = true;
		}
	}

	void GUISystem::framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		gfx::Renderer::RI->setViewport(1, 0, 0, width, height);
	}
}