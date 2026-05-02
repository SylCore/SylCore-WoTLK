/*
 * This file is part of the SylCore Project. See AUTHORS file for Copyright information
 * https://sylcore.org
 * https://www.youtube.com/@DEVSylian
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include "Define.h"
#include "Object.h"
#include "Position.h"
#include <set>
#include <functional>

class Player;

/**
 * A completely runtime, non‑DBC, non‑spell circular zone that fires
 * OnEnter/OnLeave callbacks when a player crosses its boundary.
 *
 * INSTANCE LIFECYCLE:
 *  1. Create the trigger with a map ID, position, and radius.
 *  2. Set your custom callbacks (lambdas, member functions, or free functions).
 *  3. Register it with DynamicAreaTriggerMgr::AddTrigger().
 *  4. The manager will automatically call OnPlayerEnter / OnPlayerLeave
 *     whenever a player moves into or out of the zone.
 *  5. When you no longer need the zone (e.g., tent despawns), call
 *     RemoveTrigger(). The manager holds a shared_ptr so it will stay
 *     alive until all references are gone.
 *
 * PERFORMANCE DESIGN:
 *  This class itself does no scanning. All detection is handled by the
 *  DynamicAreaTriggerMgr using the same grid cell partition as the core
 *  maps (SIZE_OF_GRIDS). Enter/leave events fire only when a player
 *  actually moves; there is no per‑tick timer and no global polling.
 *
 * THREAD SAFETY:
 *  All map updates occur under the map’s update lock. AddTrigger,
 *  RemoveTrigger, and UpdatePlayerPosition must all be called from
 *  the map thread (the same thread that processes player movement).
 */
class DynamicAreaTrigger
{
public:
    // Signature for your enter/leave logic
    using EnterCallback = std::function<void(Player*)>;
    using LeaveCallback = std::function<void(Player*)>;

    /**
     * @param mapId   ID of the map this trigger lives on.
     * @param x,y,z   World coordinates of the centre.
     * @param radius  Trigger radius in yards.
     */
    DynamicAreaTrigger(uint32 mapId, float x, float y, float z, float radius)
        : _mapId(mapId), _pos(x, y, z), _radius(radius),
        _radiusSq(radius* radius) // Precomputed to avoid sqrt in distance checks
    {
    }

    virtual ~DynamicAreaTrigger() = default;

    // ----- Setters (set callbacks before or after registration) -----
    void SetEnterCallback(EnterCallback cb) { _onEnter = std::move(cb); }
    void SetLeaveCallback(LeaveCallback cb) { _onLeave = std::move(cb); }

    // ----- Getters used by the manager -----
    uint32 GetMapId() const { return _mapId; }
    Position const& GetPos() const { return _pos; }
    float GetRadius() const { return _radius; }
    float GetRadiusSq() const { return _radiusSq; }

    // Access to the set of players currently inside (const view)
    [[nodiscard]] const std::set<ObjectGuid>& GetInsidePlayers() const { return _insidePlayers; }

    // ----- Core logic called ONLY by DynamicAreaTriggerMgr -----
    /**
     * Called when a player who was not inside is now inside.
     * Inserts the GUID and fires the OnEnter callback if set.
     */
    void OnPlayerEnter(Player* player);

    /**
     * Called when a player who was inside is now outside.
     * Erases the GUID and fires the OnLeave callback if set.
     */
    void OnPlayerLeave(Player* player);

    /**
     * Force removal of a player from the inside set WITHOUT firing callbacks.
     * Used when a player leaves the map (teleport, logout, etc.) to prevent
     * stale entries.
     */
    void CleanPlayer(Player* player);

    // ----- Simple distance test (horizontal only, add Z if needed) -----
    bool IsPlayerInside(Player* player) const;

private:
    uint32         _mapId;
    Position       _pos;
    float          _radius;
    float          _radiusSq;   // squared for fast distance check

    EnterCallback  _onEnter;
    LeaveCallback  _onLeave;

    // Set of GUIDs of players currently inside the trigger.
    // Modified ONLY under the map's update lock (single‑threaded).
    std::set<ObjectGuid> _insidePlayers;
};
