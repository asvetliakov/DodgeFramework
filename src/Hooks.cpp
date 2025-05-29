#include "Hooks.h"

#include "Events.h"
#include "Settings.h"

namespace Hooks
{
	void Install()
	{
		logger::trace("Hooking...");

		SprintHandlerHook::Hook();

		logger::trace("...success");
	}

	static bool bStoppingSprint = false;
	//static float heldDownTimeOffset = 0.f;

	void SprintHandlerHook::ProcessButton(RE::SprintHandler* a_this, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data)
	{
		using FlagBDD = RE::PlayerCharacter::PlayerFlags;

		if (Settings::bUseSprintButton) {
			auto playerCharacter = RE::PlayerCharacter::GetSingleton();
			auto userEvent = a_event->QUserEvent();
			auto userEvents = RE::UserEvents::GetSingleton();

			if (userEvent == userEvents->sprint) {
				// 1) short-tap → dodge
				if (a_event->IsUp() && a_event->HeldDuration() < Settings::fSprintHoldDuration) {
					Events::Dodge();
					bStoppingSprint = false;
					return;  // eat the Up so vanilla doesn’t toggle sprint
				}

				// 2) keep vanilla sprint alive but don’t let HeldDuration grow while sprinting
				if (playerCharacter->playerFlags.isSprinting && !bStoppingSprint) {
					a_event->heldDownSecs = 0.f;  // optional – see note below
				}
			}
		}

		_ProcessButton(a_this, a_event, a_data);
	}
}
