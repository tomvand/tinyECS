#pragma once

#include <etl/vector.h>
#include <etl/unordered_map.h>
#include <etl/utility.h>
#include <assert.h>

// Unique identifier for all entities
class Entity
{
    unsigned int id;
    static unsigned int id_count; // starts from 1, entity 0 is the default initialization
public:
    // Note, indices of already deleted entities aren't re-used in this simple implementation.
    Entity() : id(id_count++) {}
    operator unsigned int() const { return id; } // this enables automatic casting to int
    inline bool operator==(const Entity& other) const { return id == other.id; } // enables comparison of entities
};

// Common interface to refer to all containers in the ECS registry
struct ContainerInterface
{
    virtual void clear() = 0;
    virtual size_t size() const = 0;
    virtual size_t max_size() const = 0;
    virtual size_t available() const = 0;
    virtual void remove(Entity e) = 0;
    virtual bool has(Entity e) const = 0;
};

// An abstract, unsized container interface to store components of type 'Component' and associated entities
template <typename Component>
struct ComponentContainer : public ContainerInterface
{
    virtual Component& get(Entity e) = 0;
    virtual Component& insert(Entity e, Component c, bool check_for_duplicates = true) = 0;
};

// A concrete, sized container that stores components of type 'Component' and associated entities
template <typename Component, const size_t MAX_COMPONENTS> // A component can be any class
class SizedComponentContainer : public ComponentContainer<Component>
{
private:
    // The hash map from Entity -> array index.
    etl::unordered_map<unsigned int, unsigned int, MAX_COMPONENTS> map_entity_componentID; // the entity is cast to uint to be hashable.
    bool registered = false;
public:
    // Container of all components of type 'Component'
    etl::vector<Component, MAX_COMPONENTS> components;

    // The corresponding entities
    etl::vector<Entity, MAX_COMPONENTS> entities;

    // Constructor that registers the type
    SizedComponentContainer()
    {
    }

    // Inserting a component c associated to entity e
    inline Component& insert(Entity e, Component c, bool check_for_duplicates = true)
    {
        if (check_for_duplicates) {
            return emplace(e, etl::move(c)); // the move enforces move instead of copy constructor)
        } else {
            return emplace_with_duplicates(e, etl::move(c)); // the move enforces move instead of copy constructor)
        }
    }

    // The emplace function takes the the provided arguments Args, creates a new object of type Component, and inserts it into the ECS system
    template<typename... Args>
    Component& emplace(Entity e, Args &&... args) {
        // Usually, every entity should only have one instance of each component type
        assert(!has(e) && "Entity already contained in ECS registry");
        return emplace_with_duplicates(e, etl::forward<Args>(args)...); // the forward ensures that arguments are moved not copied
    }

    template<typename... Args>
    Component& emplace_with_duplicates(Entity e, Args &&... args) {
        map_entity_componentID[e] = (unsigned int)components.size();
        components.emplace_back(etl::forward<Args>(args)...); // the forward ensures that arguments are moved not copied
        entities.push_back(e);
        return components.back();
    }

    // A wrapper to return the component of an entity
    Component& get(Entity e) {
        assert(has(e) && "Entity not contained in ECS registry");
        return components[map_entity_componentID[e]];
    }

    const Component& get(Entity e) const {
        assert(has(e) && "Entity not contained in ECS registry");
        return components[map_entity_componentID.at(e)];
    }

    // Check if entity has a component of type 'Component'
    bool has(Entity e) const {
        return map_entity_componentID.count(e) > 0;
    }

    // Remove an component and pack the container to re-use the empty space
    void remove(Entity e)
    {
        if (!has(e)) return;

        // Get the current position
        int cID = map_entity_componentID[e];

        // Move the last element to position cID using the move operator
        // Note, components[cID] = components.back() would trigger the copy instead of move operator
        components[cID] = etl::move(components.back());
        entities[cID] = entities.back(); // the entity is only a single index, copy it.
        map_entity_componentID[entities.back()] = cID;

        // Erase the old component and free its memory
        map_entity_componentID.erase(e);
        components.pop_back();
        entities.pop_back();
        // Note, one could mark the id for re-use
    }

    // Remove all components of type 'Component'
    void clear()
    {
        map_entity_componentID.clear();
        components.clear();
        entities.clear();
    }

    // Report the number of components of type 'Component'
    size_t size() const
    {
        return components.size();
    }

    // Report the maximum capacity of the underlying container
    size_t max_size() const
    {
        return components.MAX_SIZE;
    }

    // Report the number of components that can be added to this container before it is full
    size_t available() const
    {
        return components.available();
    }
};
