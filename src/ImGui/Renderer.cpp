#include "Renderer.h"

#include "CauseOfDeath.h"
#include "FontStyles.h"
#include "ImGuiVRHelperClientSDK.h"
#include "Manager.h"

namespace ImGui::Renderer
{
	namespace
	{
		ImGuiVRHelperPluginAPI::Client g_vrClient;
	}

	void Connect()
	{
		if (REL::Module::IsVR()) {
			// Register as an always-on HUD layer (no interactive overlay, no dashboard).
			if (!g_vrClient.Connect("KillFeed", Version::NAME.data(),
					ImGuiVRHelperPluginAPI::kClientFlag_HUDMode)) {
				logger::warn("ImGuiVRHelper not found or registration failed — VR kill feed will not be available."sv);
				return;
			}

			logger::info("Connected to ImGuiVRHelper as HUD client."sv);

			// The HUD context is private to the helper, so load the same font/style and
			// icon textures into it that the flat path loads at D3D init.
			g_vrClient.SetHudStyleCallback([]() {
				ImGui::FontStyles::GetSingleton()->LoadFontStyles();
				CauseOfDeathManager::GetSingleton()->LoadIcons();
			});
		}
	}

	struct CreateD3DAndSwapChain
	{
		static void thunk()
		{
			func();

			if (const auto renderer = RE::BSGraphics::Renderer::GetSingleton()) {
				const auto swapChain = reinterpret_cast<IDXGISwapChain*>(renderer->GetRuntimeData().renderWindows[0].swapChain);
				if (!swapChain) {
					logger::error("couldn't find swapChain");
					return;
				}

				DXGI_SWAP_CHAIN_DESC desc{};
				if (FAILED(swapChain->GetDesc(std::addressof(desc)))) {
					logger::error("IDXGISwapChain::GetDesc failed.");
					return;
				}

				const auto device = reinterpret_cast<ID3D11Device*>(renderer->GetRuntimeData().forwarder);
				const auto context = reinterpret_cast<ID3D11DeviceContext*>(renderer->GetRuntimeData().context);

				if (REL::Module::IsVR()) {
					const auto ss = RE::BSGraphics::Renderer::GetScreenSize();
					logger::info("D3D initialized — VR kill feed rendering via ImGuiVRHelper. ScreenSize {}x{}", ss.width, ss.height);

					// Store device/context so DrawKillFeed can call RenderHud each frame.
					g_d3dDevice = device;
					g_d3dContext = context;

					initialized.store(true);
				} else {
					logger::info("Initializing ImGui..."sv);

					ImGui::CreateContext();

					auto& io = ImGui::GetIO();
					io.ConfigFlags = ImGuiConfigFlags_None;
					io.IniFilename = nullptr;

					if (!ImGui_ImplWin32_Init(desc.OutputWindow)) {
						logger::error("ImGui initialization failed (Win32)");
						return;
					}
					if (!ImGui_ImplDX11_Init(device, context)) {
						logger::error("ImGui initialization failed (DX11)"sv);
						return;
					}

					logger::info("ImGui initialized.");

					ImGui::FontStyles::GetSingleton()->LoadFontStyles();
					CauseOfDeathManager::GetSingleton()->LoadIcons();

					initialized.store(true);
				}
			}
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	void DrawKillFeed()
	{
		// Skip if Imgui is not loaded
		if (!initialized.load() || Manager::GetSingleton()->IsFeedEmpty()) {
			return;
		}

		if (REL::Module::IsVR()) {
			if (!g_vrClient.IsConnected()) {
				return;
			}

			static const auto [width, height] = RE::BSGraphics::Renderer::GetScreenSize();
			const ImVec2      displaySize{ static_cast<float>(width), static_cast<float>(height) };

			g_vrClient.RenderHud(g_d3dDevice, g_d3dContext, displaySize, []() {
				CauseOfDeathManager::GetSingleton()->ReloadIconsOnDemand();
				Manager::GetSingleton()->Draw();
			});
			return;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		{
			//trick imgui into rendering at game's real resolution (ie. if upscaled with Display Tweaks)
			static const auto [width, height] = RE::BSGraphics::Renderer::GetScreenSize();

			auto& io = ImGui::GetIO();
			io.DisplaySize.x = static_cast<float>(width);
			io.DisplaySize.y = static_cast<float>(height);
		}
		ImGui::NewFrame();
		{
			CauseOfDeathManager::GetSingleton()->ReloadIconsOnDemand();
			Manager::GetSingleton()->Draw();
		}
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	// IMenu::PostDisplay
	struct HUDMenu_PostDisplay
	{
		static void thunk(RE::HUDMenu* a_menu)
		{
			DrawKillFeed();

			func(a_menu);
		}
		static inline REL::Relocation<decltype(thunk)> func;
		static inline std::size_t                      idx{ 0x6 };
	};

	struct JournalMenu_PostDisplay
	{
		static void thunk(RE::JournalMenu* a_menu)
		{
			DrawKillFeed();

			func(a_menu);
		}
		static inline REL::Relocation<decltype(thunk)> func;
		static inline std::size_t                      idx{ 0x6 };
	};

	void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(75595, 77226), OFFSET(0x9, 0x275) };  // BSGraphics::InitD3D
		stl::write_thunk_call<CreateD3DAndSwapChain>(target.address());

		stl::write_vfunc<RE::HUDMenu, HUDMenu_PostDisplay>();
		stl::write_vfunc<RE::JournalMenu, JournalMenu_PostDisplay>();
	}
}
