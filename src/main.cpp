#include "Events.h"
#include "Hooks.h"
#include "Papyrus.h"
#include "Settings.h"

#include "TrueHUDAPI.h"

TRUEHUD_API::IVTrueHUD1* trueHudApi = nullptr;

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kPostLoad:
		{
			auto trueHUDAPI = reinterpret_cast<TRUEHUD_API::IVTrueHUD1*>(TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V1));
			if (trueHUDAPI) {
				trueHudApi = trueHUDAPI;
				logger::info("Successfully acquired TrueHUD API.");
			} else {
				logger::error("Failed to acquire TrueHUD API");
			}
			break;
		}
	case SKSE::MessagingInterface::kDataLoaded:
		Events::SinkEventHandlers();
		Settings::ReadSettings();
		break;
	}
}

void InitLog()
{
	auto path = logger::log_directory();
	if (!path) {
		SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitLog();
	logger::info("Dodge Framework loaded");

	SKSE::Init(a_skse);

	auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener("SKSE", MessageHandler)) {
		return false;
	}

	Hooks::Install();
	Papyrus::Register();

	return true;
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName(Version::PROJECT.data());
	v.PluginVersion(Version::VERSION);
	v.UsesAddressLibrary();
	v.UsesNoStructs();
	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI
	SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = Version::PROJECT.data();
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = 1;
	return true;
}
