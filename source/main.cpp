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

#include "elfio/elfio.hpp"
#include "library/library.h"
#include "version.h"
#include "wiiu_zlib.hpp"
#include <wums.h>

#define VERSION        "v0.1"
#define LAST_ERROR_LEN 256

WUMS_MODULE_EXPORT_NAME("homebrew_dll");
WUMS_MODULE_SKIP_INIT_FINI();
WUMS_MODULE_INIT_BEFORE_RELOCATION_DONE_HOOK();

static char last_error[LAST_ERROR_LEN] = {};
static bool has_error                  = false;

static const char *ERR_BAD_RPL    = "Does not seem to be a library";
static const char *ERR_BAD_HANDLE = "Expected a library handle but got a nullptr";
static const char *ERR_BAD_SYM    = "Symbol name is null or empty";

static void set_error(const char *error_message);

WUMS_INITIALIZE(args) {
    static bool setup_needed = true;

    if (!setup_needed)
        return;

    setup_needed = false;
}

WUMS_APPLICATION_STARTS() {
    OSReport("Running DynamicLibraryModule " VERSION VERSION_EXTRA "\n");
#ifdef DEBUG
    initLogging();
#endif
}

#ifdef DEBUG
WUMS_APPLICATION_REQUESTS_EXIT() {
    deinitLogging();
}
#endif

void *dlopen(const char *library) {
    dl_handle *handle = new dl_handle();
    ELFIO::elfio reader(new wiiu_zlib());
    if (!reader.load(library)) {
        set_error(ERR_BAD_RPL);
        delete handle;
        return nullptr;
    }

    LibraryLoader loader(handle, reader);
    if (!loader.load()) {
        set_error(loader.error_message());
        delete handle;
        return nullptr;
    }

    return (void *) handle;
}

void *dlsym(void *handle, const char *symbol) {
    if (handle == nullptr) {
        set_error(ERR_BAD_HANDLE);
        return nullptr;
    }

    if (symbol == nullptr || *symbol == '\0') {
        set_error(ERR_BAD_SYM);
        return nullptr;
    }

    SymbolResolver resolver((dl_handle *) handle);

    uint32_t symbol_address = resolver.resolve(symbol);
    if (symbol_address == 0) {
        set_error(resolver.error_message());
        return nullptr;
    }

    return (void *) symbol_address;
}

char *dlerror() {
    char *error_message = has_error ? &last_error[0] : nullptr;
    has_error           = false;

    return error_message;
}

int dlclose(void *handle) {
    dl_handle *h = (dl_handle *) handle;
    if (h)
        delete h;
    return 0;
}

static void set_error(const char *error_message) {
    snprintf(last_error, LAST_ERROR_LEN, "%s", error_message);
    has_error = true;
}

uint32_t DynamicLibraryOpen __attribute__((__section__(".data")))   = (uint32_t) dlopen;
uint32_t DynamicLibraryClose __attribute__((__section__(".data")))  = (uint32_t) dlclose;
uint32_t DynamicLibrarySymbol __attribute__((__section__(".data"))) = (uint32_t) dlsym;
uint32_t DynamicLibraryError __attribute__((__section__(".data")))  = (uint32_t) dlerror;

WUMS_EXPORT_DATA(DynamicLibraryOpen);
WUMS_EXPORT_DATA(DynamicLibraryClose);
WUMS_EXPORT_DATA(DynamicLibrarySymbol);
WUMS_EXPORT_DATA(DynamicLibraryError);