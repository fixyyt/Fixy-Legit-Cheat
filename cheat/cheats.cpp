#include"cheats.h"
#include"gui.h"
#include"globals.h"
#include"vector.h"
#include<math.h>
#include<Windows.h>
#include<TlHelp32.h>
#include <string>

#include<thread>
#include<array>

constexpr Vector3 CalculateAngle(
    const Vector3& localPosition,
    const Vector3& enemyPosition,
    const Vector3& viewAngles) noexcept
{
    return ((enemyPosition - localPosition).ToAngle() - viewAngles);
}

struct Vector2 {
    float x = {}, y = {};
};
auto oldPunch = Vector2{};

void DrawRect(IDirect3DDevice9* dev, int x, int y, int w, int h, D3DCOLOR color)
{
    D3DRECT BarRect = { x, y, x + w, y + h };
    dev->Clear(1, &BarRect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 0, 0);
}
struct Color
{
    std::uint8_t r{}, g{}, b{};
};

void cheats::VisualsThread(const Memory& mem) noexcept
{
    const auto& localPlayer = mem.Read<std::uintptr_t>(globals::clientAdress + offsets::signatures::dwLocalPlayer);
    const auto& beggfov = mem.Read<int>(localPlayer + offsets::netvars::m_iDefaultFOV);
    HWND hwnd = FindWindow(nullptr, "Counter-Strike: Global Offensive - Direct3D 9");
    HBRUSH hbrush = CreateSolidBrush(RGB(255, 0, 0));
    HDC hdc = GetDC(hwnd);
    RECT crosshairRect, clientRect;
    uint16_t crosshairSize = 7;
    POINT crosshairPos{}, resolution;

	while(gui::isRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		//const auto localPlayer = mem.Read<std::uintptr_t>(globals::clientAdress + offsets::signatures::dwLocalPlayer);

		if (!localPlayer) continue;

		const auto glowManager = mem.Read<std::uintptr_t>(globals::clientAdress + offsets::signatures::dwGlowObjectManager);

		const auto localTeam = mem.Read < std::int32_t>(localPlayer + offsets::netvars::m_iTeamNum);


		for (int i = 1; i <= 32; i++)
		{
			const auto player = mem.Read<std::uintptr_t>(globals::clientAdress + offsets::signatures::dwEntityList + i * 0x10);

			if (!player) continue;

			const auto team = mem.Read < std::int32_t>(player + offsets::netvars::m_iTeamNum);


			const auto lifeState = mem.Read<std::int32_t>(player + offsets::netvars::m_lifeState);
			if (lifeState != 0) continue;

			if (globals::glow)
			{

				const auto glowIndex = mem.Read<std::int32_t>(player + offsets::netvars::m_iGlowIndex);

                if (team == localTeam)
                {
                    if (globals::teamGlow)
                    {
                        mem.Write(glowManager + (glowIndex * 0x38) + 0x8, globals::glowColorTeam[0]); //r
                        mem.Write(glowManager + (glowIndex * 0x38) + 0xC, globals::glowColorTeam[1]); //g
                        mem.Write(glowManager + (glowIndex * 0x38) + 0x10, globals::glowColorTeam[2]); //b
                        mem.Write(glowManager + (glowIndex * 0x38) + 0x14, globals::glowColorTeam[3]); //alpha

                        mem.Write(glowManager + (glowIndex * 0x38) + 0x28, true);
                        mem.Write(glowManager + (glowIndex * 0x38) + 0x29, false);
                    }
                }
                else {
                    mem.Write(glowManager + (glowIndex * 0x38) + 0x8, globals::glowColor[0]); //r
                    mem.Write(glowManager + (glowIndex * 0x38) + 0xC, globals::glowColor[1]); //g
                    mem.Write(glowManager + (glowIndex * 0x38) + 0x10, globals::glowColor[2]); //b
                    mem.Write(glowManager + (glowIndex * 0x38) + 0x14, globals::glowColor[3]); //alpha

                    mem.Write(glowManager + (glowIndex * 0x38) + 0x28, true);
                    mem.Write(glowManager + (glowIndex * 0x38) + 0x29, false);
                }
			}
			if (globals::radar)
			{
				mem.Write(player + offsets::netvars::m_bSpotted, true);
			}
		
		}
        if (globals::aimassist)
        {
            if (GetAsyncKeyState(VK_XBUTTON2))
            {

                // get local player
                const auto localTeam = mem.Read<std::int32_t>(localPlayer + offsets::netvars::m_iTeamNum);

                // eye position = origin + viewOffset
                const auto localEyePosition = mem.Read<Vector3>(localPlayer + offsets::netvars::m_vecOrigin) +
                    mem.Read<Vector3>(localPlayer + offsets::netvars::m_vecViewOffset);

                const auto clientState = mem.Read<std::uintptr_t>(globals::engineAdress + offsets::signatures::dwClientState);

                const auto localPlayerId =
                    mem.Read<std::int32_t>(clientState + offsets::signatures::dwClientState_GetLocalPlayer);

                const auto viewAngles = mem.Read<Vector3>(clientState + offsets::signatures::dwClientState_ViewAngles);
                const auto aimPunch = mem.Read<Vector3>(localPlayer + offsets::netvars::m_aimPunchAngle) * 2;

                // aimbot fov
                auto bestFov = globals::aimbotfov;
                auto bestAngle = Vector3{ };


                for (auto i = 1; i <= 32; ++i)
                {
                    const auto player = mem.Read<std::uintptr_t>(globals::clientAdress + offsets::signatures::dwEntityList + i * 0x10);

                    if (mem.Read<std::int32_t>(player + offsets::netvars::m_iTeamNum) == localTeam)
                        continue;

                    if (mem.Read<bool>(player + offsets::signatures::m_bDormant))
                        continue;

                    if (mem.Read<std::int32_t>(player + offsets::netvars::m_lifeState))
                        continue;

                    if (mem.Read<std::int32_t>(player + offsets::netvars::m_bSpottedByMask) & (1 << localPlayerId))
                    {
                        const auto boneMatrix = mem.Read<std::uintptr_t>(player + offsets::netvars::m_dwBoneMatrix);

                        // pos of player head in 3d space
                        // 8 is the head bone index :)
                        const auto playerHeadPosition = Vector3{
                            mem.Read<float>(boneMatrix + 0x30 * 8 + 0x0C),
                            mem.Read<float>(boneMatrix + 0x30 * 8 + 0x1C),
                            mem.Read<float>(boneMatrix + 0x30 * 8 + 0x2C)
                        };

                        const auto angle = CalculateAngle(
                            localEyePosition,
                            playerHeadPosition,
                            viewAngles + aimPunch
                        );

                        const auto fov = std::hypot(angle.x, angle.y);

                        if (fov < bestFov)
                        {
                            bestFov = fov;
                            bestAngle = angle;
                        }
                    }
                }
                // if we have a best angle, do aimbot
                if (!bestAngle.IsZero())
                    mem.Write<Vector3>(clientState + offsets::signatures::dwClientState_ViewAngles, viewAngles + bestAngle / globals::aimassistSmooth); // smoothing
            }
        }
        if (globals::aimassist2)
        {
                // get local player
                const auto localTeam = mem.Read<std::int32_t>(localPlayer + offsets::netvars::m_iTeamNum);

                // eye position = origin + viewOffset
                const auto localEyePosition = mem.Read<Vector3>(localPlayer + offsets::netvars::m_vecOrigin) + mem.Read<Vector3>(localPlayer + offsets::netvars::m_vecViewOffset);

                const auto clientState = mem.Read<std::uintptr_t>(globals::engineAdress + offsets::signatures::dwClientState);

                const auto localPlayerId =
                    mem.Read<std::int32_t>(clientState + offsets::signatures::dwClientState_GetLocalPlayer);

                const auto viewAngles = mem.Read<Vector3>(clientState + offsets::signatures::dwClientState_ViewAngles);
                const auto aimPunch = mem.Read<Vector3>(localPlayer + offsets::netvars::m_aimPunchAngle) * 2;

                // aimbot fov
                auto bestFov = globals::aimbotfov;
                auto bestAngle = Vector3{ };


                for (auto i = 1; i <= 32; ++i)
                {
                    const auto player = mem.Read<std::uintptr_t>(globals::clientAdress + offsets::signatures::dwEntityList + i * 0x10);

                    if (mem.Read<std::int32_t>(player + offsets::netvars::m_iTeamNum) == localTeam)
                        continue;

                    if (mem.Read<bool>(player + offsets::signatures::m_bDormant))
                        continue;

                    if (mem.Read<std::int32_t>(player + offsets::netvars::m_lifeState))
                        continue;

                    if (mem.Read<std::int32_t>(player + offsets::netvars::m_bSpottedByMask) & (1 << localPlayerId))
                    {
                        const auto boneMatrix = mem.Read<std::uintptr_t>(player + offsets::netvars::m_dwBoneMatrix);

                        // pos of player head in 3d space
                        // 8 is the head bone index :)
                        const auto playerHeadPosition = Vector3{
                            mem.Read<float>(boneMatrix + 0x30 * 8 + 0x0C),
                            mem.Read<float>(boneMatrix + 0x30 * 8 + 0x1C),
                            mem.Read<float>(boneMatrix + 0x30 * 8 + 0x2C)
                        };

                        const auto angle = CalculateAngle(
                            localEyePosition,
                            playerHeadPosition,
                            viewAngles + aimPunch
                        );

                        const auto fov = std::hypot(angle.x, angle.y);

                        if (fov < bestFov)
                        {
                            bestFov = fov;
                            bestAngle = angle;
                        }
                    }
                }
                // if we have a best angle, do aimbot
                if (!bestAngle.IsZero())
                    mem.Write<Vector3>(clientState + offsets::signatures::dwClientState_ViewAngles, viewAngles + bestAngle / globals::aimassistSmooth); // smoothing
        }
        if (globals::trigger)
        {
            int iCrosshairId = mem.Read<int>(localPlayer + offsets::netvars::m_iCrosshairId);
            if (GetAsyncKeyState(VK_XBUTTON2)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(globals::ms));
                for (int id = 1; id <= 64; id++) {
                    if (iCrosshairId > 0 && iCrosshairId <= 64) {
                        DWORD entity = mem.Read<DWORD>(globals::clientAdress + offsets::signatures::dwEntityList + ((iCrosshairId - 1) * 0x10));
                        int iTeamNum = mem.Read<int>(entity + offsets::netvars::m_iTeamNum);
                        int playerTeamNum = mem.Read<int>(localPlayer + offsets::netvars::m_iTeamNum);
                        if (iTeamNum != playerTeamNum) { // check if entity is on enemy team
                            mem.Write(globals::clientAdress + offsets::signatures::dwForceAttack, 6);
                            std::this_thread::sleep_for(std::chrono::milliseconds(globals::sprayTime));
                            mem.Write(globals::clientAdress + offsets::signatures::dwForceAttack, 4);
                        }
                        
                    }
                    
                }
            }
        }
        if (globals::bhop)
        {
            if (GetAsyncKeyState(VK_SPACE)) {
                DWORD fFlags = mem.Read<DWORD>(localPlayer + offsets::netvars::m_fFlags);
                if (fFlags == 257) {
                    mem.Write(globals::clientAdress + offsets::signatures::dwForceJump, 6);
                }
                else
                {
                    mem.Write(globals::clientAdress + offsets::signatures::dwForceJump, 4);
                }
            }
        }
        if (globals::noFlash)
        {
            float flFlashDuration = mem.Read<float>(localPlayer + offsets::netvars::m_flFlashDuration);
            if (flFlashDuration != 0.f) {
                // change flash duration to zero
                if (globals::changeFlash)
                {
                    mem.Write(localPlayer + offsets::netvars::m_flFlashMaxAlpha, globals::strenght);
                }
                else if(globals::removeFlash) {
                    mem.Write(localPlayer + offsets::netvars::m_flFlashDuration, 0.f);
                }
            }
            
        }
        if (globals::noRecoil)
        {
            const auto& shotsFired = mem.Read<std::int32_t>(localPlayer + offsets::netvars::m_iShotsFired);
            if (shotsFired)
            {
                const auto& clientState = mem.Read<std::uintptr_t>(globals::engineAdress + offsets::signatures::dwClientState);
                const auto& viewAngles = mem.Read<Vector2>(clientState + offsets::signatures::dwClientState_ViewAngles);

                const auto& aimPunch = mem.Read<Vector2>(localPlayer + offsets::netvars::m_aimPunchAngle);

                auto newAngles = Vector2{
                    viewAngles.x + oldPunch.x - aimPunch.x * 2.f,
                    viewAngles.y + oldPunch.y - aimPunch.y * 2.f,
                };

                if (newAngles.x > 89.f)
                {
                    newAngles.x = 89.f;
                }
                if (newAngles.x < -89.f)
                {
                    newAngles.x = -89.f;
                }
                while (newAngles.y > 180.f)
                {
                    newAngles.y -= 360.f;
                }
                while (newAngles.y < -180.f)
                {
                    newAngles.y += 360.f;
                }

                mem.Write<Vector2>(clientState + offsets::signatures::dwClientState_ViewAngles, newAngles);

                oldPunch.x = aimPunch.x * 2.f;
                oldPunch.y = aimPunch.y * 2.f;

            }
            else {
                oldPunch.x = oldPunch.y = 0.f;
            } 
        }
        if (globals::fovChanger)
        {
            mem.Write<int>(localPlayer + offsets::netvars::m_iDefaultFOV, globals::fov);
        }

        GetClientRect(hwnd, &clientRect);
        if (globals::crossh) {
            const auto& aimPunch = mem.Read<Vector2>(localPlayer + offsets::netvars::m_aimPunchAngle);
            resolution = {
                clientRect.right - clientRect.left,
                clientRect.bottom - clientRect.top
            };
            crosshairPos.x = (int32_t)(clientRect.left + (resolution.x / 2.0f) - (resolution.x / 90.f * aimPunch.y));
            crosshairPos.y = (int32_t)(clientRect.top + (resolution.y / 2.0f) - (resolution.y / 90.f * -aimPunch.x));
            crosshairRect = {
                crosshairPos.x - crosshairSize / 2,
                crosshairPos.y - crosshairSize / 2,
                crosshairPos.x + crosshairSize / 2,
                crosshairPos.y + crosshairSize / 2
            };
            FillRect(hdc, &crosshairRect, hbrush);
        }

        if (globals::chams)
        {
            const auto& localTeam = mem.Read<std::int32_t>(localPlayer + offsets::netvars::m_iTeamNum);
            constexpr const auto enemyColor = Color{ 255, 0 , 0 };
            constexpr const auto teamColor = Color{ 0, 0, 255 };
            
            for (auto i = 1; i <= 32; i++)
            {
                const auto& entity = mem.Read<std::uintptr_t>(globals::clientAdress + offsets::signatures::dwEntityList + i * 0x10);
                const auto& alive = mem.Read<int>(entity + offsets::netvars::m_iHealth);
                if (alive > 0) {
                    if (mem.Read<std::int32_t>(entity + offsets::netvars::m_iTeamNum) == localTeam)
                    {
                        mem.Write<Color>(entity + offsets::netvars::m_clrRender, teamColor);
                    }
                    else
                    {
                        mem.Write<Color>(entity + offsets::netvars::m_clrRender, enemyColor);
                    }
                }

            }
        }
        constexpr const auto defEnemy = Color{ 0, 0, 0 };
        constexpr const auto defTeam = Color{ 255, 255, 255 };
        if (!globals::chams)
        {
            for (auto i = 1; i <= 32; i++)
            {
                const auto& entity = mem.Read<std::uintptr_t>(globals::clientAdress + offsets::signatures::dwEntityList + i * 0x10);
                const auto& alive = mem.Read<int>(entity + offsets::netvars::m_iHealth);
                if (alive > 0) {
                    if (mem.Read<std::int32_t>(entity + offsets::netvars::m_iTeamNum) == localTeam)
                    {
                        mem.Write<Color>(entity + offsets::netvars::m_clrRender, defEnemy);
                    }
                    else
                    {
                        mem.Write<Color>(entity + offsets::netvars::m_clrRender, defTeam);
                    }
                }
            }
        }
        if (globals::addBright)
        {
            const auto _this = static_cast<std::uintptr_t>(globals::engineAdress + offsets::signatures::model_ambient_min - 0x2c);
            mem.Write<std::int32_t>(globals::engineAdress + offsets::signatures::model_ambient_min, *reinterpret_cast<std::uintptr_t*>(&globals::brightness) ^ _this);
        }

        if (globals::fovChanger)
        {
            mem.Write<int>(localPlayer + offsets::netvars::m_iDefaultFOV, globals::fov);
        }
        if (!globals::fovChanger)
        {
            mem.Write<int>(localPlayer + offsets::netvars::m_iDefaultFOV, 90);
        }
        if (!globals::addBright)
        {
            const auto _this = static_cast<std::uintptr_t>(globals::engineAdress + offsets::signatures::model_ambient_min - 0x2c);
            mem.Write<std::int32_t>(globals::engineAdress + offsets::signatures::model_ambient_min, *reinterpret_cast<std::uintptr_t*>(&globals::a) ^ _this);
        }
        if (globals::autoPistol)
        {
            uintptr_t active_weapon = mem.Read<uintptr_t>(localPlayer + offsets::netvars::m_hActiveWeapon) & 0xFFF;
            uintptr_t active_weapon_entity = mem.Read<uintptr_t>(globals::clientAdress + offsets::signatures::dwEntityList + (active_weapon - 1) * 0x10);
            short weaponID = mem.Read<short>(active_weapon_entity + offsets::netvars::m_iItemDefinitionIndex);

            if (weaponID == 1 || weaponID == 2 || weaponID == 3 || weaponID == 4 || weaponID == 30 || weaponID == 36 || weaponID == 63 || weaponID == 32 || weaponID == 61)
            {
                if (GetAsyncKeyState(VK_LBUTTON))
                {
                    mem.Write(globals::clientAdress + offsets::signatures::dwForceAttack, 6);
                }
            }
        }
    }
}
