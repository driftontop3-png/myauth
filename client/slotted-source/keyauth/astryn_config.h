#pragma once

#include "skStr.h"
#include <string>

// Update these values from your Astryn Resell Portal dashboard (Implement tab).
namespace AstrynConfig {
	inline std::string appName() {
		return skCrypt("Your Application Name").decrypt();
	}

	inline std::string ownerId() {
		return skCrypt("YOUR_OWNER_ID").decrypt();
	}

	inline std::string appVersion() {
		return skCrypt("1.0").decrypt();
	}

	inline std::string apiUrl() {
		return skCrypt("https://your-project.vercel.app/api/1.3/").decrypt();
	}

	inline std::string appPath() {
		return skCrypt("").decrypt();
	}
}
