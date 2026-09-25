#pragma once
#include "nicolai/address_space.hpp"
#include "nicolai/static_files.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

struct ResourceRegistryHeader {
    bool valid = false;
    std::uint32_t logical_offset = 0;
    std::uint32_t aux_logical = 0;
    std::uint32_t base_dir_logical = 0;
    std::uint32_t head_logical = 0;
    std::string base_dir;
};

struct ResourceRegistryEntry {
    std::uint32_t node_logical = 0;
    std::uint32_t name_logical = 0;
    std::uint32_t path_logical = 0;
    std::uint32_t next_logical = 0;
    std::string name;
    std::string path;
};

struct ResourceRegistry {
    bool valid = false;
    bool terminated = false;
    ResourceRegistryHeader header{};
    std::vector<ResourceRegistryEntry> entries;
};

ResourceRegistry parse_exec_resource_registry(const std::vector<std::uint8_t>& bytes,
                                              const EdatLayout& layout,
                                              const StaticFileRecord& rsrc,
                                              std::size_t max_entries = 4096);
const ResourceRegistryEntry* find_resource(const ResourceRegistry& registry,
                                           const std::string& name);

} // namespace nicolai
