#include "environment.hpp"
#include "image.hpp"

void Environment::load(const string& path, Resources& resources)
{
	std::string err;
	Json def = Json::parse(resources.getText(path, Resources::NO_CACHE), err);
	if (!err.empty())
		panic("Failed to read environment: %s", err.c_str());

	ASSERT(def.is_object());
	if (def["skybox"].is_string()) {
		const string& skyboxPath = def["skybox"].string_value();
		if (skyboxPath.back() == '/' || skyboxPath.back() == '\\') {
			skybox[0] = resources.getImage(skyboxPath + "px.jpg");
			skybox[1] = resources.getImage(skyboxPath + "nx.jpg");
			skybox[2] = resources.getImage(skyboxPath + "py.jpg");
			skybox[3] = resources.getImage(skyboxPath + "ny.jpg");
			skybox[4] = resources.getImage(skyboxPath + "pz.jpg");
			skybox[5] = resources.getImage(skyboxPath + "nz.jpg");
		} else {
			for (int i = 0; i < 6; i++)
				skybox[i] = resources.getImage(skyboxPath);
		}
	} else if (def["skybox"].is_array()) {
		for (int i = 0; i < 6; i++)
			skybox[i] = resources.getImage(def["skybox"][i].string_value());
	}
	for (int i = 0; i < 6; i++)
		if (skybox[i])
			skybox[i]->sRGB = true;

	if (def["exposure"].is_number())
		exposure = def["exposure"].number_value();
	if (def["tonemap"].is_number())
		tonemap = (Tonemap)def["tonemap"].number_value();
	if (!def["ambient"].is_null())
		ambient = colorToVec3(def["ambient"]);
	if (!def["sunDirection"].is_null())
		sunDirection = toVec3(def["sunDirection"]);
	if (!def["sunColor"].is_null())
		sunColor = colorToVec3(def["sunColor"]);
	if (!def["fogColor"].is_null())
		fogColor = colorToVec3(def["fogColor"]);
	if (!def["fogDensity"].is_null())
		fogDensity = def["fogDensity"].number_value();
}

void Environment::reset()
{
	*this = Environment();
}
