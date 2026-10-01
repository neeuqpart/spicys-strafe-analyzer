#include "../Interfaces/Interfaces.h"
#include "netvars.h"
#include "fnv1.h"

//antario if i remember correctly
namespace netvar_manager {
	using netvar_key_value_map = std::unordered_map< uint32_t, uintptr_t >;
	using netvar_table_map = std::unordered_map< uint32_t, netvar_key_value_map >;
	void initialize_props(netvar_table_map& table_map);

	uintptr_t get_net_var(const uint32_t table, const uint32_t prop) {
		static netvar_table_map map = {};
		if (map.empty())
			initialize_props(map);

		if (table != 0) {
			auto it = map.find(table);
			if (it != map.end()) {
				auto prop_it = it->second.find(prop);
				if (prop_it != it->second.end())
					return prop_it->second;
			}
		}

		return 0;
	}

	void add_props_for_table(netvar_table_map& table_map, const uint32_t table_name_hash, const std::string& table_name, RecvTable* table, const bool dump_vars, std::map< std::string, std::map< uintptr_t, std::string > >& var_dump, const size_t child_offset = 0) {
		for (auto i = 0; i < table->m_nProps; ++i) {
			auto& prop = table->m_pProps[i];

			if (prop.m_pDataTable) {
				if (!prop.m_pVarName || prop.m_pVarName[0] == '0')
					continue;

				add_props_for_table(table_map, table_name_hash, table_name, prop.m_pDataTable, dump_vars, var_dump, prop.m_Offset + child_offset);
			}

			if (!prop.m_pVarName)
				continue;

			if (prop.m_pVarName[0] != 'm' && prop.m_pVarName[0] != 'b')
				continue;

			const auto name_hash = fnv::hash(prop.m_pVarName);
			const auto offset = uintptr_t(prop.m_Offset) + child_offset;

			table_map[table_name_hash][name_hash] = offset;
			// Store offsets only under the owning root. A nested table can
			// appear in other entities at different offsets; aliasing it here
			// overwrote the player's velocity/flags with unrelated fields.

			if (dump_vars)
				var_dump[table_name][offset] = prop.m_pVarName;
		}
	}

	void initialize_props(netvar_table_map& table_map) {
		const auto dump_vars = false;

		std::map< std::string, std::map< uintptr_t, std::string > > var_dump;
		for (auto client_class = Interfaces::client->get_client_classes();
			client_class;
			client_class = client_class->m_pNext) {
			const auto table = reinterpret_cast<RecvTable*>(client_class->m_pRecvTable);
			if (!table || !table->m_pNetTableName)
				continue;
			const auto table_name = table->m_pNetTableName;
			const auto table_name_hash = fnv::hash(table_name);

			add_props_for_table(table_map, table_name_hash, table_name, table, dump_vars, var_dump);
		}
	}
}
