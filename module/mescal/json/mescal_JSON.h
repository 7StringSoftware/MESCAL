#pragma once

class JSONObject;
class JSONArray;

class JSONStructure
{
public:
    JSONStructure();
    JSONStructure(juce::var jsonVar_);
    virtual ~JSONStructure() = default;

    virtual bool isArray() const
    {
        return jsonVar.isArray();
    }
    virtual bool isObject() const
    {
        return jsonVar.getDynamicObject() != nullptr;
    }
    virtual bool isValid() const
    {
        return false;
    }

#if JUCE_DEBUG
    virtual void dump(int /*depth*/ = 0) const
    {
    }
#endif

    juce::String toString() const;
    void toMemoryBlock(juce::MemoryBlock& block) const;
    virtual void fromMemoryBlock(juce::MemoryBlock const& block);

    std::unique_ptr<JSONObject> getObject() const;
    std::unique_ptr<JSONArray> getArray() const;

    juce::var jsonVar{};
};

class JSONArray;

class JSONObject : public JSONStructure
{
public:
    JSONObject();
    JSONObject(juce::var jsonVar_);
    JSONObject(const char* buffer, size_t bufferBytes);
    ~JSONObject() override = default;

    void fromMemoryBlock(juce::MemoryBlock const& block) override;

    bool isObject() const override
    {
        return true;
    }
    bool isValid() const override
    {
        return jsonVar.getDynamicObject() != nullptr;
    }

#if JUCE_DEBUG
    void dump(int depth = 0) const override;
#endif

    int getNumProperties() const;
    bool hasProperty(const juce::Identifier& propertyName) const;

    template<typename T> T get(const juce::Identifier& propertyName) const
    {
        if (auto object = jsonVar.getDynamicObject())
        {
            return static_cast<T>(object->getProperty(propertyName));
        }

        return {};
    }

    template<> juce::String get<juce::String>(const juce::Identifier& propertyName) const
    {
        return get<juce::var>(propertyName).toString();
    }

    template<> JSONArray get<JSONArray>(const juce::Identifier& propertyName) const;

    JSONArray getArray(const juce::Identifier& propertyName) const;

    template<> juce::MACAddress get<juce::MACAddress>(const juce::Identifier& propertyName) const
    {
        return juce::MACAddress{ get<juce::String>(propertyName) };
    }

    juce::String getString(const juce::Identifier& propertyName) const
    {
        return get<juce::String>(propertyName);
    }

    juce::MACAddress getMACAddress(const juce::Identifier& propertyName) const
    {
        return juce::MACAddress{ get<juce::String>(propertyName) };
    }

    template<> float get<float>(const juce::Identifier& propertyName) const
    {
        return (float)(double)get<juce::var>(propertyName);
    }

    template<typename T> void set(const juce::Identifier& propertyName, T value)
    {
        if (auto object = jsonVar.getDynamicObject())
        {
            object->setProperty(propertyName, value);
        }
    }

    template<> void set<JSONArray>(const juce::Identifier& propertyName, JSONArray array);

    void setArray(const juce::Identifier& propertyName, JSONArray array);

    void setString(const juce::Identifier& propertyName, juce::String value)
    {
        set<juce::String>(propertyName, value);
    }

    template<> void set<juce::MACAddress>(const juce::Identifier& propertyName, juce::MACAddress value)
    {
        set<juce::String>(propertyName, value.toString().toUpperCase());
    }

    void setMACAddress(const juce::Identifier& propertyName, juce::MACAddress value)
    {
        set<juce::MACAddress>(propertyName, value);
    }
};

class JSONArray : public JSONStructure
{
public:
    JSONArray(juce::var jsonVar_);
    JSONArray(juce::StringArray const& strings_);
    template<typename T> JSONArray(juce::Array<T> const& sourceArray_)
    {
        for (auto const& entry : sourceArray_)
        {
            add<T>(entry);
        }
    }
    JSONArray();
    ~JSONArray() override = default;

    void fromMemoryBlock(juce::MemoryBlock const& block) override;

    bool isArray() const override
    {
        return true;
    }

    bool isValid() const override
    {
        return jsonVar.getArray() != nullptr;
    }

#if JUCE_DEBUG
    void dump(int depth = 0) const override;
#endif

    int size() const;

    template<typename T> T get(int index) const
    {
        if (auto array = jsonVar.getArray())
        {
            return static_cast<T>(*array)[index];
        }

        return {};
    }

    template<> juce::String get<juce::String>(int index) const
    {
        return get<juce::var>(index).toString();
    }

    template<> juce::MACAddress get<juce::MACAddress>(int index) const
    {
        return juce::MACAddress{ get<juce::String>(index) };
    }

    JSONObject getObject(int index)
    {
        return JSONObject{ get<juce::var>(index) };
    }

    template<typename T> juce::Array<T> getArray() const
    {
        if (auto array = jsonVar.getArray())
        {
            juce::Array<T> outputArray;
            for (auto const& entry : *array)
            {
                outputArray.add(entry);
            }

            return outputArray;
        }

        return {};
    }

    template<> float get<float>(int index) const
    {
        return (float)(double)get<juce::var>(index);
    }

    template<typename T> void add(T value)
    {
        if (auto array = jsonVar.getArray())
        {
            array->add(value);
        }
    }

    template<> void add<juce::MACAddress>(juce::MACAddress address)
    {
        add(address.toString().toUpperCase());
    }

    template<> void add<JSONObject>(JSONObject object)
    {
        if (auto array = jsonVar.getArray())
        {
            array->add(object.jsonVar);
        }
    }

    template<typename T> void addArray(juce::Array<T> const& sourceArray)
    {
        for (auto const& entry : sourceArray)
        {
            add<T>(entry);
        }
    }

    bool contains(juce::var const& value) const
    {
        if (auto array = jsonVar.getArray())
        {
            return array->contains(value);
        }

        return false;
    }

    juce::var* begin() const
    {
        if (auto array = jsonVar.getArray())
        {
            return array->begin();
        }

        return nullptr;
    }

    juce::var* end() const
    {
        if (auto array = jsonVar.getArray())
        {
            return array->end();
        }

        return nullptr;
    }
};
