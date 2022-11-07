#pragma once

/****************************************************************************
 * Copyright (C) 2018-2020 Nathan Strong
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include "Loader.h"

class SymbolResolver {
public:
    SymbolResolver(dl_handle *handle) : handle(handle) {}
    ~SymbolResolver() = default;

    uint32_t resolve(const char *name);
    const char *error_message();

private:
    dl_handle *handle;
    std::string error;
};