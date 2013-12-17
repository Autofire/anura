/*
	Copyright (C) 2003-2013 by David White <davewx7@gmail.com>
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef NO_EDITOR
#include <iostream>

#include "editor_variable_info.hpp"
#include "foreach.hpp"
#include "formula.hpp"
#include "string_utils.hpp"
#include "variant_utils.hpp"

namespace {
const decimal DefaultMinValue(decimal::from_int(-100));
const decimal DefaultMaxValue(decimal::from_int(100));
}

editor_variable_info::editor_variable_info(variant node)
  : name_(node["name"].as_string()),
    is_property_(false),
    type_(TYPE_INTEGER), info_(node["info"].as_string_default()),
    help_(node["help"].as_string_default()),
    formula_(game_logic::formula::create_optional_formula(node["value"])),
	numeric_decimal_(false),
	numeric_min_(node["min_value"].as_decimal(DefaultMinValue)),
	numeric_max_(node["max_value"].as_decimal(DefaultMaxValue))
{
	ASSERT_LOG(numeric_max_ > numeric_min_, "EDITOR max_value <= min_value: " << node.write_json());

	const std::string& type = node["type"].as_string_default();
	if(type == "x") {
		type_ = XPOSITION;
		//std::cerr << "XPOS VARIABLE\n";
	} else if(type == "y") {
		type_ = YPOSITION;
	} else if(type == "level") {
		type_ = TYPE_LEVEL;
	} else if(type == "label") {
		type_ = TYPE_LABEL;
	} else if(type == "text" || type == "string") {
		type_ = TYPE_TEXT;
	} else if(type == "boolean") {
		type_ = TYPE_BOOLEAN;
	} else if(type == "enum") {
		type_ = TYPE_ENUM;
		if(node["enum_values"].is_list()) {
			enum_values_ = node["enum_values"].as_list_string();
		} else {
			enum_values_ = util::split(node["enum_values"].as_string());
		}
		ASSERT_LOG(enum_values_.empty() == false, "IN PROPERTY: " << name_ << " ENUM WITH NO VALUES SPECIFIED");
	} else if(type == "points") {
		type_ = TYPE_POINTS;
	} else if(type == "decimal") {
		numeric_decimal_ = true;
	}
}

variant editor_variable_info::write() const
{
	variant_builder node;
	node.add("name", name_);
	if(info_.empty() == false) {
		node.add("info", info_);
	}

	if(numeric_decimal_) {
		node.add("type", "decimal");
	}

	if(numeric_min_ != DefaultMinValue) {
		node.add("min_value", numeric_min_);
	}

	if(numeric_max_ != DefaultMaxValue) {
		node.add("max_value", numeric_max_);
	}

	switch(type_) {
	case XPOSITION:
		node.add("type", "x");
		break;
	case YPOSITION:
		node.add("type", "y");
		break;
	case TYPE_LEVEL:
		node.add("type", "level");
		break;
	case TYPE_LABEL:
		node.add("type", "label");
		break;
	case TYPE_TEXT:
		node.add("type", "text");
		break;
	case TYPE_BOOLEAN:
		node.add("type", "boolean");
		break;
	case TYPE_ENUM:
		node.add("type", "enum");
		node.add("values", util::join(enum_values_));
		break;
	case TYPE_POINTS:
		node.add("type", "points");
		break;
	case TYPE_INTEGER:
		// Handled by numeric_decimal_
		break;
	}
	return node.build();
}

void editor_variable_info::set_name(const std::string& name)
{
	name_ = name;
}

editor_entity_info::editor_entity_info(variant node)
  : category_(node["category"].as_string()),
    classification_(node["classification"].as_string_default()),
    editable_events_(node["events"].as_list_string_optional()),
	help_(node["help"].as_string_default())
{
	foreach(variant var_node, node["var"].as_list()) {
		//std::cerr << "CREATE VAR INFO...\n";	
		vars_.push_back(editor_variable_info(var_node));
	}

	vars_and_properties_ = vars_;
}

variant editor_entity_info::write() const
{
	variant_builder node;
	node.add("category", category_);
	node.add("classification", classification_);
	foreach(const editor_variable_info& v, vars_) {
		node.add("var", v.write());
	}

	return node.build();
}

const editor_variable_info* editor_entity_info::get_var_info(const std::string& var_name) const
{
	foreach(const editor_variable_info& v, vars_) {
		if(v.variable_name() == var_name) {
			return &v;
		}
	}

	return NULL;
}

const editor_variable_info* editor_entity_info::get_property_info(const std::string& var_name) const
{
	foreach(const editor_variable_info& v, properties_) {
		if(v.variable_name() == var_name) {
			return &v;
		}
	}

	return NULL;
}

const editor_variable_info* editor_entity_info::get_var_or_property_info(const std::string& var_name) const
{
	const editor_variable_info* result = get_var_info(var_name);
	if(result == NULL) {
		result = get_property_info(var_name);
	}

	return result;
}

void editor_entity_info::add_property(const editor_variable_info& prop)
{
	properties_.push_back(prop);
	vars_and_properties_ = vars_;
	vars_and_properties_.insert(vars_and_properties_.end(), properties_.begin(), properties_.end());
}

#endif // !NO_EDITOR

