/*!
@file       CEO.h
@author     Ng Wei Jie (weijie.ng) (100%)
@date       25/09/2025

    Declares the CEO class, which functions as the central service locator and
    global manager container for the engine. The CEO stores and provides access
    to all engine-level managers using a type-indexed registry. This file also
    defines the BaseManager interface and the DeriveManager<T> wrapper used to
    enable type-erased storage of heterogeneous manager instances.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/

#pragma once
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <cassert>

/*!
* \brief
*   Base class for all manager wrappers stored inside the CEO.
*   Enables heterogeneous manager types to be stored in a single container.
*/
class BaseManager {
public:
    virtual ~BaseManager() = default;
};

/*!
* \brief
*   Templated wrapper that stores an instance of a concrete manager type.
*
* \tparam T
*   Type of manager to be stored.
*/
template <typename T>
class DeriveManager : public BaseManager {
public:
    T instance;   
};

/*!
* \brief
*   The CEO class acts as the central service locator for all engine managers.
*   Managers are stored in a type-indexed map and can be added, removed, and retrieved.
*/
class CEO
{
public:

    /*!
    * \brief
    *   Retrieves the global CEO instance.
    *   SetInstance() must be called before using this.
    *
    * \return
    *   Reference to the CEO singleton instance.
    */
    static CEO& Instance()
    {
        return *instance;
    }

    /*!
    * \brief
    *   Sets the global CEO singleton pointer.
    *   Typically called once during engine initialization.
    *
    * \param[in] ceo
    *   Pointer to an externally allocated CEO instance.
    */
    void SetInstance(CEO* ceo) 
    {
        instance = ceo;
    }
    
    /*!
 * \brief  Returns the raw pointer to the CEO instance.
 */
    CEO* GetPtr()
    {
        return instance;
    }


    /*!
    * \brief
    *   Frees the CEO singleton instance.
    *   Only call this if the CEO was allocated using new.
    */
    void Free()
    {
        if(instance)
            delete instance;
    }

    /*!
* \brief
*   Creates and registers a new manager of type T inside the CEO.
*   If a manager of the same type already exists, it will be replaced.
*
* \tparam T
*   Type of manager to add.
*/
    template <typename T>
    void AddManager()
    {
        Managers[typeid(T)] = std::make_shared<DeriveManager<T>>();
    }

    /*!
* \brief
*   Removes a manager of type T from the CEO.
*
* \tparam T
*   Type of manager to remove.
*/
    template <typename T>
    void RemoveManager()
    {
        Managers.erase(typeid(T));
    }

    /*!
* \brief
*   Retrieves a pointer to the manager of type T stored in the CEO.
*   Asserts if the manager does not exist.
*
* \tparam T
*   Type of manager to retrieve.
*
* \return
*   Pointer to the manager instance of type T.
*/
    template <typename T>
    T* GetManager()
    {
        auto it = Managers.find(typeid(T));
        if (it != Managers.end())
        {
            // Cast BaseManager* to DeriveManager<T>*
            auto derived = dynamic_cast<DeriveManager<T>*>(it->second.get());
            if (derived)
                return &derived->instance;   // return T*
        }

        assert(false && "Manager cannot be found! make sure it is added into CEO");
        return nullptr;
    }

    /*!
    * \brief
    *   Retrieves a pointer to the manager of type T stored in the CEO.
    *   Asserts if the manager does not exist.
    *
    * \tparam T
    *   Type of manager to retrieve.
    *
    * \return
    *   Pointer to the manager instance of type T.
    */
    template <typename T>
    static T* Get()
    {
        
        auto it = instance->Managers.find(typeid(T));
        if (it != instance->Managers.end())
        {
            // Cast BaseManager* to DeriveManager<T>*
            auto derived = dynamic_cast<DeriveManager<T>*>(it->second.get());
            if (derived)
                return &derived->instance;   // return T*
        }
        assert(false && "Manager cannot be found! make sure it is added into CEO");
        return nullptr;
    }
private:
    std::unordered_map<std::type_index, std::shared_ptr<BaseManager>> Managers;
    static CEO* instance;

};
