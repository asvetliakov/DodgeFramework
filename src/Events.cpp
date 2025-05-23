#include "Events.h"
#include "Settings.h"
#include "Utils.h"

namespace Events
{
	enum Direction : std::uint32_t
	{
		kNeutral = 0,
		kForward = 1,
		kRightForward = 2,
		kRight = 3,
		kRightBackward = 4,
		kBackward = 5,
		kLeftBackward = 6,
		kLeft = 7,
		kLeftForward = 8
	};

	std::string_view menus[] = {
		RE::BarterMenu::MENU_NAME,
		RE::BookMenu::MENU_NAME,
		RE::Console::MENU_NAME,
		RE::ContainerMenu::MENU_NAME,
		RE::CraftingMenu::MENU_NAME,
		RE::CreationClubMenu::MENU_NAME,
		RE::DialogueMenu::MENU_NAME,
		RE::FavoritesMenu::MENU_NAME,
		RE::GiftMenu::MENU_NAME,
		RE::InventoryMenu::MENU_NAME,
		RE::JournalMenu::MENU_NAME,
		RE::LevelUpMenu::MENU_NAME,
		RE::LockpickingMenu::MENU_NAME,
		RE::MagicMenu::MENU_NAME,
		RE::MapMenu::MENU_NAME,
		RE::MessageBoxMenu::MENU_NAME,
		RE::ModManagerMenu::MENU_NAME,
		RE::RaceSexMenu::MENU_NAME,
		RE::SleepWaitMenu::MENU_NAME,
		RE::StatsMenu::MENU_NAME,
		RE::TrainingMenu::MENU_NAME,
		RE::TutorialMenu::MENU_NAME,
		RE::TweenMenu::MENU_NAME,
	};

	InputEventHandler*
		InputEventHandler::GetSingleton()
	{
		static InputEventHandler singleton;
		return std::addressof(singleton);
	}

	auto InputEventHandler::ProcessEvent(RE::InputEvent* const* a_event, [[maybe_unused]] RE::BSTEventSource<RE::InputEvent*>* a_eventSource)
		-> EventResult
	{
		using EventType = RE::INPUT_EVENT_TYPE;
		using DeviceType = RE::INPUT_DEVICE;

		if (Settings::uDodgeKey == kInvalid) {
			return EventResult::kContinue;
		}

		if (!a_event) {
			return EventResult::kContinue;
		}

		for (auto event = *a_event; event; event = event->next) {
			if (event->eventType != EventType::kButton) {
				continue;
			}

			auto button = static_cast<RE::ButtonEvent*>(event);
			if (!button->IsDown()) {
				continue;
			}

			auto key = button->idCode;
			switch (button->device.get()) {
			case DeviceType::kMouse:
				key += kMouseOffset;
				break;
			case DeviceType::kKeyboard:
				key += kKeyboardOffset;
				break;
			case DeviceType::kGamepad:
				key = GetGamepadIndex((RE::BSWin32GamepadDevice::Key)key);
				break;
			default:
				continue;
			}

			if (key == Settings::uDodgeKey) {
				Dodge();
				break;
			}
		}

		return EventResult::kContinue;
	}

	bool IsAnyMenuOpen()
	{
		auto ui = RE::UI::GetSingleton();
		for (const std::string_view& menu : menus) {
			if (ui->IsMenuOpen(menu)) {
				return true;
			}
		}
		return false;
	}

	void Dodge()
	{
		auto playerCharacter = RE::PlayerCharacter::GetSingleton();
		auto playerControls = RE::PlayerControls::GetSingleton();

		auto ui = RE::UI::GetSingleton();
		auto controlMap = RE::ControlMap::GetSingleton();

		if (!playerCharacter || !playerControls) {
			return;
		}

		if (
			ui->GameIsPaused() ||
			!controlMap->IsMovementControlsEnabled() ||
			!controlMap->IsLookingControlsEnabled() ||
			IsAnyMenuOpen() ||
			playerCharacter->AsActorState()->GetSitSleepState() != RE::SIT_SLEEP_STATE::kNormal ||
			playerCharacter->GetActorBase()->GetActorValue(RE::ActorValue::kStamina) <= 0) {
			return;
		}

		if (Settings::dodgePerk != nullptr && !playerCharacter->HasPerk(Settings::dodgePerk)) {
			logger::info("Player doesn't have required perk aborting dodge");
			return;
		}

		auto currentStamina = playerCharacter->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);

		if (Settings::uStaminaConsumption > 0 && currentStamina < Settings::uStaminaConsumption) {
			logger::info("Not enough stamina for dodge, currentStamina {}, required {}", currentStamina, Settings::uStaminaConsumption);
			if (trueHudApi != nullptr) {
				trueHudApi->FlashActorValue(playerCharacter->GetHandle(), RE::ActorValue::kStamina, true);
			}
			return;
		}

