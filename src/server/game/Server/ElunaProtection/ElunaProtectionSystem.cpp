/*
 * This file is part of the SylCore Project. See AUTHORS file for Copyright information
 * https://sylcore.org
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

#include "ElunaProtectionSystem.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <Log.h>
#include <World.h>
#include <filesystem>
#include <Config.h>

ElunaProtectionSystem::ElunaProtectionSystem(){}

std::string m_luaScriptsPath;
std::string m_lua_path_extra;
std::string m_lua_cpath_extra;

ElunaProtectionSystem* ElunaProtectionSystem::instance()
{
    static ElunaProtectionSystem instance;
    return &instance;
}

void ElunaProtectionSystem::RunElunaProtectionSystemCheck()
{
    m_luaScriptsPath = sConfigMgr->GetOption<std::string>("Eluna.ScriptPath", "lua_scripts");
    m_lua_path_extra = sConfigMgr->GetOption<std::string>("Eluna.RequirePaths", "");
    m_lua_cpath_extra = sConfigMgr->GetOption<std::string>("Eluna.RequireCPaths", "");

    if (!HasProtectedScripts()) {
        return;
    }

    // If one of the scripts failed the check.
    if (!VerifyAllElunaScripts())
        OnScriptTampered();
    
}

bool ElunaProtectionSystem::HasProtectedScripts()
{
    if (m_protectedFiles.size() == 0) {
        return false;
    }

    return true;
}

std::string ElunaProtectionSystem::BytesToHex(const unsigned char* bytes, size_t length)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return ss.str();
}


std::string ElunaProtectionSystem::ComputeFileHash(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return "";
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer))) {
        SHA256_Update(&sha256, buffer, file.gcount());
    }

    SHA256_Update(&sha256, buffer, file.gcount());

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    return BytesToHex(hash, SHA256_DIGEST_LENGTH);
}


ElunaProtectionSystem::VerifyResult
ElunaProtectionSystem::VerifyScript(const std::string& scriptPath)
{
    auto it = m_protectedFiles.find(scriptPath);
    if (it == m_protectedFiles.end()) {
        return VerifyResult::NOT_PROTECTED;
    }
    
    std::string updatedWithPath = m_luaScriptsPath + "/" + scriptPath;

    std::string expectedHash = it->second;
    std::string actualHash = ComputeFileHash(updatedWithPath);

    if (actualHash.empty()) {
        return VerifyResult::FILE_NOT_FOUND;
    }

    if (actualHash != expectedHash) {
        return VerifyResult::HASH_MISMATCH;
    }

    return VerifyResult::OK;
}

bool ElunaProtectionSystem::VerifyAllElunaScripts()
{

    bool allValid = true;

    for (const auto& pair : m_protectedFiles) {
        VerifyResult result = VerifyScript(pair.first);

        switch (result) {
        case VerifyResult::OK:
            break;
        case VerifyResult::HASH_MISMATCH:
            allValid = false;
            break;
        case VerifyResult::FILE_NOT_FOUND:
            allValid = false;
            break;
            
        }
    }

    return allValid;
}

void ElunaProtectionSystem::OnScriptTampered()
{
    sWorld->ShutdownServ(10, SHUTDOWN_EXIT_CODE, 0, TamperBanner);
}
