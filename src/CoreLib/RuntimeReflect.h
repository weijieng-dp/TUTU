/*!
@file       RuntimeReflect.h
@author     Kaeden Tan (kaedenjiawei.tan) 90%
@co-author  Ng Wei Jie (weijie.ng) 10%

@date       01/10/2025
@brief      Runtime reflection system providing type metadata and instance
            inspection/modification capabilities. Includes utilities for
            retrieving type information, accessing member names, and
            dynamically getting/setting field values through type-safe
            wrappers.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/

#pragma once
#include "refl.hpp"

#include <unordered_map>
#include <functional>
#include <any>
#include <typeindex>

//-----------------------------------------------------------------------------
// Macro to make a type reflectable
//-----------------------------------------------------------------------------
#define REFLECTABLE_PROPERTIES                                                               \
    virtual const rtr::TypeInfo &GetTypeInfo() const override                             \
    {                                                                                     \
        return rtr::TypeInfo::Get<::refl::trait::remove_qualifiers_t<decltype(*this)>>(); \
    };                                                                                    \
    virtual rtr::Instance GetInstance() override                                          \
    {                                                                                     \
        rtr::Instance ret;                                                                \
        return ret.Create<::refl::trait::remove_qualifiers_t<decltype(*this)>>(*this);    \
    }

namespace ComponentTraits {
    template<typename T>
    struct serializableType : std::true_type {};

    template<typename T>
    constexpr bool isSerializable = serializableType<T>::value;

    template<typename T>
    struct editorInspectableType : std::true_type {};

    template<typename T>
    constexpr bool isEditorInspectable = editorInspectableType<T>::value;
}

#define DO_NOT_SERIALIZE(type) \
template<> \
struct ComponentTraits::serializableType<type>: std::false_type{}

#define NOT_INSPECTABLE(type) \
template<> \
struct ComponentTraits::editorInspectableType<type>: std::false_type{}



//-----------------------------------------------------------------------------
// Runtime reflection system namespace
//-----------------------------------------------------------------------------

namespace rtr
{
	// Forward declarations
	// Used by SetSharedMap/GetTypeMap
	class TypeInfo;
    class Instance;

	// Pointer to shared type map, typename mapped to TypeInfo
	// used to simulate a singleton pattern of a global type map
    inline std::unordered_map<std::string, TypeInfo>* g_sharedMap = nullptr;

    /*!
    * \brief
	*   allocates a shared type map to be used by typeinfo
	*   uses a pointer to simulate singleton pattern
    *
    * \tparam map
	*   - Map to be used as the shared type map
    */
    inline void SetSharedMap(std::unordered_map<std::string, TypeInfo>* map) {
        g_sharedMap = map;
    }


    /*!
    * \brief
	*   Gets the shared type map used by TypeInfo
    *
    * \return
	*   - reference to the shared type map
    */
    inline std::unordered_map<std::string, TypeInfo>& GetTypeMap() {
        return *g_sharedMap;
    }


    class TypeInfo
    {
    public:
        using MemberFlag = unsigned;

        /*!
        * \brief
        *   Retrieves the singleton TypeInfo instance for the given type T.
        *
        * \tparam T
        *   - The reflected type.
        *
        * \return
        *   - Reference to the TypeInfo associated with T.
        */


        template <typename T>
        static TypeInfo const& Get()
        {
            static const TypeInfo ti(refl::reflect<T>());
            return ti;
        }

        template<>
        static TypeInfo const& Get<void>()
        {
            static const TypeInfo ti{};
            return ti;
        }
        /*!
        * \brief
        *   Initializes a TypeInfo entry for the given type T and registers it.
        *
        * \tparam T
        *   - The reflected type.
        */
        template <typename T>
        static void Init() {
            TypeInfo ti(refl::reflect<T>());
            if (auto search = GetTypeMap().find(ti.name); search == GetTypeMap().end()) {
                GetTypeMap().emplace(ti.name, ti);
            }
            else {
                search->second = ti;
            }
        };

        /*!
        * \brief
        *   Retrieves TypeInfo by reflected type name.
        *
        * \param[in] name
        *   - The string name of the type to look up.
        *
        * \return
        *   - The corresponding TypeInfo object.
        */
        static TypeInfo const& GetByName(std::string const& name);

        bool isValid() const {
            return type != std::type_index(typeid(void));
		}

        /*!
        * \brief
        *   Retrieves the name of the type.
        *
        * \return
        *   - The reflected type name.
        */
        std::string const& Name() const;

        /*!
        * \brief
        *   Retrieves all reflected member names of the type.
        *
        * \return
        *   - Vector containing member names.
        */
        std::vector<std::string> Members() const;

        /*!
        * \brief
        *   Retrieves the type information of a reflected member field.
        *
        * \param[in] name
        *   - Field name to query.
        *
        * \return
        *   - Type index of the field.
        */
        std::type_index const& GetMemberType(std::string const& name) const;

        /*!
        * \brief
        *   Retrieves the std::type_index representing the type.
        *
        * \return
        *   - Type information.
        */
        std::type_index const& Type() const;
        
        /*!
        * \brief
        *   Retrieves the names of all members with a specific type T.
        *
        * \tparam T
        *   - Type to search for in the component's members.
        *
        * \return
        *   - Vector of member names whose type matches T.
        */
        template <typename T>
        std::vector<std::string> GetMembersWithType() const {
            std::vector<std::string> outVec{};
            std::type_index searchType{ typeid(T) };
            for (auto const& [memberName, m_type] : members) {
                if (searchType == m_type) {
                    outVec.push_back(memberName);
                }
            }
            return outVec;
        }

        /*!
        * \brief
        *   Retrieves the MemberFlag associated with a specific member.
        *
        * \param[in] memberName
        *   - Name of the member field.
        *
        * \return
        *   - MemberFlag representing properties/permissions of the member.
        */
        MemberFlag GetMemberFlag(std::string const& memberName) const;

        /*!
        * \brief
        *   Retrieves the names of all members that match a specific flag.
        *
        * \param[in] _flag
        *   - MemberFlag to match.
        *
        * \return
        *   - Vector of member names with the given flag.
        */
		std::vector<std::string> GetMemberNameByFlag(MemberFlag _flag) const;

        /*!
        * \brief
        *   Retrieves the combined flags for all members.
        *
        * \return
        *   - MemberFlag representing all flags set on this type.
        */
        MemberFlag AllFlags() const;

        /*!
        * \brief
        *   Checks if this component is serializable.
        *
        * \return
        *   - true if the component can be serialized, false otherwise.
        */
		bool IsSerializable() const { return serializable; }

        /*!
        * \brief
        *   Checks if this component is inspectable in the editor.
        *
        * \return
        *   - true if the component can be inspected in the editor, false otherwise.
        */
		bool IsEditorInspectable() const { return editorInspectable; }

    private:
		// Type name
        std::string name;

		// Member name to type_index mapping
        std::unordered_map<std::string, std::type_index> members;
        std::unordered_map<std::string, unsigned long> memberFlag;

		// type_index of the type
        std::type_index type;
		bool serializable{ true };
        bool editorInspectable{ true };


        /*!
        * \brief
        *   Constructs a TypeInfo object from a type descriptor.
        *
        * \tparam T
        *   - Reflected type.
        *
        * \param[in] td
        *   - Type descriptor containing reflection metadata.
        */
        template <typename T>
        TypeInfo(refl::type_descriptor<T> td)
            : name(td.name), members(), type(std::type_index(typeid(T)))
        {
            T defaultInstance{};


            refl::util::for_each(td.members, [&](auto member, size_t index) {
                members.insert({
                    member.name.c_str(),
                    std::type_index(typeid(decltype(member(defaultInstance))))
                    });
                memberFlag.insert({
                    member.name.c_str(),
                    1 << (index)
                    });
                GetTypeMap().insert({ name, *this });
                });
            serializable = ComponentTraits::isSerializable<T>;
            editorInspectable = ComponentTraits::isEditorInspectable<T>;
        }

        TypeInfo() : name(), members(), memberFlag(), type(typeid(void)) {};
    };

    class Instance
    {
    public:
        /*!
        * \brief
        *   Default constructor for Instance.
        */
        Instance() = default;

        /*!
        * \brief
        *   Copy assignment operator.
        *
        * \param[in] other
        *   - Instance to copy from.
        *
        * \return
        *   - Reference to this instance.
        */
        Instance& operator=(Instance const&) = default;

        /*!
        * \brief
        *   Creates a runtime reflection instance from an object reference.
        *
        * \tparam T
        *   - The type of the object being reflected.
        *
        * \param[in] val
        *   - Reference to the object.
        *
        * \return
        *   - This instance after initialization.
        */
        template <typename T>
        Instance& Create(T& val)
        {
            *this = Instance(refl::reflect<T>(), val);
            return *this;
        }

        template <typename T>
        T* GetPtr(std::string const& name)
        {
            return std::any_cast<T*>(fields.at(name).value);
        }

        /*!
        * \brief
        *   Retrieves a const reference to a field value by name.
        *
        * \tparam T
        *   - Expected type of the field.
        *
        * \param[in] name
        *   - Field name to retrieve.
        *
        * \return
        *   - Const reference to the field value.
        */
        template <typename T>
        T const& GetVal(std::string const& name) const
        {
            return *(std::any_cast<T*>(fields.at(name).value));
        }

        /*!
        * \brief
        *   Sets a fieldÅfs value by name.
        *
        * \tparam T
        *   - Type of the value to assign.
        *
        * \param[in] name
        *   - Field name to set.
        * \param[in] val
        *   - New value for the field.
        */
        template <typename T>
        void SetVal(std::string const& name, T const& val)
        {
            *(std::any_cast<T*>(fields.at(name).value)) = val;
        }

        /*!
        * \brief
        *   Retrieves the reflected type information for this instance.
        *
        * \return
        *   - Reference to associated TypeInfo.
        */
        TypeInfo const& GetType() const { return *ti; }

        /*!
        * \brief
        *   Retrieves the type information of a reflected member field.
        *
        * \param[in] name
        *   - Field name to query.
        *
        * \return
        *   - Type index of the field.
        */
        std::type_index const& GetMemberType(std::string const& name) const
        {
            return fields.at(name).type;
        }

        /*!
        * \brief
        *   Checks if this instance is valid (constructed properly).
        *
        * \return
        *   - True if valid, false otherwise.
        */
        bool isValid() const;


    private:
        struct FieldInfo
        {
            std::type_index type;
            std::any value;
        };

		// Field name to pair<std::type_index, std::any> mapping
        std::unordered_map<std::string, FieldInfo> fields{};

		// Validity MemberFlag, true if instance is properly constructed
		// false if instance is default constructed
        bool valid{ false };

		// Pointer to associated TypeInfo
        TypeInfo const* ti{};

        /*!
        * \brief
        *   Constructs an instance from a type descriptor and object reference.
        *
        * \tparam T
        *   - Reflected type.
        *
        * \param[in] td
        *   - Type descriptor of T.
        * \param[in] val
        *   - Reference to the object to reflect.
        */
        template <typename T>
        Instance(refl::type_descriptor<T> td, T& val)
            : fields(), valid(true)
        {
            ti = &(val.GetTypeInfo());
            refl::util::for_each(td.members, [&](auto member) {
                fields.emplace(
                    member.name.c_str(),
                    FieldInfo{
                        std::type_index(typeid(decltype(member(val)))),
                        std::any(&(member(val)))
                    }
                );
                });
        }
    };

    class Reflectable
    {
    public:
        /*!
        * \brief
        *   Retrieves type information of this reflectable type.
        *
        * \return
        *   - Associated TypeInfo.
        */
        virtual const TypeInfo& GetTypeInfo() const = 0;

        /*!
        * \brief
        *   Retrieves an instance wrapper around this object.
        *
        * \return
        *   - Reflection-enabled instance.
        */
        virtual Instance GetInstance() = 0;
    };
}