		auto normalizedInputDirection = Vec2Normalize(playerControls->data.prevMoveVec);
		bool didDodge = false;
		if (normalizedInputDirection.x == 0.f && normalizedInputDirection.y == 0.f) {
			playerCharacter->SetGraphVariableFloat("Dodge_Angle", PI);
			playerCharacter->SetGraphVariableInt("Dodge_Direction", kNeutral);
			playerCharacter->NotifyAnimationGraph("Dodge_N");
			playerCharacter->NotifyAnimationGraph("Dodge");
			logger::debug("neutral");
			didDodge = true;
		} else {
			RE::NiPoint2 forwardVector(0.f, 1.f);
			float dodgeAngle = GetAngle(normalizedInputDirection, forwardVector);

			if (dodgeAngle >= -PI8 && dodgeAngle < PI8) {
				playerCharacter->SetGraphVariableFloat("Dodge_Angle", dodgeAngle);
				playerCharacter->SetGraphVariableInt("Dodge_Direction", kForward);
				playerCharacter->NotifyAnimationGraph("Dodge_F");
				playerCharacter->NotifyAnimationGraph("Dodge");
				didDodge = true;
				logger::debug("forward");
			} else if (dodgeAngle >= PI8 && dodgeAngle < 3 * PI8) {
				playerCharacter->SetGraphVariableFloat("Dodge_Angle", dodgeAngle);
				playerCharacter->SetGraphVariableInt("Dodge_Direction", kRightForward);
				playerCharacter->NotifyAnimationGraph("Dodge_RF");
				playerCharacter->NotifyAnimationGraph("Dodge");
				didDodge = true;
				logger::debug("right-forward");
			} else if (dodgeAngle >= 3 * PI8 && dodgeAngle < 5 * PI8) {
				playerCharacter->SetGraphVariableFloat("Dodge_Angle", dodgeAngle);
				playerCharacter->SetGraphVariableInt("Dodge_Direction", kRight);
				playerCharacter->NotifyAnimationGraph("Dodge_R");
				playerCharacter->NotifyAnimationGraph("Dodge");
				didDodge = true;
				logger::debug("right");
			} else if (dodgeAngle >= 5 * PI8 && dodgeAngle < 7 * PI8) {
				playerCharacter->SetGraphVariableFloat("Dodge_Angle", dodgeAngle);
				playerCharacter->SetGraphVariableInt("Dodge_Direction", kRightBackward);
				playerCharacter->NotifyAnimationGraph("Dodge_RB");
				playerCharacter->NotifyAnimationGraph("Dodge");
				didDodge = true;
				logger::debug("right-backward");
			} else if (dodgeAngle >= 7 * PI8 || dodgeAngle < 7 * -PI8) {
				playerCharacter->SetGraphVariableFloat("Dodge_Angle", dodgeAngle);
				playerCharacter->SetGraphVariableInt("Dodge_Direction", kBackward);
				playerCharacter->NotifyAnimationGraph("Dodge_B");
				playerCharacter->NotifyAnimationGraph("Dodge");
				didDodge = true;
				logger::debug("backward");
			} else if (dodgeAngle >= 7 * -PI8 && dodgeAngle < 5 * -PI8) {
				playerCharacter->SetGraphVariableFloat("Dodge_Angle", dodgeAngle);
				playerCharacter->SetGraphVariableInt("Dodge_Direction", kLeftBackward);
				playerCharacter->NotifyAnimationGraph("Dodge_LB");
				playerCharacter->NotifyAnimationGraph("Dodge");
				didDodge = true;
				logger::debug("left-backward");
			} else if (dodgeAngle >= 5 * -PI8 && dodgeAngle < 3 * -PI8) {
				playerCharacter->SetGraphVariableFloat("Dodge_Angle", dodgeAngle);
				playerCharacter->SetGraphVariableInt("Dodge_Direction", kLeft);
				playerCharacter->NotifyAnimationGraph("Dodge_L");
				playerCharacter->NotifyAnimationGraph("Dodge");
				didDodge = true;
				logger::debug("left");
			} else if (dodgeAngle >= 3 * -PI8 && dodgeAngle < -PI8) {
				playerCharacter->SetGraphVariableFloat("Dodge_Angle", dodgeAngle);
				playerCharacter->SetGraphVariableInt("Dodge_Direction", kLeftForward);
				playerCharacter->NotifyAnimationGraph("Dodge_LF");
				playerCharacter->NotifyAnimationGraph("Dodge");
				didDodge = true;
				logger::debug("left-forward");
			}
		}

		if (Settings::uStaminaConsumption > 0 && didDodge) {
			playerCharacter->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -(float)Settings::uStaminaConsumption);
		}
	}

	std::uint32_t InputEventHandler::GetGamepadIndex(RE::BSWin32GamepadDevice::Key a_key)
	{
		using Key = RE::BSWin32GamepadDevice::Key;

		std::uint32_t index;
		switch (a_key) {
		case Key::kUp:
			index = 0;
			break;
		case Key::kDown:
			index = 1;
			break;
		case Key::kLeft:
			index = 2;
			break;
		case Key::kRight:
			index = 3;
			break;
		case Key::kStart:
			index = 4;
			break;
		case Key::kBack:
			index = 5;
			break;
		case Key::kLeftThumb:
			index = 6;
			break;
		case Key::kRightThumb:
			index = 7;
			break;
		case Key::kLeftShoulder:
			index = 8;
			break;
		case Key::kRightShoulder:
			index = 9;
			break;
		case Key::kA:
			index = 10;
			break;
		case Key::kB:
			index = 11;
			break;
		case Key::kX:
			index = 12;
			break;
		case Key::kY:
			index = 13;
			break;
		case Key::kLeftTrigger:
			index = 14;
			break;
		case Key::kRightTrigger:
			index = 15;
			break;
		default:
			index = kInvalid;
			break;
		}

		return index != kInvalid ? index + kGamepadOffset : kInvalid;
	}

	void SinkEventHandlers()
	{
		auto deviceManager = RE::BSInputDeviceManager::GetSingleton();
		deviceManager->AddEventSink(InputEventHandler::GetSingleton());
		logger::info("Added input event sink");
	}
}
