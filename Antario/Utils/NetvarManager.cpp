#include "NetvarManager.h"
#include "..\SDK\IBaseClientDll.h"

namespace sdk::util
{
	using netvar_key_value_map = std::unordered_map< uint32_t, uintptr_t >;
	using netvar_table_map = std::unordered_map< uint32_t, netvar_key_value_map >;
	void initProps(netvar_table_map &table_map);

	uintptr_t getNetVar(const uint32_t table,
		const uint32_t prop)
	{
		static netvar_table_map map = {};
		if (map.empty())
		{
			initProps(map);
		}

		if (map.find(table) == map.end())
		{
			return 0;
		}

		netvar_key_value_map &tableMap = map.at(table);
		if (tableMap.find(prop) == tableMap.end())
		{
			return 0;
		}

		return tableMap.at(prop);
	}

	void addPropsForTable(netvar_table_map &table_map, const uint32_t table_name_hash, const std::string &table_name, classes::recv_table *table, const bool dump_vars, std::map< std::string, std::map< uintptr_t, std::string > > &var_dump, const size_t child_offset = 0)
	{
		for (auto i = 0; i < table->num_props; ++i)
		{
			auto &prop = table->props[i];

			if (prop.data_table && prop.num_elements > 0)
			{
				if (std::string(prop.name).substr(0, 1) == std::string("0"))
					continue;

				addPropsForTable(table_map,
					table_name_hash,
					table_name,
					prop.data_table,
					dump_vars,
					var_dump,
					prop.offset + child_offset);
			}

			auto name = std::string(prop.name);

			if (name.substr(0, 1) != "m" /*&& name.substr( 0, 1 ) != "b"*/)
				continue;

			const auto name_hash = fnv::hash(prop.name);
			const auto offset = uintptr_t(prop.offset) + child_offset;

			table_map[table_name_hash][name_hash] = offset;

			if (dump_vars)
			{
				var_dump[table_name][offset] = prop.name;
			}
		}
	}

	void initProps(netvar_table_map &table_map)
	{
		const auto dump_vars = true;  //true if netvar dump

		std::map< std::string, std::map< uintptr_t, std::string > > var_dump;
		for (auto client_class = g_pClientDll->GetAllClasses();
			client_class;
			client_class = client_class->pNext)
		{
			const auto table = reinterpret_cast<classes::recv_table*>(client_class->pRecvTable);
			const auto table_name = table->net_table_name;
			const auto table_name_hash = fnv::hash(table_name);

			if (table == nullptr)
				continue;

			addPropsForTable(table_map, table_name_hash, table_name, table, dump_vars, var_dump);
		}
#ifdef _DEBUG
		write_netvar_dump(var_dump);
#endif
#ifdef PRINT_NETVARS
		print_map(var_dump);
#endif
	}
}
