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

#include <coreinit/dynload.h>
#include <map>
#include <memory/mappedmemory.h>
#include <memory>
#include <whb/log_console.h>

#include "../elfio/elfio.hpp"
#include "ExportData.h"
#include "LibraryData.h"

typedef int (*rpl_entrypoint_fn)(void *handle, int reason);

typedef struct dl_handle_t {
    void *library;
    size_t library_size;
    LibraryData library_data;
    rpl_entrypoint_fn entrypoint;
    std::map<std::string, uint32_t> exports;

    dl_handle_t() : library_data(LibraryData()) {}
    ~dl_handle_t() {
        if (library) {
            if (this->entrypoint != nullptr) {
                this->entrypoint(nullptr, OS_DYNLOAD_UNLOADED);
            }
            MEMFreeToMappedMemory(library);
            library = nullptr;
        }
    }
} dl_handle;

struct f_export_header {
    uint32_t num_entries;
    uint32_t id;
};

class LibraryLoader {
public:
    LibraryLoader(dl_handle *h, ELFIO::elfio &r) : handle(h),
                                                   reader(r),
                                                   destinations(std::unique_ptr<uint8_t *[]>(new uint8_t *[r.sections.size()])) {}
    ~LibraryLoader() = default;

    bool load();
    const char *error_message();

private:
    bool allocate_memory();
    void parse_library_metadata();
    void init_sections();
    bool link_sections();
    bool link_section(uint32_t section_index);
    void add_relocation_data();
    void resolve_exports();
    bool process_relocations();
    void parse_exports(const char *export_section_data);

    bool has_executed = false;
    bool result       = false;
    dl_handle *handle;
    ELFIO::elfio &reader;
    size_t code_size = 0;
    std::unique_ptr<uint8_t *[]> destinations;
    std::vector<ELFIO::section *> code_sections;
    std::vector<ELFIO::section *> relocation_sections;
    std::vector<ExportData> export_entries;
    std::map<uint32_t, std::string> import_names;
    std::string error;

    uint32_t base_offset = 0;
    uint32_t text_offset = 0;
    uint32_t data_offset = 0;
    uint32_t entrypoint  = 0;
};