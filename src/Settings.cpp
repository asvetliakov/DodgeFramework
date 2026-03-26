#include "Settings.h"

void Settings::ReadSettings()
{
	constexpr auto path = L"Data/MCM/Settings/DodgeFramework.ini";

	constexpr auto pathRestrictionSettings = L"Data/SKSE/Plugins/DodgeFrameworkRestrictions.ini";

	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(path);

	CSimpleIniA iniRestrictions;
	iniRestrictions.SetUnicode();
	iniRestrictions.LoadFile(pathRestrictionSettings);

	bUseSprintButton = ini.GetBoolValue("Controls", "bUseSprintButton", true);
	fSprintHoldDuration = (float)ini.GetDoubleValue("Controls", "fSprintHoldDuration", 0.25f);
	uDodgeKey = static_cast<uint32_t>(ini.GetLongValue("Controls", "uDodgeKey"));

	uStaminaConsumption = static_cast<uint32_t>(iniRestrictions.GetLongValue("General", "staminaConsumption", 25));
	logger::info("Dodge stamina consumption set to {}", uStaminaConsumption);

	std::string perkModFileName = iniRestrictions.GetValue("General", "perkModFileName", "");
	uint32_t perkFormId = static_cast<uint32_t>(iniRestrictions.GetLongValue("General", "perkFormID", 0));
	logger::info("Restrictions, perk mod: {}, perk form: {}", perkModFileName, perkFormId);
	if (!perkModFileName.empty() && perkFormId > 0) {
		auto dataHandler = RE::TESDataHandler::GetSingleton();

		dodgePerk = skyrim_cast<RE::BGSPerk*>(dataHandler->LookupForm(perkFormId, perkModFileName));
		if (dodgePerk) {
			logger::info("Sucessfully loaded the restriction perk");
		}
	}
}
