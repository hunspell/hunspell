/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <hunspell/hunspell.hxx>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>

std::vector<Hunspell*> dictionaries;

bool endswith(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    char* exe_path = (*argv)[0];
    // dirname() can modify its argument.
    char* exe_path_copy = strdup(exe_path);
    char* dir = dirname(exe_path_copy);
    DIR* d = opendir(dir);
    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL)
    {
        std::string entry(direntry->d_name);
        if (endswith(entry, ".aff"))
        {
            std::string dic = entry.substr(0, entry.size() - 4) + ".dic";
            dictionaries.push_back(new Hunspell(entry.c_str(), dic.c_str()));
        }
    }
    closedir(d);
    free(exe_path_copy);

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const char* data, size_t size)
{
    std::string word(data, size);
    for (std::vector<Hunspell*>::const_iterator it = dictionaries.begin(); it != dictionaries.end(); ++it)
    {
        Hunspell *dict = *it;
        if (!dict->spell(word))
            dict->suggest(word);
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
