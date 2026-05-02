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
#include "DynamicAreaTrigger.h"
#include <unordered_map>
#include <vector>
#include <memory>

class Map;
class Player;

/**
 * Singleton manager that handles all runtime DynamicAreaTriggers.
 *
 * HOW IT LEVERAGES THE EXISTING GRID SYSTEM:
 *  The game world is already split into grid cells of SIZE_OF_GRIDS yards
 *  (defined in GridDefines.h, typically 32 yards). This manager uses that
 *  same cell size to spatially index triggers. When a trigger is added, we
 *  compute which cells it overlaps and register it there. When a player moves,
 *  we look only at triggers in the player's current cell and the eight
 *  immediate neighbours – exactly the same cells the grid already uses to
 *  determine active objects. This gives O(1) per‑move cost without touching
 *  the rest of the map.
 *
 * USAGE FLOW:
 *  1. Create a DynamicAreaTrigger (shared_ptr recommended) and set callbacks.
 *  2. Call AddTrigger(trigger) – it will be stored and start responding
 *     to player movement.
 *  3. The core must call UpdatePlayerPosition(player) from within
 *     Player::UpdatePosition() (after the grid/cell update).
 *  4. When the source object (e.g., tent) is destroyed, call RemoveTrigger().
 *  5. Additionally, call RemovePlayerFromAll() when a player leaves the map
 *     (logout, teleport) to avoid stale inside‑player entries.
 *
 * THREADING NOTE:
 *  All methods are designed to be called from the map update thread only.
 *  This is the thread that runs Map::Update() and processes player movement.
 *  No external synchronisation is needed.
 */
class DynamicAreaTriggerMgr
{
public:
    static DynamicAreaTriggerMgr* instance();

    // ---- Lifetime management ----
    void AddTrigger(std::shared_ptr<DynamicAreaTrigger> trigger);
    void RemoveTrigger(DynamicAreaTrigger* trigger);

    // Must be called from Player::UpdatePosition after grid/cell updates
    void UpdatePlayerPosition(Player* player);

    // Clean up a player from all triggers on their current map (logout/teleport)
    void RemovePlayerFromAll(Player* player);

private:
    // ---------- Internal spatial index (identical to grid cell partitioning) ----------
    struct CellCoord
    {
        int32 x, y;
        bool operator==(CellCoord const& o) const { return x == o.x && y == o.y; }
    };

    struct CellCoordHash
    {
        std::size_t operator()(CellCoord const& c) const
        {
            return std::hash<int32>()(c.x) ^ (std::hash<int32>()(c.y) << 1);
        }
    };

    // MapId -> list of all triggers + cell‑based lookup table
    struct MapData
    {
        std::vector<std::shared_ptr<DynamicAreaTrigger>> allTriggers;
        std::unordered_map<CellCoord, std::vector<std::shared_ptr<DynamicAreaTrigger>>, CellCoordHash> cellIndex;
    };

    // Convert world coordinates to cell coordinates using SIZE_OF_GRIDS
    static CellCoord PositionToCell(float x, float y);

    // Register a trigger to all cells its circle touches
    void RegisterToCells(MapData& data, std::shared_ptr<DynamicAreaTrigger> trigger);

    // Remove a trigger from the cell index (simple full scan; rare operation)
    void UnregisterFromCells(MapData& data, std::shared_ptr<DynamicAreaTrigger> trigger);

    std::unordered_map<uint32, MapData> _mapData; // one entry per map
};

#define sDynamicAreaTriggerMgr DynamicAreaTriggerMgr::instance()
