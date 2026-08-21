/*!
@file       RuntimeReflect.cpp
@author     Kaeden Tan (kaedenjiawei.tan) 100%
@date       01/10/2025
@brief      Runtime reflection system providing type metadata and instance
            inspection/modification capabilities. Includes utilities for
            retrieving type information, accessing member names, and
            dynamically getting/setting field values through type-safe
            wrappers.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "pch.h"
#include "RuntimeReflect.h"
namespace rtr
{
    // Returns the name of the type.
    std::string const & TypeInfo::Name() const
    {
        return name;
    }

    // Returns a list of all reflected member names.
    std::vector<std::string> TypeInfo::Members() const
    {
		std::vector<std::string> membersList;
        for(auto const& [key, _] : members){
            membersList.push_back(key);
		}
        return membersList;
    }

    // Returns the std::type_index of the type.
    std::type_index const& TypeInfo::Type() const {
        return type;
    }

    // Returns the type_index of a member by its name.
    std::type_index const& TypeInfo::GetMemberType(std::string const& memberName) const {
        if (auto search = members.find(memberName); search != members.end()) {
            return search->second;
        };
		throw (std::out_of_range{ "Accessing member that does not exist" });
    }

    // Retrieves TypeInfo by its registered name.
    TypeInfo const &  TypeInfo::GetByName(std::string const& name) {
        std::unordered_map<std::string, rtr::TypeInfo>& typeMap = GetTypeMap();
        if (auto search = typeMap.find(name); search != typeMap.end()) {
            return search->second;
        };
		return TypeInfo::Get<void>();
    }

    // Checks if the instance is valid.
    bool Instance::isValid() const {
        return valid;
    }
    
    TypeInfo::MemberFlag TypeInfo::GetMemberFlag(std::string const& memberName) const {
        if (auto search = memberFlag.find(memberName); search == memberFlag.end()) {
            return 0;
        }
        else {
            return search->second;
        }
    }
    TypeInfo::MemberFlag TypeInfo::AllFlags() const {
        MemberFlag retVal{};
        for (auto const& [key, MemberFlag] : memberFlag) {
            retVal |= MemberFlag;
        }
        return retVal;
    }

    std::vector<std::string> TypeInfo::GetMemberNameByFlag(MemberFlag _flag) const {
        std::vector<std::string> vec{};
        for(auto const& [memberName, MemberFlag] : memberFlag){
            if (MemberFlag & _flag) {
                vec.push_back(memberName);
            }
		}
        return vec;
    }


}
