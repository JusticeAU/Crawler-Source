#include "Material.h"

void Material::DrawGUI()
{
	// Ka
	float KaGUI[3] = { Ka.r, Ka.g, Ka.b, };
	if (ImGui::ColorEdit3("Ambient Colour", KaGUI))
		Ka = { KaGUI[0], KaGUI[1], KaGUI[2] };

	// Kd
	float KdGUI[3] = { Kd.r, Kd.g, Kd.b, };
	if (ImGui::ColorEdit3("Diffuse Colour", KdGUI))
		Kd = { KdGUI[0], KdGUI[1], KdGUI[2] };

	// Ks
	float KsGUI[3] = { Ks.r, Ks.g, Ks.b, };
	if (ImGui::ColorEdit3("Specular Colour", KsGUI))
		Ks = { KsGUI[0], KsGUI[1], KsGUI[2] };

	// Specular Power
	ImGui::DragFloat("Specular Power", &specularPower);
}
