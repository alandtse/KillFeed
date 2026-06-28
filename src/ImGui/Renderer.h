#pragma once

namespace ImGui::Renderer
{
	void Install();
	void Connect();

	// members
	inline std::atomic initialized{ false };

	// Device/context captured at D3D init, used by RenderHud each frame in VR.
	inline ID3D11Device*        g_d3dDevice = nullptr;
	inline ID3D11DeviceContext* g_d3dContext = nullptr;
}
