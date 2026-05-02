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

#include "DynamicAreaTrigger.h"
#include "Player.h"

bool DynamicAreaTrigger::IsPlayerInside(Player* player) const
{
    float dx = player->GetPositionX() - _pos.GetPositionX();
    float dy = player->GetPositionY() - _pos.GetPositionY();
    // X/Y horizontal check only (no Z). For tents this is perfect.
    return (dx * dx + dy * dy) <= _radiusSq;
}

void DynamicAreaTrigger::OnPlayerEnter(Player* player)
{
    auto [iter, inserted] = _insidePlayers.insert(player->GetGUID());
    if (!inserted)
        return; // Already inside; nothing to do

    // Fire the user‑supplied callback, if any
    if (_onEnter)
        _onEnter(player);
}

void DynamicAreaTrigger::CleanPlayer(Player* player)
{
    _insidePlayers.erase(player->GetGUID());
}

void DynamicAreaTrigger::OnPlayerLeave(Player* player)
{
    if (_insidePlayers.erase(player->GetGUID()) == 0)
        return; // Was not inside; nothing to do

    if (_onLeave)
        _onLeave(player);
}
