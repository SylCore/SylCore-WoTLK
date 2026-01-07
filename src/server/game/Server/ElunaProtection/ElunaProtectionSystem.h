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

#pragma once
#ifndef ELUNA_PROTECTION_SYSTEM_H
#define ELUNA_PROTECTION_SYSTEM_H
#include <string>
#include <vector>
#include <unordered_map>


class ElunaProtectionSystem
{
public:
	static ElunaProtectionSystem* instance();

	void RunElunaProtectionSystemCheck();
	
	bool HasProtectedScripts();

	enum class VerifyResult { OK, FILE_NOT_FOUND, HASH_MISMATCH, NOT_PROTECTED };
	VerifyResult VerifyScript(const std::string& scriptPath);

	bool VerifyAllElunaScripts();

	void OnScriptTampered();
private:
	ElunaProtectionSystem();

	std::string ComputeFileHash(const std::string& filePath);

	std::string BytesToHex(const unsigned char* bytes, size_t length);



    /*
     * Guide on how the Eluna Protection System works.
     *
     * How to add new files to be protected?
     *  - First, you will have to add it to the "m_protectedFiles" map.
     *  - Below you will see how we would add a file, and its checksum needed to verify the file.
     *  {"NameOfFile.lua", "ChecksumCode"},
     *
     * 
     *  - If the file is inside a folder, you will need to do the path to it.
     *  {"NameOfFolder/NameOfFile.lua", "ChecksumCode"},
     *
     * 
     * Use this website to get the checksum
     * https://emn178.github.io/online-tools/sha256_checksum.html
     *
     *
     *
     * How to change the message on tamper?
     *  - All you have to do is change the TamperBanner.
     *  - It can be anything you want, banner, or even normal text, have fun!
     *
    */

    // Message sent to the console when tamper has been detected.
    const char* TamperBanner = R"(

   ███████╗██╗     ██╗   ██╗███╗   ██╗ █████╗ 
   ██╔════╝██║     ██║   ██║████╗  ██║██╔══██╗
   █████╗  ██║     ██║   ██║██╔██╗ ██║███████║
   ██╔══╝  ██║     ██║   ██║██║╚██╗██║██╔══██║
   ███████╗███████╗╚██████╔╝██║ ╚████║██║  ██║
   ╚══════╝╚══════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═╝
-----------------------------------------------------
        Protected Eluna Script Integrity Violation
        Unauthorized modification detected.
-----------------------------------------------------
)";


	// Here you will put your protected files in, please always make sure to take the path to the file, then the hash, I will leave examples.
	std::unordered_map<std::string /*scriptPath*/, std::string /*expectedHash*/> m_protectedFiles = {

	};

#define sElunaProtectionSys ElunaProtectionSystem::instance()
};

#endif
