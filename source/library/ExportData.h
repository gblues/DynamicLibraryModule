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

#include "../elfio/elfio.hpp"
#include "library.h"

class ExportData {
public:
    ExportData(const char *&export_section_data, ELFIO::endianess_convertor &convertor, const char *base_addr) {
        function_offset      = read_uint32_t(export_section_data, convertor);
        uint32_t name_offset = read_uint32_t(export_section_data, convertor);

        if (function_offset > 0x02000000 && function_offset < 0x10000000)
            function_offset -= 0x02000000;
        else if (function_offset >= 0x10000000 && function_offset < 0xC0000000)
            function_offset -= 0x10000000;

        name = std::string(base_addr + name_offset);
    }

    [[nodiscard]] uint32_t getFunctionOffset() const {
        return function_offset;
    }

    [[nodiscard]] std::string getName() const {
        return name;
    }

private:
    uint32_t read_uint32_t(const char *&export_section_data, ELFIO::endianess_convertor &convertor) {
        union _int32buffer {
            uint32_t word;
            char buf[4];
        } int32buffer = {0};
        memcpy(int32buffer.buf, export_section_data, 4);
        export_section_data += 4;
        int32buffer.word = convertor(int32buffer.word);

        return int32buffer.word;
    }
    uint32_t function_offset;
    std::string name;
};